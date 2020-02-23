#ifndef _MidiWriter_h_
#define _MidiWriter_h_

#include <inttypes.h>
#include "device.h"
#include "MidiMessage.h"
#include "MidiStatus.h"
#include "MidiHandler.h"

class MidiWriter {
private:
  uint8_t channel;
public:
  MidiWriter() : channel(0) {}
  void setOutputChannel(uint8_t ch){
    channel = ch;
  }
  void sendPc(uint8_t pc);
  void sendCc(uint8_t cc, uint8_t value);
  void sendPitchBend(uint16_t value);
  void sendNote(uint8_t note, uint8_t velocity);
  void sendSysEx(uint8_t* data, uint16_t size);
  void sendErrorMessage(const char* msg);
  void send(MidiMessage msg);
  
  void transmit();
};

#endif /* _MidiWriter_h_ */
