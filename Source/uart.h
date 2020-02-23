#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

  void uart_init();
  bool uart_ready();
  void uart_tx(uint8_t* data, size_t len);
  void uart_rx_callback(uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* __UART_H__ */
