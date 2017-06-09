#ifndef __Codec_h
#define __Codec_h

#include <stdint.h>

class Codec {
 public:
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
  int32_t getMin();
  int32_t getMax();
  float getAvg();
 private:
};

extern Codec codec;
#endif /* __Codec_h */
