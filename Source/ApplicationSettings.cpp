#include "ApplicationSettings.h"
// #include "eepromcontrol.h"
#include "MidiStatus.h"
#include "PatchRegistry.h"
#include <string.h>

// #define APPLICATION_SETTINGS_ADDR ADDR_FLASH_SECTOR_1
// #define APPLICATION_SETTINGS_SECTOR FLASH_Sector_1

void ApplicationSettings::init(){
  checksum = sizeof(*this) ^ 0xf0f0f0f0;
  if(settingsInFlash())
    loadFromFlash();
  else
    reset();
}

void ApplicationSettings::reset(){
  audio_codec_swaplr = false;
  audio_codec_bypass = false;
  audio_samplingrate = AUDIO_SAMPLINGRATE;
  audio_bitdepth = AUDIO_BITDEPTH;
  audio_dataformat = AUDIO_DATAFORMAT;
  audio_blocksize = AUDIO_BLOCK_SIZE;
  audio_input_gain = AUDIO_INPUT_GAIN;
  audio_output_gain = AUDIO_OUTPUT_GAIN;
  program_index = DEFAULT_PROGRAM;
  program_change_button = true;
#ifdef USE_DIGITALBUS
  bus_enabled = DIGITAL_BUS_ENABLED;
  bus_forward_midi = DIGITAL_BUS_FORWARD_MIDI;
#endif
  input_offset = AUDIO_INPUT_OFFSET;
  input_scalar = AUDIO_INPUT_SCALAR;
  output_offset = AUDIO_OUTPUT_OFFSET;
  output_scalar = AUDIO_OUTPUT_SCALAR;
  midi_input_channel = MIDI_INPUT_CHANNEL;
  midi_output_channel = MIDI_OUTPUT_CHANNEL;
#ifdef USE_TLC5946
  leds_brightness = LEDS_BRIGHTNESS;
#endif
}

bool ApplicationSettings::settingsInFlash(){
  // return eeprom_read_word(APPLICATION_SETTINGS_ADDR) == checksum;
  // return false;
  ResourceHeader* resource = registry.getResource(APPLICATION_SETTINGS_RESOURCE_INDEX);
  if(resource == NULL)
    return false;
  ApplicationSettings* data = (ApplicationSettings*)((uint8_t*)resource + sizeof(ResourceHeader));
  if(data->checksum != checksum)
    return false;
  return true;
}

void ApplicationSettings::loadFromFlash(){
  // eeprom_read_block(APPLICATION_SETTINGS_ADDR, this, sizeof(*this));
  ResourceHeader* resource = registry.getResource(APPLICATION_SETTINGS_RESOURCE_INDEX);
  uint8_t* data = (uint8_t*)resource + sizeof(ResourceHeader);
  if(resource != NULL){
    memcpy(this, data, sizeof(*this));
  }
}

void ApplicationSettings::saveToFlash(){
  uint16_t totalsize = sizeof(ResourceHeader) + sizeof(*this);
  uint8_t buffer[totalsize];
  ResourceHeader* resource = (ResourceHeader*)buffer;
  resource->magic = 0XDADADEED;
  resource->size = sizeof(*this);
  strcpy(resource->name, APPLICATION_SETTINGS_NAME);  
  memcpy(buffer+sizeof(ResourceHeader), this, sizeof(*this));
  registry.store(APPLICATION_SETTINGS_RESOURCE_INDEX, buffer, totalsize);
}
