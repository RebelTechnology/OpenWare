#include "message.h"
#include "uart.h"
#include "device.h"
#include "errorhandlers.h"
#include "SerialBuffer.hpp"

#if defined USE_UART_MIDI_TX || defined USE_UART_RX
#include "uart_midi.h"
extern UART_HandleTypeDef UART_MIDI_HANDLE;
extern SerialBuffer<UART_MIDI_RX_BUFFER_SIZE> uart_rx_buf;
#endif
#ifdef USE_DIGITALBUS
#include "bus.h"
#include "DigitalBusReader.h"
extern UART_HandleTypeDef BUS_HUART;
extern SerialBuffer<DIGITAL_BUS_BUFFER_SIZE> bus_tx_buf;
extern SerialBuffer<DIGITAL_BUS_BUFFER_SIZE> bus_rx_buf;
extern DigitalBusReader bus;
extern uint32_t bus_tx_packets;
extern uint32_t bus_rx_packets;
#endif

extern "C" {
#if defined USE_UART_MIDI_TX || defined USE_DIGITALBUS
  void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
    // Serial MIDI doesn't use TX callback
#ifdef USE_DIGITALBUS
    if (huart == &BUS_HUART) {
      // /* Disable TXEIE and TCIE interrupts */
      // CLEAR_BIT(huart->Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE));
      int size = huart->TxXferSize; // - huart->TxXferCount;
      bus_tx_buf.incrementReadHead(size);
      bus_tx_packets += size/4;
      initiateBusWrite();
    }
#endif
  }
#endif
  
#if defined USE_UART_MIDI_RX || defined USE_DIGITALBUS
  void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
#ifdef USE_UART_MIDI_RX
    if (huart == &UART_MIDI_HANDLE) {
      // what is the correct size if IDLE interrupts?
      size_t size = huart->RxXferSize - __HAL_DMA_GET_COUNTER(huart->hdmarx);
      uart_rx_buf.incrementWriteHead(size);
      /* bus_rx_packets += size; */
      initiateUartRead();
      size = uart_rx_buf.getContiguousReadCapacity();
      if(size) {
        uart_rx_callback(uart_rx_buf.getReadHead(), size);
        uart_rx_buf.incrementReadHead(size);
      }
    }
#endif
#ifdef USE_DIGITALBUS
    if (huart == &BUS_HUART) {
      // what is the correct size if IDLE interrupts?
      size_t size = huart->RxXferSize - __HAL_DMA_GET_COUNTER(huart->hdmarx);
      bus_rx_buf.incrementWriteHead(size);
      bus_rx_packets += size/4;
      initiateBusRead();
    }
#endif
  }
#endif
  
#if defined USE_UART_MIDI_TX || defined USE_UART_MIDI_RX || defined USE_DIGITALBUS
  void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
#ifdef USE_UART_MIDI_RX
    if (huart == &UART_MIDI_HANDLE) {
      uart_rx_buf.reset();
#ifndef DEBUG
      return;
#endif
    }
#endif
#ifdef USE_DIGITALBUS
    if (huart == &BUS_HUART) {
      bus.reset();
      bus_tx_buf.reset();
      bus_rx_buf.reset();
#ifndef DEBUG
      return;
#endif
    }
#endif
    // We don't want to show this to users in case of hotplug
    error(RUNTIME_ERROR, "uart error");
  }
#endif
}
