#ifndef __KX122_H__
#define __KX122_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

  void kx122_setup();
  void kx122_rx_callback(int16_t* data, size_t len);
  void kx122_set_range(uint8_t range);
  void kx122_drdy();
  void kx122_cplt();

  /* void kx122_rx_poll(int32_t* data, size_t len); */

#ifdef __cplusplus
}
#endif

#endif /* __KX122_H__ */
