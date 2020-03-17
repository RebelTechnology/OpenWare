#ifndef _MidiReader_h_
#define _MidiReader_h_

#include <inttypes.h>
#include "MidiStatus.h"
#include "MidiHandler.h"
#include "device.h"

class MidiReader : public MidiHandler {
protected:
  uint8_t buffer[MIDI_SYSEX_BUFFER_SIZE];
  static constexpr unsigned int size = sizeof(buffer);
  unsigned int pos;
public:
  MidiReader() : pos(0) {}  
  bool readMidiFrame(uint8_t* frame);
  void reset();
};

#endif /* _MidiReader_h_ */
