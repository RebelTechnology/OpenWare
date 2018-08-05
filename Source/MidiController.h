#ifndef __MIDI_CONTROLLER_H
#define __MIDI_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>
#include "OpenWareMidiControl.h"
#include "SerialBuffer.hpp"

class MidiController;
extern MidiController midi;

class MidiController {
private:
  uint8_t channel;
  SerialBuffer<256> buffer;
public:
  void setOutputChannel(uint8_t ch){
    channel = ch;
  }
  void sendPc(uint8_t pc);
  void sendCc(uint8_t cc, uint8_t value);
  void sendPitchBend(uint16_t value);
  void sendNoteOn(uint8_t note, uint8_t velocity);
  void sendNoteOff(uint8_t note, uint8_t velocity);

  void sendSysEx(uint8_t* data, uint16_t size);
  void sendSettings();
  void sendConfigurationSetting(const char* name, uint32_t value);
  void sendPatchParameterNames();
  void sendPatchParameterName(PatchParameterId pid, const char* name);
  void sendPatchParameterValues();
  void sendPatchNames();
  void sendPatchName(uint8_t index);
  void sendDeviceInfo();
  void sendDeviceStats();
  void sendProgramStats();
  void sendStatus();
  void sendFirmwareVersion();
  void sendDeviceId();
  void sendSelfTest();
  void sendProgramMessage();

  void write(uint8_t* data, uint16_t size);
  void push();
};

#endif /* __MIDI_CONTROLLER_H */
