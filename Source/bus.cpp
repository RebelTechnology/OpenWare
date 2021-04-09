#include "bus.h"
#include "midi.h"
#include "device.h"
#include "message.h"
#include "SerialBuffer.hpp"
#include "DigitalBusReader.h"
#include "errorhandlers.h"
#include "basicmaths.h"
#include "Owl.h"

#ifdef USE_DIGITALBUS

#define DIGITAL_BUS_BUFFER_SIZE 512

// static uint8_t busframe[4];
DigitalBusReader bus;
static SerialBuffer<DIGITAL_BUS_BUFFER_SIZE> bus_tx_buf;
static SerialBuffer<DIGITAL_BUS_BUFFER_SIZE> bus_rx_buf;
// todo: store data in 32bit frame buffers
uint32_t bus_tx_packets = 0;
uint32_t bus_rx_packets = 0;

static void initiateBusRead(){
#ifndef OWL_RACK // currently we suppress all returning messages
  extern UART_HandleTypeDef BUS_HUART;
  UART_HandleTypeDef *huart = &BUS_HUART;
  /* Check that a Rx process is not already ongoing */
  if(huart->RxState == HAL_UART_STATE_READY){
    uint16_t size = min(bus_rx_buf.getCapacity()/2, bus_rx_buf.getContiguousWriteCapacity());
    // keep at least half the buffer back, it will fill up while this half is processing
    HAL_UART_Receive_DMA(huart, bus_rx_buf.getWriteHead(), size);
  }
#endif
}

static void initiateBusWrite(){
  if(bus_tx_buf.notEmpty()){
    /* Check that a tx process is not already ongoing */
    extern UART_HandleTypeDef BUS_HUART;
    UART_HandleTypeDef *huart = &BUS_HUART;
    if(huart->gState == HAL_UART_STATE_READY) 
#ifdef OWL_PEDAL // no DMA available for UART4 tx!
      HAL_UART_Transmit_IT(huart, bus_tx_buf.getReadHead(), bus_tx_buf.getContiguousReadCapacity());
#else
      HAL_UART_Transmit_DMA(huart, bus_tx_buf.getReadHead(), bus_tx_buf.getContiguousReadCapacity());
#endif
  }
}

extern "C" {
  void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
    // /* Disable TXEIE and TCIE interrupts */
    // CLEAR_BIT(huart->Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE));
    int size = huart->TxXferSize; // - huart->TxXferCount;
    bus_tx_buf.incrementReadHead(size);
    bus_tx_packets += size/4;
    initiateBusWrite();
  }
  void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    // what is the correct size if IDLE interrupts?
    // int size = huart->RxXferSize - huart->RxXferCount;
    size_t size = huart->RxXferSize - __HAL_DMA_GET_COUNTER(huart->hdmarx);
    bus_rx_buf.incrementWriteHead(size);
    bus_rx_packets += size/4;
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
  }
}

uint8_t* bus_deviceid(){
  // return ((uint8_t*)0x1ffff7e8); /* STM32F1 */
  return ((uint8_t *)0x1FFF7A10); /* STM32F4, STM32F0 */ 
}

void bus_setup(){
  debug << "bus_setup\r\n";
  // serial_setup(USART_BAUDRATE);
  // bus.sendReset();

  extern UART_HandleTypeDef BUS_HUART;
  UART_HandleTypeDef *huart = &BUS_HUART;

  /* Enable IDLE line detection */
  __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
    
  /* Enable Parity Error Interrupt */
  __HAL_UART_ENABLE_IT(huart, UART_IT_PE);

  /* Enable Error Interrupt */
  __HAL_UART_ENABLE_IT(huart, UART_IT_ERR);

  initiateBusRead();
}

#define BUS_IDLE_INTERVAL 2197

int bus_status(){
  // incoming data
  while(bus_rx_buf.getReadCapacity() >= 4){
    uint8_t frame[4];
    bus_rx_buf.pull(frame, 4);
    if(frame[0] == OWL_COMMAND_RESET){
      bus_rx_buf.skipUntilLast(OWL_COMMAND_RESET);
    }else{
      bus.readBusFrame(frame);
    }
  }
  initiateBusRead();

  // outgoing data
  initiateBusWrite();
  
  if(settings.bus_enabled){
    static uint32_t lastpolled = 0;
    if(HAL_GetTick() > lastpolled + BUS_IDLE_INTERVAL){
      bus.connected();
      lastpolled = HAL_GetTick();
    }
  }
  return bus.getStatus();
}

void bus_tx_frame(uint8_t* data){
  bus.sendFrame(data);
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
  debug << "tx err[" << reason << "]";
  bus_tx_buf.reset();
}

__weak void bus_rx_parameter(uint8_t pid, int16_t value){
  setParameterValue(pid, value);
  debug << "rx par[" << pid << "]";
}

__weak void bus_rx_command(uint8_t cmd, int16_t data){
  debug << "rx cmd[" << cmd << "]";
}

__weak void bus_rx_message(const char* msg){
  debug << "rx msg[" << msg << "]";
}

__weak void bus_rx_data(const uint8_t* data, uint16_t size){
  debug << "rx data[" << size << "]";
}

void bus_rx_error(const char* reason){
  debug << "rx error: " << reason << ".";
  bus_rx_buf.reset();
  bus.reset();
  // bus.sendReset();
}

void bus_set_input_channel(uint8_t ch){
  bus.setInputChannel(ch);
}

#endif /* USE_DIGITALBUS */
