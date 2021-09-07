#ifndef __UART_MIDI_H
#define __UART_MIDI_H

#include <stdint.h>

#ifdef __cplusplus

 extern "C" {
#endif

  void uart_init();
  bool uart_ready();
  void uart_tx(uint8_t* data, size_t len);
  void uart_rx_callback(uint8_t* data, size_t len);

  void initiateUartRead();
#ifdef __cplusplus
}
#endif

#endif /* __UART_MIDI_H */
