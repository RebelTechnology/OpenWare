#ifndef _MidiReader_h_
#define _MidiReader_h_

#include <inttypes.h>
#include "MidiStatus.h"
#include "MidiHandler.h"

#ifndef MIDI_MAX_MESSAGE_SIZE
#define MIDI_MAX_MESSAGE_SIZE 64 // also used for bus messages and bus data
#endif

class MidiReader : public MidiHandler {
protected:
  uint8_t buffer[MIDI_MAX_MESSAGE_SIZE];
  static const unsigned int size = MIDI_MAX_MESSAGE_SIZE;
  unsigned int pos;
public:
  MidiReader() : pos(0) {}  
  bool readMidiFrame(uint8_t* frame);
  void reset();
};

#endif /* _MidiReader_h_ */
