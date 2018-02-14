#include "bus.h"
#include "midi.h"
#include "serial.h"
#include "device.h"
#include "message.h"
#include "DigitalBusStreamReader.h"
#include "MidiReader.h"
#include "cmsis_os.h"
#include "MidiWriter.hpp"
#include "errorhandlers.h"

#ifdef USE_DIGITALBUS

// static uint8_t busframe[4];
MidiWriter writer;
static DigitalBusStreamReader bus;
static SerialBuffer<128> bus_tx_buf;
static SerialBuffer<128> bus_rx_buf;
static MidiReader midi;
bool DIGITAL_BUS_PROPAGATE_MIDI = 0;
bool DIGITAL_BUS_ENABLE_BUS = 0;

void midiSendCC(uint8_t ch, uint8_t cc, uint8_t value){
  writer.controlChange(ch, cc, value); // send to bus
  // midi.controlChange(ch, cc, value); // send over USB
  // send to bus and USB
  // uint8_t packet[4] = { USB_COMMAND_CONTROL_CHANGE,
  // 			(uint8_t)(CONTROL_CHANGE | ch),
  // 			cc, value };
  // serial_write(packet, sizeof(packet));
  // midi_tx_usb_buffer(packet, sizeof(packet));
  // todo: to update an attached USB client the message should be forwarded
  // but to avoid message feedback loops this is disabled
}

void midiSendPC(uint8_t ch, uint8_t pc){
  writer.programChange(ch, pc);
}

extern "C" {

#if 1
  void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
    bus_tx_buf.incrementReadHead(huart->TxXferCount);
    if(bus_tx_buf.notEmpty())
      HAL_UART_Transmit_DMA(huart, bus_tx_buf.getReadHead(), bus_tx_buf.getContiguousReadCapacity());
  }
  void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    bus_rx_buf.incrementWriteHead(huart->RxXferCount);
    HAL_UART_Receive_DMA(huart, bus_rx_buf.getWriteHead(), bus_rx_buf.getContiguousWriteCapacity());
  }
  void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
    error(RUNTIME_ERROR, "uart error");    
  }

#else

  void USART1_IRQHandler(void){
    extern UART_HandleTypeDef huart1;
    UART_HandleTypeDef *huart = &huart1;
    uint32_t isrflags   = READ_REG(huart->Instance->ISR);
    uint32_t errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE));
    /* uint16_t uhMask = huart->Mask; */
    /* If no error occurs */
    if(errorflags == RESET){
      /* UART in mode Receiver ---------------------------------------------------*/
      if(((isrflags & USART_ISR_RXNE) != RESET)){ // && ((cr1its & USART_CR1_RXNEIE) != RESET))
	// serial_rx_callback((uint8_t)(huart->Instance->RDR & (uint8_t)uhMask));
	// serial_rx_callback((uint8_t)(huart->Instance->RDR));
	bus.read((uint8_t)(huart->Instance->RDR));
      }
      /* UART in mode Transmitter ------------------------------------------------*/
      if(((isrflags & USART_ISR_TXE) != RESET)){  // && ((cr1its & USART_CR1_TXEIE) != RESET)){
	if(bus_tx_buf.notEmpty()){
	  huart->Instance->TDR = bus_tx_buf.pull(); // (uint8_t)(*huart->pTxBuffPtr++ & (uint8_t)0xFFU);
	  // USART_SendData(USART_PERIPH, bus_tx_buf.pull());
	}else{
	  /* Disable the UART Transmit Data Register Empty Interrupt */
	  CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE);
	  // USART_ITConfig(USART_PERIPH, USART_IT_TXE, DISABLE);
	}
      }
    }else{
      /* If some errors occur */
      /* if((errorflags != RESET) && ((cr3its & (USART_CR3_EIE | USART_CR1_PEIE)) != RESET)){ */
      /* UART parity error interrupt occurred -------------------------------------*/
      if(((isrflags & USART_ISR_PE) != RESET)) //  && ((cr1its & USART_CR1_PEIE) != RESET))
	{
	  __HAL_UART_CLEAR_IT(huart, UART_CLEAR_PEF);
	  setErrorMessage(UART_ERROR, "uart parity error");
	  huart->ErrorCode |= HAL_UART_ERROR_PE;
	}
      /* UART frame error interrupt occurred --------------------------------------*/
      if(((isrflags & USART_ISR_FE) != RESET)) // && ((cr3its & USART_CR3_EIE) != RESET))
	{
	  __HAL_UART_CLEAR_IT(huart, UART_CLEAR_FEF);
	  setErrorMessage(UART_ERROR, "uart frame error");
	  huart->ErrorCode |= HAL_UART_ERROR_FE;
	}
      /* UART noise error interrupt occurred --------------------------------------*/
      if(((isrflags & USART_ISR_NE) != RESET)) // && ((cr3its & USART_CR3_EIE) != RESET))
	{
	  __HAL_UART_CLEAR_IT(huart, UART_CLEAR_NEF);
	  setErrorMessage(UART_ERROR, "uart noise error");
	  huart->ErrorCode |= HAL_UART_ERROR_NE;
	}    
      /* UART Over-Run interrupt occurred -----------------------------------------*/
      if(((isrflags & USART_ISR_ORE) != RESET)) //  &&
	// (((cr1its & USART_CR1_RXNEIE) != RESET) || ((cr3its & USART_CR3_EIE) != RESET)))
	{
	  __HAL_UART_CLEAR_IT(huart, UART_CLEAR_OREF);
	  __HAL_UART_FLUSH_DRREGISTER(huart);
	  setErrorMessage(UART_ERROR, "uart overrun");
	  huart->ErrorCode |= HAL_UART_ERROR_ORE;
	}
    }
  }
#endif

  void serial_write(uint8_t* data, uint16_t size){
    bus_tx_buf.push(data, size);
    extern UART_HandleTypeDef huart1;
    UART_HandleTypeDef *huart = &huart1;
    /* Check that a Tx process is not already ongoing */
    if(huart->gState == HAL_UART_STATE_READY) 
      HAL_UART_Transmit_DMA(huart, bus_tx_buf.getReadHead(), bus_tx_buf.getContiguousReadCapacity());

    // if(huart->RxState == HAL_UART_STATE_READY)
    //   HAL_UART_Transmit_DMA(huart, bus_tx_buf.getReadHead(), bus_tx_buf.getContiguousReadCapacity());

      
      
    /* Enable the UART Transmit Data Register Empty Interrupt */
    // extern UART_HandleTypeDef huart1;
    // SET_BIT(huart1.Instance->CR1, USART_CR1_TXEIE);
    // USART_ITConfig(USART_PERIPH, USART_IT_TXE, ENABLE);
  }

  // void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
  //   error(UART_ERROR, "uart error");
  //   bus.reset();
  //   rxbuf.reset();
  // }

  // void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
  //   rxbuf.incrementWriteHead(4);
  //   __HAL_UART_FLUSH_DRREGISTER(huart);
  //   serial_read(rxbuf.getWriteHead(), 4);
  // }

  // void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
  // }
}

uint8_t* bus_deviceid(){
  // return ((uint8_t*)0x1ffff7e8); /* STM32F1 */
  return ((uint8_t *)0x1FFF7A10); /* STM32F4, STM32F0 */ 
}

void bus_setup(){
  debug << "bus_setup\r\n";
  // serial_setup(USART_BAUDRATE);
  // bus.sendReset();

  extern UART_HandleTypeDef huart1;
  UART_HandleTypeDef *huart = &huart1;
  HAL_UART_Receive_DMA(huart, bus_rx_buf.getWriteHead(), bus_rx_buf.getContiguousWriteCapacity());

  // extern UART_HandleTypeDef huart1;
  // UART_HandleTypeDef *huart = &huart1;
  // /* Enable the UART Parity Error Interrupt */
  // SET_BIT(huart->Instance->CR1, USART_CR1_PEIE);
  // /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  // // SET_BIT(huart->Instance->CR3, USART_CR3_EIE);
  // /* Enable the UART Data Register not empty Interrupt */
  // SET_BIT(huart->Instance->CR1, USART_CR1_RXNEIE);

  // serial_read(rxbuf.getWriteHead(), 4);
}

#define BUS_IDLE_INTERVAL 2197

int bus_status(){
  bus.process();
  static uint32_t lastpolled = 0;
  if(osKernelSysTick() > lastpolled + BUS_IDLE_INTERVAL){
    bus.connected();
    lastpolled = osKernelSysTick();
  }
  return bus.getStatus();
}

void bus_tx_parameter(uint8_t pid, int16_t value){
  debug << "tx parameter [" << pid << "][" << value << "]" ;
  bus.sendParameterChange(pid, value);
}

void bus_tx_command(uint8_t cmd, int16_t data){
  debug << "tx command [" << cmd << "][" << data << "]" ;
  bus.sendCommand(cmd, data);
}

void bus_tx_message(const char* msg){
  debug << "tx msg [" << msg << "]" ;
  bus.sendMessage(msg);
}

void bus_tx_error(const char* reason){
  debug << "Digital bus send error: " << reason << ".";
}

void bus_rx_parameter(uint8_t pid, int16_t value){}
void bus_rx_command(uint8_t cmd, int16_t data){}
void bus_rx_message(const char* msg){}
void bus_rx_data(const uint8_t* data, uint16_t size){}

void bus_rx_error(const char* reason){
  debug << "Digital bus receive error: " << reason << ".";
  bus.reset();
  bus.sendReset();
}

extern "C" {
  void midi_rx_usb_buffer(uint8_t *buffer, uint32_t length){
    midi.readMidiFrame(buffer); // process locally
    bus.sendFrame(buffer); // forward USB MIDI to serial bus
  }
}

#endif /* USE_UART */
