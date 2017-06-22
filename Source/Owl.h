#ifndef __OWL_H__
#define __OWL_H__
#include <stdint.h>
#include "ProgramVector.h"

#ifdef __cplusplus
extern "C" {
#endif

  void setup(void);
  void loop(void);
  void updateProgramVector(ProgramVector* pv);

  uint16_t getAnalogValue(uint8_t index);
  void setAnalogValue(uint8_t ch, uint16_t value);

  void encoderReset(uint8_t encoder, int32_t value);
  void encoderChanged(uint8_t encoder, int32_t value);

  int16_t getParameterValue(uint8_t index);
  void setParameterValue(uint8_t ch, int16_t value);

  void delay(uint32_t ms);

  void audioCallback(int32_t* rx, int32_t* tx, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /*  __OWL_H__ */
