#ifndef __MIDI_CONTROLLER_H
#define __MIDI_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>
#include "device.h"
#include "MidiWriter.h"
#include "OpenWareMidiControl.h"

class MidiController;
extern MidiController midi;

class MidiController : public MidiWriter {
private:
  uint8_t channel;
public:
  void setOutputChannel(uint8_t ch){
    channel = ch;
  }
  void sendSettings();
  void sendConfigurationSetting(const char* name, uint32_t value);
  void sendPatchParameterNames();
  void sendPatchParameterName(PatchParameterId pid, const char* name);
  void sendPatchParameterValues();
  void sendPatchNames();
  void sendPatchName(uint8_t index, const char* name);
  void sendDeviceInfo();
  void sendDeviceStats();
  void sendProgramStats();
  void sendStatus();
  void sendFirmwareVersion();
  void sendDeviceId();
  void sendProgramMessage();
};

#endif /* __MIDI_CONTROLLER_H */
