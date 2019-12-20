#ifndef __BLE_MIDI_H__
#define __BLE_MIDI_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

  void ble_init();
  void ble_tx(uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* __BLE_MIDI_H__ */
