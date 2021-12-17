#ifndef __ApplicationSettings_H__
#define __ApplicationSettings_H__

#include <inttypes.h>
#include "device.h"

class ApplicationSettings {
public:
  uint32_t checksum;
  uint32_t audio_samplingrate;
  uint8_t audio_bitdepth;
  uint8_t audio_dataformat;
  uint16_t audio_blocksize;
  uint8_t audio_input_gain;
  uint8_t audio_output_gain;
  bool audio_codec_swaplr;
  bool audio_codec_bypass;
  uint8_t program_index;
  bool program_change_button;
#ifdef USE_DIGITALBUS
  bool bus_enabled;
  bool bus_forward_midi;
#endif
  uint32_t input_offset;
  uint32_t input_scalar;
  uint32_t output_offset;
  uint32_t output_scalar;
  uint8_t midi_input_channel;
  uint8_t midi_output_channel;
#ifdef USE_TLC5946
  uint8_t leds_brightness;
#endif
#ifdef OWL_PEDAL
  uint8_t expression_mode;
#endif
public:
  void init();
  void reset();
  bool settingsInFlash();
  void loadFromFlash();
  void saveToFlash();
};

extern ApplicationSettings settings;

#endif // __ApplicationSettings_H__
