#ifndef __MIDI_H
#define __MIDI_H

#include <stdbool.h>
#include <stdint.h>
#include "device.h"

#ifdef __cplusplus
 extern "C" {
#endif

   // defined in Owl.cpp
   void usbd_midi_rx(uint8_t *buffer, uint32_t length);
   // defined in usbd_midi.c
   void usbd_midi_tx(uint8_t* buffer, uint32_t length);
   uint8_t usbd_midi_connected(void);
   uint8_t usbd_midi_ready(void);
#ifdef USE_USB_HOST
   // defined in usbh_midi.c
   bool usbh_midi_connected(void);
   bool usbh_midi_ready(void);

   void usbh_midi_rx(uint8_t *buffer, uint32_t length);
   void usbh_midi_tx(uint8_t* buffer, uint32_t length);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MIDI_H */
