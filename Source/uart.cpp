#include "message.h"
#include "uart.h"
#include "device.h"
#include "errorhandlers.h"
#include "SerialBuffer.hpp"

#if defined USE_UART_MIDI_TX || defined USE_UART_RX
extern UART_HandleTypeDef UART_MIDI_HANDLE;
#endif

#ifdef USE_UART_MIDI_RX
static SerialBuffer<UART_MIDI_RX_BUFFER_SIZE> bus_rx_buf DMA_RAM;
#define min(a,b) ((a)<(b)?(a):(b))

static void initiateBusRead(){
  UART_HandleTypeDef *huart = &UART_MIDI_HANDLE;
  /* Check that a Rx process is not already ongoing */
  if(huart->RxState == HAL_UART_STATE_READY){
    uint16_t size = min(bus_rx_buf.getCapacity()/2, bus_rx_buf.getContiguousWriteCapacity());
    // keep at least half the buffer back, it will fill up while this half is processing
    HAL_UART_Receive_DMA(huart, bus_rx_buf.getWriteHead(), size);
  }
}
#endif

void uart_init(){
#ifdef USE_UART_MIDI_RX
  UART_HandleTypeDef *huart = &UART_MIDI_HANDLE;
  /* Enable IDLE line detection */
  __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);    
  /* Enable Parity Error Interrupt */
  __HAL_UART_ENABLE_IT(huart, UART_IT_PE);
  /* Enable Error Interrupt */
  __HAL_UART_ENABLE_IT(huart, UART_IT_ERR);
  initiateBusRead();
#endif
}

bool uart_ready(){
#if defined USE_UART_MIDI_TX || defined USE_UART_RX
  UART_HandleTypeDef *huart = &UART_MIDI_HANDLE;
  return huart->gState == HAL_UART_STATE_READY;
#else
  return false;
#endif
}

void uart_tx(uint8_t* data, size_t len){
#ifdef USE_UART_MIDI_TX
  UART_HandleTypeDef *huart = &UART_MIDI_HANDLE;
#ifdef OWL_PEDAL // no DMA available for UART4 tx!
  HAL_UART_Transmit_IT(huart, data, len);
#else
  HAL_UART_Transmit_DMA(huart, data, len);
#endif
#endif
}

extern "C"{
#ifdef USE_UART_MIDI_TX
  void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
    // /* Disable TXEIE and TCIE interrupts */
    // CLEAR_BIT(huart->Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE));
    /* int size = huart->TxXferSize; // - huart->TxXferCount; */
    /* bus_tx_buf.incrementReadHead(size); */
    /* bus_tx_packets += size; */
    /* initiateBusWrite(); */
  }
#endif
  
#ifdef USE_UART_MIDI_RX
  void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    // what is the correct size if IDLE interrupts?
    size_t size = huart->RxXferSize - __HAL_DMA_GET_COUNTER(huart->hdmarx);
    bus_rx_buf.incrementWriteHead(size);
    /* bus_rx_packets += size; */
    initiateBusRead();
    size = bus_rx_buf.getContiguousReadCapacity();
    if(size){
      uart_rx_callback(bus_rx_buf.getReadHead(), size);
      bus_rx_buf.incrementReadHead(size);
    }
  }
#endif
  
#if defined USE_UART_MIDI_TX || defined USE_UART_RX
  void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
    error(RUNTIME_ERROR, "uart error");    
#ifdef USE_UART_MIDI_RX
    bus_rx_buf.reset();
#endif
  }
#endif
}
