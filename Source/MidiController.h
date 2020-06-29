#ifndef __MIDI_CONTROLLER_H
#define __MIDI_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>
#include "device.h"
#include "MidiMessage.h"
#include "MidiWriter.h"
#include "OpenWareMidiControl.h"

class MidiController;
extern MidiController midi_tx;

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
  
  void handleMidiMessage(MidiMessage msg); // process MIDI from usbd
  void forwardMidiMessage(MidiMessage msg); // send MIDI from all destinations to program callback
};

#endif /* __MIDI_CONTROLLER_H */
