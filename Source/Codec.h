#ifndef __Codec_h
#define __Codec_h

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

  void codec_init();
  void codec_bypass(int bypass);
  void codec_set_gain_in(int8_t volume);
  void codec_set_gain_out(int8_t volume);
  void codec_mute(bool doMute);
  uint8_t codec_read(uint8_t reg);
#ifdef USE_WM8731
  void codec_write(uint8_t reg, uint16_t data);
#else
  void codec_write(uint8_t reg, uint8_t data);
#endif
  void audioCallback(int32_t* rx, int32_t* tx, uint16_t size);

#ifdef __cplusplus
}

class Codec {
 public:
  void init();
  void reset();
  void start();
  void stop();
  void pause();
  void resume();
  void bypass(bool doBypass);
  void mute(bool doMute);
  void clear();
  void txrx();
  void set(int32_t value);
  void ramp(uint32_t max);
  void setInputGain(int8_t value);
  /* int8_t getOutputGain(); */
  /* Set gain between 0 (mute) and 127 (max) */
  void setOutputGain(int8_t value);
  void setHighPass(bool hpf);
  int32_t getMin();
  int32_t getMax();
  float getAvg();
  uint16_t getBlockSize();
  size_t getSampleCounter();
};

extern Codec codec;
   
#endif

#endif /* __Codec_h */
