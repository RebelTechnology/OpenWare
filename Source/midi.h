#ifndef __MIDI_H
#define __MIDI_H

#include <stdint.h>
#include "device.h"

#ifdef __cplusplus
 extern "C" {
#endif

   // defined in Owl.cpp
   void midi_device_rx(uint8_t *buffer, uint32_t length);
   // defined in usbd_midi.c
   void midi_device_tx(uint8_t* buffer, uint32_t length);
   uint8_t midi_device_connected(void);
   uint8_t midi_device_ready(void);
#ifdef USE_USB_HOST
   // defined in usbh_midi.c
   uint8_t midi_host_connected(void);
   uint8_t midi_host_ready(void);

   void midi_host_rx(uint8_t *buffer, uint32_t length);
   void midi_host_tx(uint8_t* buffer, uint32_t length);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MIDI_H */
