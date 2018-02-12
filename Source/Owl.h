#ifndef __OWL_H__
#define __OWL_H__
#include <stdint.h>
#include "device.h"
#include "ProgramVector.h"

#ifdef __cplusplus
extern "C" {
#endif

  void setup(void);
  void loop(void);
  void updateProgramVector(ProgramVector* pv);

  uint16_t getAnalogValue(uint8_t index);
  void setAnalogValue(uint8_t ch, uint16_t value);

  void setLed(uint8_t led, uint32_t rgb);
#ifdef USE_ENCODERS
  void encoderReset(uint8_t encoder, int32_t value);
  void encoderChanged(uint8_t encoder, int32_t value);
#endif

#ifdef OWL_MAGUS
#define PORT_UNI_INPUT 1
#define PORT_UNI_OUTPUT 2
#define PORT_BI_INPUT 3
#define PORT_BI_OUTPUT 4

  void setPortMode(uint8_t index, uint8_t mode);
  uint8_t getPortMode(uint8_t index);
#endif /* OWL_MAGUS */

  int16_t getParameterValue(uint8_t index);
  void setParameterValue(uint8_t ch, int16_t value);

  void delay(uint32_t ms);

  void audioCallback(int32_t* rx, int32_t* tx, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /*  __OWL_H__ */
