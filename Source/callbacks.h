#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__
#include <stdint.h>

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
  void setup(void);
  void loop(void);
  void initLed();
  void setLed(uint8_t led, uint32_t rgb);
  void setButtonValue(uint8_t ch, uint8_t value);
  void setAnalogValue(uint8_t ch, int16_t value);
  void setGateValue(uint8_t bid, int16_t value);
  void onChangePin(uint16_t pin);
  void onChangeMode(OperationMode new_mode, OperationMode old_mode);
  // sets a progress bar (if available) to a value from 0 to 4095
  void setProgress(uint16_t value, const char* reason);
  void onResourceUpdate(void);
  void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len);

  int16_t getParameterValue(uint8_t pid);
  void setParameterValue(uint8_t pid, int16_t value);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /*  __CALLBACKS_H__ */
