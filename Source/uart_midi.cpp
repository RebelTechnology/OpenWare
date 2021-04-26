#include "device.h"
#include "uart_midi.h"
#include "SerialBuffer.hpp"

#define min(a,b) ((a)<(b)?(a):(b))

#ifdef USE_UART_MIDI_RX
SerialBuffer<UART_MIDI_RX_BUFFER_SIZE> uart_rx_buf DMA_RAM;
#endif


#ifdef USE_UART_MIDI_RX
void initiateUartRead(){
  extern UART_HandleTypeDef UART_MIDI_HANDLE;
  UART_HandleTypeDef *huart = &UART_MIDI_HANDLE;
  /* Check that a Rx process is not already ongoing */
  if(huart->RxState == HAL_UART_STATE_READY){
    uint16_t size = min(uart_rx_buf.getCapacity()/2, uart_rx_buf.getContiguousWriteCapacity());
    // keep at least half the buffer back, it will fill up while this half is processing
    HAL_UART_Receive_DMA(huart, uart_rx_buf.getWriteHead(), size);
  }
}
#endif

void uart_init(){
#ifdef USE_UART_MIDI_RX
  extern UART_HandleTypeDef UART_MIDI_HANDLE;
  UART_HandleTypeDef *huart = &UART_MIDI_HANDLE;
  /* Enable IDLE line detection */
  __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);    
  /* Enable Parity Error Interrupt */
  __HAL_UART_ENABLE_IT(huart, UART_IT_PE);
  /* Enable Error Interrupt */
  __HAL_UART_ENABLE_IT(huart, UART_IT_ERR);
  initiateUartRead();
#endif
}

bool uart_ready(){
#if defined USE_UART_MIDI_TX || defined USE_UART_RX
  extern UART_HandleTypeDef UART_MIDI_HANDLE;
  UART_HandleTypeDef *huart = &UART_MIDI_HANDLE;
  return huart->gState == HAL_UART_STATE_READY;
#else
  return false;
#endif
}

void uart_tx(uint8_t* data, size_t len){
#ifdef USE_UART_MIDI_TX
  extern UART_HandleTypeDef UART_MIDI_HANDLE;
  UART_HandleTypeDef *huart = &UART_MIDI_HANDLE;
#ifdef OWL_PEDAL // no DMA available for UART4 tx!
  HAL_UART_Transmit_IT(huart, data, len);
#else
  HAL_UART_Transmit_DMA(huart, data, len);
#endif
#endif
}
