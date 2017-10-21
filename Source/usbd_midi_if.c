#include "usbd_midi_if.h"
#include "midi.h"

USBD_Midi_ItfTypeDef USBD_Midi_fops = {
  midi_device_rx,
};
