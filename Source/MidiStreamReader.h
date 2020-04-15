#ifndef _MidiStreamReader_h_
#define _MidiStreamReader_h_

#include <inttypes.h>
#include "MidiStatus.h"
#include "MidiHandler.h"
#include "device.h"

enum MidiReaderStatus {
  READY_STATUS, INCOMPLETE_STATUS, ERROR_STATUS
};

class MidiStreamReader : public MidiHandler {
protected:
  uint8_t buffer[MIDI_INPUT_BUFFER_SIZE];
  static const int size = MIDI_INPUT_BUFFER_SIZE;
  MidiReaderStatus status;
  unsigned char runningStatus;
  int pos;
private:
public:
  MidiStreamReader() : 
    status(READY_STATUS), 
    runningStatus(0), 
    pos(0) {
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

  MidiReaderStatus read(unsigned char data);
};


#endif /* _MidiStreamReader_h_ */
