#ifndef __MIDI_H
#define __MIDI_H

#include <stdbool.h>
#include <stdint.h>
#include "device.h"

#ifdef __cplusplus
 extern "C" {
#endif

#ifdef USE_USBD_MIDI
   uint8_t usbd_midi_connected(void);
   uint8_t usbd_midi_ready(void);
   void usbd_midi_rx(uint8_t *buffer, uint32_t length);
   void usbd_midi_tx(uint8_t* buffer, uint32_t length);
#endif

#ifdef USE_USB_HOST
   bool usbh_midi_connected(void);
   bool usbh_midi_ready(void);
   void usbh_midi_rx(uint8_t *buffer, uint32_t length);
   void usbh_midi_tx(uint8_t* buffer, uint32_t length);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MIDI_H */
