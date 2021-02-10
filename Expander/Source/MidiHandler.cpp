#include <string.h>
// #include "device.h"
// #include "owlcontrol.h"
// #include "MidiStatus.h"
// #include "OpenWareMidiControl.h"
// #include "MidiController.h"
// #include "CodecController.h"
// #include "ApplicationSettings.h"
// #include "FirmwareLoader.hpp"
// #include "ProgramManager.h"
// #include "Owl.h"
#include "MidiHandler.h"
#include "OpenWareMidiControl.h"

extern "C"  void setParameter(uint8_t pid, int16_t value);

MidiHandler::MidiHandler(){
  // memset(midi_values, 0, NOF_PARAMETERS*sizeof(uint16_t));
}

void MidiHandler::handlePitchBend(uint8_t status, uint16_t value){
  // setParameter(PARAMETER_G, ((int16_t)value - 8192)>>1);
}

void MidiHandler::handleNoteOn(uint8_t status, uint8_t note, uint8_t velocity){
  // setButton(MIDI_NOTE_BUTTON+note, velocity<<5);
}

void MidiHandler::handleNoteOff(uint8_t status, uint8_t note, uint8_t velocity){
  // setButton(MIDI_NOTE_BUTTON+note, 0);
}

void MidiHandler::handleProgramChange(uint8_t status, uint8_t pid){
}

void MidiHandler::handleControlChange(uint8_t status, uint8_t cc, uint8_t value){
  if(cc >= PATCH_PARAMETER_AA && cc <= PATCH_PARAMETER_BH)
    setParameter(PARAMETER_AA+(cc-PATCH_PARAMETER_AA), value<<5);
}

#if 0
void MidiHandler::handleConfigurationCommand(uint8_t* data, uint16_t size){
}

void MidiHandler::handleFirmwareUploadCommand(uint8_t* data, uint16_t size){
  int32_t ret = loader.handleFirmwareUpload(data, size);
  if(ret > 0){
    // firmware upload complete: wait for run or store
    setLed(NONE);
  }else if(ret == 0){
    toggleLed();
  }// else error
}

void MidiHandler::handleFirmwareRunCommand(uint8_t* data, uint16_t size){
  if(loader.isReady()){
    program.loadDynamicProgram(loader.getData(), loader.getSize());
    loader.clear();
    program.startProgram(true);
  }else{
    setErrorMessage(PROGRAM_ERROR, "No program to run");
  }      
}

void MidiHandler::handleFirmwareFlashCommand(uint8_t* data, uint16_t size){
  if(loader.isReady() && size == 5){
    uint32_t checksum = loader.decodeInt(data);
    if(checksum == loader.getChecksum()){
      program.saveProgramToFlash(-1, loader.getData(), loader.getSize());
      loader.clear();
    }else{
      setErrorMessage(PROGRAM_ERROR, "Invalid FLASH checksum");
    }
  }else{
    setErrorMessage(PROGRAM_ERROR, "Invalid FLASH command");
  }
}

void MidiHandler::handleFirmwareStoreCommand(uint8_t* data, uint16_t size){
  if(loader.isReady() && size == 5){
    uint32_t slot = loader.decodeInt(data);
    if(slot >= 0 && slot < MAX_USER_PATCHES){
      program.saveProgramToFlash(slot, loader.getData(), loader.getSize());
      loader.clear();
    }else{
      setErrorMessage(PROGRAM_ERROR, "Invalid program slot");
    }
  }else{
    setErrorMessage(PROGRAM_ERROR, "No program to store");
  }
}

void MidiHandler::handleSysEx(uint8_t* data, uint16_t size){
  if(size < 3 || 
     data[0] != MIDI_SYSEX_MANUFACTURER || 
     data[1] != MIDI_SYSEX_DEVICE)
    return;
  switch(data[2]){
  case SYSEX_CONFIGURATION_COMMAND:
    handleConfigurationCommand(data+3, size-3);
    break;
  case SYSEX_DFU_COMMAND:
    jump_to_bootloader();
    break;
  case SYSEX_FIRMWARE_UPLOAD:
    handleFirmwareUploadCommand(data, size);
    break;
  case SYSEX_FIRMWARE_RUN:
    handleFirmwareRunCommand(data+3, size-3);
    break;
  case SYSEX_FIRMWARE_STORE:
    handleFirmwareStoreCommand(data+3, size-3);
    break;
  case SYSEX_FIRMWARE_FLASH:
    handleFirmwareFlashCommand(data+3, size-3);
    break;
  }
}
#else
void MidiHandler::handleSysEx(uint8_t* data, uint16_t size){}

#endif

