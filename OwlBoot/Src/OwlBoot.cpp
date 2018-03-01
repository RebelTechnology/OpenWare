#include "device.h"
#include "midi.h"
#include "MidiReader.h"
#include "MidiStatus.h"
#include "OpenWareMidiControl.h"
#include "FirmwareLoader.hpp"
#include "errorhandlers.h"

static MidiReader mididevice;
static FirmwareLoader loader;

MidiHandler::MidiHandler(){}

// MidiReader::MidiReader(){}

extern "C" {
  void error(int8_t code, const char* reason){
    // todo!
  }
  void setErrorStatus(int8_t err){}
  void setErrorMessage(int8_t err, const char* msg){}
  bool midi_error(const char* str){
    error(PROGRAM_ERROR, str);
    return false;
  }
}

void MidiHandler::handleFirmwareUploadCommand(uint8_t* data, uint16_t size){
  int32_t ret = loader.handleFirmwareUpload(data, size);
  if(ret > 0){
    // firmware upload complete: wait for run or store
    // setLed(NONE); todo!
  }else if(ret == 0){
    // toggleLed(); todo!
  }// else error
  // TODO: set screen to LOADING mode if available
  setParameterValue(PARAMETER_A, loader.index*4095/loader.size);
}

void MidiHandler::handleFlashEraseCommand(uint8_t* data, uint16_t size){
  if(size == 5){
    uint32_t sector = loader.decodeInt(data);
    // program.eraseFromFlash(sector); todo!
    loader.clear();
  }else{
    error(PROGRAM_ERROR, "Invalid FLASH ERASE command");
  }
}

void MidiHandler::handleFirmwareFlashCommand(uint8_t* data, uint16_t size){
  if(loader.isReady() && size == 5){
    uint32_t checksum = loader.decodeInt(data);
    if(checksum == loader.getChecksum()){
      // program.saveToFlash(-1, loader.getData(), loader.getSize()); // todo!
      loader.clear();
    }else{
      error(PROGRAM_ERROR, "Invalid FLASH checksum");
    }
  }else{
    error(PROGRAM_ERROR, "Invalid FLASH command");
  }
}

void MidiHandler::handleFirmwareStoreCommand(uint8_t* data, uint16_t size){
  if(loader.isReady() && size == 5){
    uint32_t slot = loader.decodeInt(data);
    if(slot > 0 && slot <= MAX_NUMBER_OF_PATCHES+MAX_NUMBER_OF_RESOURCES){
      // program.saveToFlash(slot, loader.getData(), loader.getSize()); // todo!
      loader.clear();
    }else{
      error(PROGRAM_ERROR, "Invalid program slot");
    }
  }else{
    error(PROGRAM_ERROR, "No program to store");
  }
}

void MidiHandler::handleSysEx(uint8_t* data, uint16_t size){
  if(size < 5 || data[1] != MIDI_SYSEX_MANUFACTURER)     
    return;
  if(data[2] != MIDI_SYSEX_DEVICE && data[2] != (MIDI_SYSEX_OWL_DEVICE | channel))
    // not for us
    return; // if channel == OMNI && data[2] == 0xff this message will also be processed
  switch(data[3]){
  // case SYSEX_CONFIGURATION_COMMAND:
  //   handleConfigurationCommand(data+4, size-5);
  //   break;
  // case SYSEX_DFU_COMMAND:
  //   jump_to_bootloader();
  //   break;
  case SYSEX_FIRMWARE_UPLOAD:
    handleFirmwareUploadCommand(data+1, size-2);
    break;
  // case SYSEX_FIRMWARE_RUN:
  //   handleFirmwareRunCommand(data+4, size-5);
  //   break;
  case SYSEX_FIRMWARE_STORE:
    handleFirmwareStoreCommand(data+4, size-5);
    break;
  case SYSEX_FIRMWARE_FLASH:
    handleFirmwareFlashCommand(data+4, size-5);
    break;
  case SYSEX_FLASH_ERASE:
    handleFlashEraseCommand(data+4, size-5);
    break;
  default:
    error(PROGRAM_ERROR, "Invalid SysEx Message");
    break;
  }
}

bool MidiReader::readMidiFrame(uint8_t* frame){
  switch(frame[0] & 0x0f){ // accept any cable number /  port
  case USB_COMMAND_SINGLE_BYTE:
    // Single Byte: in some special cases, an application may prefer not to use parsed MIDI events. Using CIN=0xF, a MIDI data stream may be transferred by placing each individual byte in one 32 Bit USB-MIDI Event Packet. This way, any MIDI data may be transferred without being parsed.
    if(frame[1] == 0xF7 && pos > 2){
      // suddenly found the end of our sysex as a Single Byte Unparsed
      buffer[pos++] = frame[1];
      handleSysEx(buffer, pos);
      pos = 0;
    }else if(frame[1]&0x80){
      // handleSystemRealTime(frame[1]);
    }else if(pos > 2){
      // we are probably in the middle of a sysex
      buffer[pos++] = frame[1];
    }else{
      return false;
    }
    break;
  case USB_COMMAND_SYSEX_EOX1:
    if(pos < 3 || buffer[0] != SYSEX || frame[1] != SYSEX_EOX){
      return midi_error("Invalid SysEx");
    }else if(pos >= size){
      return midi_error("SysEx buffer overflow");
    }else{
      buffer[pos++] = frame[1];
      handleSysEx(buffer, pos);
    }
    pos = 0;
    break;
  case USB_COMMAND_SYSEX_EOX2:
    if(pos < 3 || buffer[0] != SYSEX || frame[2] != SYSEX_EOX){
      return midi_error("Invalid SysEx");
    }else if(pos+2 > size){
      return midi_error("SysEx buffer overflow");
    }else{
      buffer[pos++] = frame[1];
      buffer[pos++] = frame[2];
      handleSysEx(buffer, pos);
    }
    pos = 0;
    break;
  case USB_COMMAND_SYSEX_EOX3:
    if(pos < 3 || buffer[0] != SYSEX || frame[3] != SYSEX_EOX){
      return midi_error("Invalid SysEx");
    }else if(pos+3 > size){
      return midi_error("SysEx buffer overflow");
    }else{
      buffer[pos++] = frame[1];
      buffer[pos++] = frame[2];
      buffer[pos++] = frame[3];
      handleSysEx(buffer, pos);
    }
    pos = 0;
    break;
  case USB_COMMAND_SYSEX:
    if(pos+3 > size){
      return midi_error("SysEx buffer overflow");
    }else{
      buffer[pos++] = frame[1];
      buffer[pos++] = frame[2];
      buffer[pos++] = frame[3];
    }
    break;
  }
}

void midi_device_rx(uint8_t *buffer, uint32_t length){
  for(uint16_t i=0; i<length; i+=4){
    if(!mididevice.readMidiFrame(buffer+i))
      mididevice.reset();
  }
}
