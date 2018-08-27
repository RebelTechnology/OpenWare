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

  int16_t getAnalogValue(uint8_t index);
  void setAnalogValue(uint8_t ch, int16_t value);
  void setGateValue(uint8_t bid, int16_t value);

  void setLed(uint8_t led, uint32_t rgb);

#ifdef USE_ENCODERS
  int16_t getEncoderValue(uint8_t encoder);
  void encoderReset(uint8_t encoder, int16_t value);
  void encoderChanged(uint8_t encoder, int16_t value);
#endif

#ifdef USE_MIDI_CALLBACK
  void midi_send(uint8_t port, uint8_t status, uint8_t d1, uint8_t d2);
#endif

  void midiSetInputChannel(int8_t channel);
  void midiSetOutputChannel(int8_t channel);
			   
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
  uint8_t getButtonValue(uint8_t ch);
  void setButtonValue(uint8_t ch, uint8_t value);
  uint16_t getSampleCounter();
  void delay(uint32_t ms);

  void audioCallback(int32_t* rx, int32_t* tx, uint16_t size);

  void jump_to_bootloader(void);  

#ifdef __cplusplus
}
#endif

#endif /*  __OWL_H__ */
