#ifndef _MidiStreamReader_h_
#define _MidiStreamReader_h_

#include <inttypes.h>
#include "MidiStatus.h"
#include "MidiMessage.h"
#include "device.h"

enum MidiReaderStatus {
  READY_STATUS, INCOMPLETE_STATUS, ERROR_STATUS
};

class MidiStreamReader {
protected:
  uint8_t buffer[3];
  static const int size = sizeof(buffer);
  MidiReaderStatus status;
  unsigned char runningStatus;
  int pos;
  uint8_t cn; // Cable Number (CN) for USB MIDI 
private:
public:
  MidiStreamReader(uint8_t cn) : 
    status(READY_STATUS), 
    runningStatus(0), 
    pos(0), cn(cn<<4) {
  }

  ~MidiStreamReader(){
  }

  void clear(){
    runningStatus = buffer[0];
    pos = 0;
    status = READY_STATUS;
  }

  unsigned char* getMessage(int& length){
    length = pos;
    return buffer;
  }

  MidiMessage read(unsigned char data);
};


#endif /* _MidiStreamReader_h_ */
