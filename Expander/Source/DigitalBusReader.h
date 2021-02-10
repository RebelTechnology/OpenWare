#ifndef _DigitalBusReader_h_
#define _DigitalBusReader_h_

#include "DigitalBusHandler.h"

class DigitalBusReader : public DigitalBusHandler {
public:
  // read a 4-byte data frame
  bool readBusFrame(uint8_t* frame);
  void reset();
protected:
  uint8_t txuid = NO_UID;
  uint32_t datalen = 0;
};

#endif /* _DigitalBusReader_h_ */
