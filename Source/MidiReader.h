#ifndef _MidiReader_h_
#define _MidiReader_h_

#include <inttypes.h>
#include "MidiStatus.h"
#include "MidiHandler.h"
#include "device.h"

class MidiReader : public MidiHandler {
protected:
  uint8_t buffer[MIDI_INPUT_BUFFER_SIZE];
  static constexpr unsigned int size = MIDI_INPUT_BUFFER_SIZE;
  unsigned int pos;
  void (*midiCallback)(uint8_t port, uint8_t status, uint8_t, uint8_t);
public:
  /* MidiReader() : pos(0), midiCallback(NULL) {}   */
  MidiReader() : pos(0), midiCallback(0) {}  
  bool readMidiFrame(uint8_t* frame);
  void reset();
#ifdef USE_MIDI_CALLBACK
  void setCallback(void *callback);
#endif /* USE_MIDI_CALLBACK */
};

#endif /* _MidiReader_h_ */
