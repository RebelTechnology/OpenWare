#include "bus.h"
#include "midi.h"
#include "serial.h"
#include "device.h"
#include "message.h"
#include "SerialBuffer.hpp"
// #include "DigitalBusStreamReader.h"
#include "DigitalBusReader.h"
#include "cmsis_os.h"
#include "errorhandlers.h"
#include "basicmaths.h"

#ifdef USE_DIGITALBUS

// static uint8_t busframe[4];
static DigitalBusReader bus;
SerialBuffer<128> bus_tx_buf;
SerialBuffer<128> bus_rx_buf;
// todo: store data in 32bit frame buffers
bool DIGITAL_BUS_PROPAGATE_MIDI = 0;
bool DIGITAL_BUS_ENABLE_BUS = 1;

static void initiateBusRead(){
  extern UART_HandleTypeDef huart1;
  UART_HandleTypeDef *huart = &huart1;
  /* Check that a Rx process is not already ongoing */
  if(huart->RxState == HAL_UART_STATE_READY){
    uint16_t size = min(64, bus_rx_buf.getContiguousWriteCapacity());
    HAL_UART_Receive_DMA(huart, bus_rx_buf.getWriteHead(), size);
  }
}

extern "C" {
  void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
    int size = huart->TxXferSize; // - huart->TxXferCount;
    bus_tx_buf.incrementReadHead(size);
    if(bus_tx_buf.notEmpty())
      HAL_UART_Transmit_DMA(huart, bus_tx_buf.getReadHead(), bus_tx_buf.getContiguousReadCapacity());
  }
  void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    // what is the correct size if IDLE interrupts?
    // int size = huart->RxXferSize - huart->RxXferCount;
    int size = huart->RxXferSize - huart->hdmarx->Instance->NDTR;
    bus_rx_buf.incrementWriteHead(size);
    initiateBusRead();
  }
  void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
    error(RUNTIME_ERROR, "uart error");    
    bus.reset();
    bus_tx_buf.reset();
    bus_rx_buf.reset();
  }

  void serial_write(uint8_t* data, uint16_t size){
    bus_tx_buf.push(data, size);
    extern UART_HandleTypeDef huart1;
    UART_HandleTypeDef *huart = &huart1;
    /* Check that a Tx process is not already ongoing */
    if(huart->gState == HAL_UART_STATE_READY) 
      HAL_UART_Transmit_DMA(huart, bus_tx_buf.getReadHead(), bus_tx_buf.getContiguousReadCapacity());
  }

  // void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
  //   rxbuf.incrementWriteHead(4);
  //   __HAL_UART_FLUSH_DRREGISTER(huart);
  //   serial_read(rxbuf.getWriteHead(), 4);
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

  /* Enable IDLE line detection */
  __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
  // USART_ITConfig(huart->Instance, UART_IT_IDLE, ENABLE);
    
  /* Enable Parity Error Interrupt */
  __HAL_UART_ENABLE_IT(huart, UART_IT_PE);
  // USART_ITConfig(huart->Instance, UART_IT_PE, ENABLE);

  /* Enable Error Interrupt */
  __HAL_UART_ENABLE_IT(huart, UART_IT_ERR);
  // USART_ITConfig(huart->Instance, UART_IT_ERR, ENABLE);

    initiateBusRead();

  // extern UART_HandleTypeDef huart1;
  // UART_HandleTypeDef *huart = &huart1;
  // SET_BIT(huart->Instance->CR1, USART_CR1_PEIE);
  // /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  // // SET_BIT(huart->Instance->CR3, USART_CR3_EIE);
  // /* Enable the UART Data Register not empty Interrupt */
  // SET_BIT(huart->Instance->CR1, USART_CR1_RXNEIE);
  // serial_read(rxbuf.getWriteHead(), 4);
}

#define BUS_IDLE_INTERVAL 2197

int bus_status(){
  // bus.process();
  while(bus_rx_buf.available() >= 4){
    uint8_t frame[4];
    bus_rx_buf.pull(frame, 4);
    if(frame[0] == OWL_COMMAND_RESET){
      bus_rx_buf.skipUntilLast(OWL_COMMAND_RESET);
    }else{
      bus.readBusFrame(frame);
    }
  }
  initiateBusRead();

  static uint32_t lastpolled = 0;
  if(osKernelSysTick() > lastpolled + BUS_IDLE_INTERVAL){
    bus.connected();
    lastpolled = osKernelSysTick();
  }
  return bus.getStatus();
}

void bus_tx_parameter(uint8_t pid, int16_t value){
  debug << "tx par[" << pid << "][" << value << "]" ;
  bus.sendParameterChange(pid, value);
}

void bus_tx_command(uint8_t cmd, int16_t data){
  debug << "tx cmd[" << cmd << "][" << data << "]" ;
  bus.sendCommand(cmd, data);
}

void bus_tx_message(const char* msg){
  debug << "tx msg[" << msg << "]" ;
  bus.sendMessage(msg);
}

void bus_tx_error(const char* reason){
  debug << "tx error: " << reason << ".";
}

void bus_tx_frame(uint8_t* data){
  bus.sendFrame(data);
}

void bus_rx_parameter(uint8_t pid, int16_t value){
  debug << "rx par[" << pid << "]";
}
void bus_rx_command(uint8_t cmd, int16_t data){
  debug << "rx cmd[" << cmd << ".";
}
void bus_rx_message(const char* msg){
  debug << "rx msg[" << msg << "]";
}
void bus_rx_data(const uint8_t* data, uint16_t size){
  debug << "rx data[" << size << "]";
}

void bus_rx_error(const char* reason){
  debug << "rx error: " << reason << ".";
  bus.reset();
  bus.sendReset();
}

#endif /* USE_UART */
