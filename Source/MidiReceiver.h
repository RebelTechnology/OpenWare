#ifndef __MIDI_RECEIVER_H
#define __MIDI_RECEIVER_H

#include <stdbool.h>
#include <stdint.h>
#include "device.h"
#include "MidiMessage.h"
#include "OpenWareMidiControl.h"

class MidiReceiver;
extern MidiReceiver midi_rx;

class MidiReceiver {
private:
  void (*midiCallback)(uint8_t port, uint8_t status, uint8_t, uint8_t) = NULL;
public:
  void init();
  void receive();
  void setCallback(void *callback);
  void setInputChannel(int8_t channel);
};

#endif /* __MIDI_RECEIVER_H */
