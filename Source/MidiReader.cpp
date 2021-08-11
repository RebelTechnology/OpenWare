#include "MidiReader.h"
#include "bus.h"
#include "errorhandlers.h"

bool midi_error(const char* str){
  error(PROGRAM_ERROR, str);
  return false;
}

bool PerformanceMidiReader::readMidiFrame(uint8_t* frame){
  // handle Note and CC messages only
  // ignore first byte
  switch(frame[1] & 0xf0){ // accept any channel
  case NOTE_OFF:
    handleNoteOff(frame[1], frame[2], frame[3]);
    break;
  case NOTE_ON:
    if(frame[3] == 0)
      handleNoteOff(frame[1], frame[2], frame[3]);
    else
      handleNoteOn(frame[1], frame[2], frame[3]);
    break;
  case CONTROL_CHANGE:
    handleControlChange(frame[1], frame[2], frame[3]);
    break;
  default:
    return false; // Invalid USB MIDI message, ignore
    break;
  }
  return true;
}

bool SystemMidiReader::readMidiFrame(uint8_t* frame){
  // apparently no running status in USB MIDI frames
  // The Cable Number (CN) is a value ranging from 0x0 to 0xF indicating the number assignment of the Embedded MIDI Jack associated with the endpoint that is transferring the data.
  switch(frame[0] & 0x0f){ // accept any cable number /  port
  case USB_COMMAND_MISC:
  case USB_COMMAND_CABLE_EVENT:
    return false;
    break;
  case USB_COMMAND_SINGLE_BYTE:
    // Single Byte: in some special cases, an application may prefer not to use parsed MIDI events. Using CIN=0xF, a MIDI data stream may be transferred by placing each individual byte in one 32 Bit USB-MIDI Event Packet. This way, any MIDI data may be transferred without being parsed.
    if(frame[1] == 0xF7 && pos > 2){
      // suddenly found the end of our sysex as a Single Byte Unparsed
      buffer[pos++] = frame[1];
      handleSysEx(buffer, pos);
      pos = 0;
    }else if(frame[1]&0x80){
      handleSystemRealTime(frame[1]);
    }else if(pos > 2){
      // we are probably in the middle of a sysex
      buffer[pos++] = frame[1];
    }else{
      return false;
    }
    break;
  case USB_COMMAND_2BYTE_SYSTEM_COMMON:
    if((frame[1]&0xf0) != SYSTEM_COMMON)
      return false;
    handleSystemCommon(frame[1], frame[2]);
    break;
  case USB_COMMAND_3BYTE_SYSTEM_COMMON:
    if((frame[1]&0xf0) != SYSTEM_COMMON)
      return false;
    handleSystemCommon(frame[1], frame[2], frame[3]);
    break;
  case USB_COMMAND_SYSEX_EOX1:
    if(pos < 3 || buffer[0] != SYSEX || frame[1] != SYSEX_EOX){
      return false; // Invalid SysEx message, ignore
    }else if(pos+1 > size){
      return midi_error("SysEx buffer overflow");
    }else{
      buffer[pos++] = frame[1];
      handleSysEx(buffer, pos);
    }
    pos = 0;
    break;
  case USB_COMMAND_SYSEX_EOX2:
    if(pos < 3 || buffer[0] != SYSEX || frame[2] != SYSEX_EOX){
      return false; // Invalid SysEx message, ignore
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
      return false; // Invalid SysEx message, ignore
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
  case USB_COMMAND_PROGRAM_CHANGE:
    if((frame[1]&0xf0) != PROGRAM_CHANGE)
      return false;
    handleProgramChange(frame[1], frame[2]);
    break;
  case USB_COMMAND_CHANNEL_PRESSURE:
    if((frame[1]&0xf0) != CHANNEL_PRESSURE)
      return false;
    handleChannelPressure(frame[1], frame[2]);
    break;
  case USB_COMMAND_NOTE_OFF:
    if((frame[1]&0xf0) != NOTE_OFF)
      return false;
    handleNoteOff(frame[1], frame[2], frame[3]);
    break;
  case USB_COMMAND_NOTE_ON:
    if((frame[1]&0xf0) != NOTE_ON)
      return false;
    if(frame[3] == 0)
      handleNoteOff(frame[1], frame[2], frame[3]);
    else
      handleNoteOn(frame[1], frame[2], frame[3]);
    break;
  case USB_COMMAND_POLY_KEY_PRESSURE:
    if((frame[1]&0xf0) != POLY_KEY_PRESSURE)
      return false;
    handlePolyKeyPressure(frame[1], frame[2], frame[3]);
    break;
  case USB_COMMAND_CONTROL_CHANGE:
    if((frame[1]&0xf0) != CONTROL_CHANGE)
      return false;
    handleControlChange(frame[1], frame[2], frame[3]);
    break;
  case USB_COMMAND_PITCH_BEND_CHANGE:
    if((frame[1]&0xf0) != PITCH_BEND_CHANGE)
      return false;
    handlePitchBend(frame[1], frame[2] | (frame[3]<<7));
    break;
  default:
    return false; // Invalid USB MIDI message, ignore
    break;
  }
  return true;
}

void SystemMidiReader::reset(){
  pos = 0;
}
