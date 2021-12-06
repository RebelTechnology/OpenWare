#ifndef __OWL_H__
#define __OWL_H__
#include <stdint.h>
#include "device.h"
#include "callbacks.h"
#include "ProgramVector.h"

#define RED_COLOUR    0x3ff00000
#define GREEN_COLOUR  0x000ffc00
#define BLUE_COLOUR   0x000003ff
#define WHITE_COLOUR  0x3fffffff
#define YELLOW_COLOUR 0x3ffffc00
#define CYAN_COLOUR   0x000fffff
#define NO_COLOUR     0x00000000
#define COLOUR_LEVEL9 0x2ffbfeff
#define COLOUR_LEVEL8 0x1ff7fdff
#define COLOUR_LEVEL7 0x0ff3fcff
#define COLOUR_LEVEL6 0x07f1fc7f
#define COLOUR_LEVEL5 0x03f0fc3f

#ifdef __cplusplus
extern "C" {
#endif

  int16_t getAnalogValue(uint8_t index);
  const char* getDeviceName();
  const char* getFirmwareVersion();
  const char* getBootloaderVersion();
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

  int16_t getParameterValue(uint8_t index);
  void setParameterValue(uint8_t ch, int16_t value);
  uint8_t getButtonValue(uint8_t ch);
  uint16_t getSampleCounter();
  void delay(uint32_t ms);

  void audioCallback(int32_t* rx, int32_t* tx, uint16_t size);

  void jump_to_bootloader(void);  
  void device_reset(void);
  void MX_USB_HOST_Process(void);

#ifdef __cplusplus
} /* extern C */

class BackgroundTask {
 public:
  virtual void begin(){}
  virtual void loop() = 0;
  virtual void end(){}
};

class Owl {
 private:
  volatile uint8_t operationMode = STARTUP_MODE;
  BackgroundTask* backgroundTask = NULL;
  void (*messageCallback)(const char* msg, size_t len) = NULL;
  
 public:
  void setup();
  void loop();
  uint8_t getOperationMode();
  void setOperationMode(uint8_t mode);
  void setBackgroundTask(BackgroundTask* bt);
  void setMessageCallback(void* callback);
  void handleMessage(const char* msg, size_t len);
};
extern Owl owl;

#endif

#endif /*  __OWL_H__ */
