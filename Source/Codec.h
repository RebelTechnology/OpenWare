#ifndef __Codec_h
#define __Codec_h

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

  void codec_init();
  void codec_bypass(int bypass);
  void codec_set_gain_in(int8_t volume);
  void codec_set_gain_out(int8_t volume);

#ifdef __cplusplus
}

class Codec {
 public:
  void begin();
  void reset();
  void start();
  void stop();
  void pause();
  void resume();
  void bypass(bool doBypass);
  void mute(bool doMute);
  void clear();
  void txrx();
  void set(uint32_t value);
  void ramp(uint32_t max);
  void setInputGain(int8_t value);
  /* int8_t getOutputGain(); */
  /* Set gain between 0 (mute) and 127 (max) */
  void setOutputGain(int8_t value);
  int32_t getMin();
  int32_t getMax();
  float getAvg();
  uint16_t getBlockSize();
};

extern Codec codec;
   
#endif

#endif /* __Codec_h */
