#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

#include <stdint.h>
#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

  typedef enum {
    STARTUP_MODE,
    LOAD_MODE,
    RUN_MODE,
    CONFIGURE_MODE,
    STREAM_MODE,
    ERROR_MODE
  } OperationMode;

  /** functions with weak declarations that can be defined by subprojects
   *  to implement device specific behaviour */
  void onSetup();
  void onLoop();
  void setLed(uint8_t led, uint32_t rgb);
  void setButtonValue(uint8_t ch, uint8_t value);
  void setAnalogValue(uint8_t ch, int16_t value);
  void setGateValue(uint8_t bid, int16_t value);
  void onChangePin(uint16_t pin);
  void onChangeMode(uint8_t new_mode, uint8_t old_mode);
  void onStartProgram();
  void onError(int8_t code, const char* msg);
  // sets a progress bar (if available) to a value from 0 to 4095
  void setProgress(uint16_t value, const char* reason);
  void onResourceUpdate();
  void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len);

#ifdef USE_SCREEN
  void onScreenDraw();
  void defaultDrawCallback(uint8_t* pixels, uint16_t width, uint16_t height);
#endif

#ifdef __cplusplus
} /* extern C */
#endif

#endif /*  __CALLBACKS_H__ */
