#include "MidiStreamReader.h"
#include "errorhandlers.h"

MidiMessage MidiStreamReader::read(unsigned char data){
  if(pos == size || status == READY_STATUS)
    clear();
  buffer[pos++] = data;
  MidiMessage ret;
  switch(buffer[0] & MIDI_STATUS_MASK){
    // two byte messages
  case PROGRAM_CHANGE:
    if(pos == 2){
      status = READY_STATUS;
      ret = MidiMessage(USB_COMMAND_PROGRAM_CHANGE|cn, buffer[0], buffer[1], 0);
    }else{
      status = INCOMPLETE_STATUS;
    }
    break;
  case CHANNEL_PRESSURE:
    if(pos == 2){
      status = READY_STATUS;
      ret = MidiMessage(USB_COMMAND_CHANNEL_PRESSURE|cn, buffer[0], buffer[1], 0);
    }else{
      status = INCOMPLETE_STATUS;
    }
    break;
    // three byte messages
  case NOTE_OFF:
    if(pos == 3){
      status = READY_STATUS;
      ret = MidiMessage(USB_COMMAND_NOTE_OFF|cn, buffer[0], buffer[1], buffer[2]);
    }else{
      status = INCOMPLETE_STATUS;
    }
    break;
  case NOTE_ON:
    if(pos == 3){
      status = READY_STATUS;
      ret = MidiMessage(USB_COMMAND_NOTE_ON|cn, buffer[0], buffer[1], buffer[2]);
    }else{
      status = INCOMPLETE_STATUS;
    }
    break;
  case POLY_KEY_PRESSURE:
    if(pos == 3){
      status = READY_STATUS;
      ret = MidiMessage(USB_COMMAND_POLY_KEY_PRESSURE|cn, buffer[0], buffer[1], buffer[2]);
    }else{
      status = INCOMPLETE_STATUS;
    }
    break;
  case CONTROL_CHANGE:
    if(pos == 3){
      status = READY_STATUS;
      ret = MidiMessage(USB_COMMAND_CONTROL_CHANGE|cn, buffer[0], buffer[1], buffer[2]);
    }else{
      status = INCOMPLETE_STATUS;
    }
    break;
  case PITCH_BEND_CHANGE:
    if(pos == 3){
      status = READY_STATUS;
      ret = MidiMessage(USB_COMMAND_PITCH_BEND_CHANGE|cn, buffer[0], buffer[1], buffer[2]);
    }else{
      status = INCOMPLETE_STATUS;
    }
    break;
  case SYSTEM_COMMON:
    switch(buffer[0]){
    case TIME_CODE_QUARTER_FRAME:
    case RESERVED_F4:
    case RESERVED_F9:
    case TUNE_REQUEST:
    case TIMING_CLOCK:
    case START:
    case CONTINUE:
    case STOP:
    case RESERVED_FD:
    case ACTIVE_SENSING:
    case SYSTEM_RESET:
      // one byte messages
      status = READY_STATUS;
      ret = MidiMessage(USB_COMMAND_SINGLE_BYTE|cn, buffer[0], 0, 0);
      break;
    case SYSEX:
      if(data == SYSEX_EOX || (data >= STATUS_BYTE && pos > 1)){
	// SysEx message may be terminated by a status byte different from SYSEX_EOX
	buffer[pos-1] = SYSEX_EOX;
	status = READY_STATUS;
	switch(pos){
	case 1:
	  ret = MidiMessage(USB_COMMAND_SYSEX_EOX1|cn, buffer[0], 0, 0);
	case 2:
	  ret = MidiMessage(USB_COMMAND_SYSEX_EOX2|cn, buffer[0], buffer[1], 0);
	case 3:
	  ret = MidiMessage(USB_COMMAND_SYSEX_EOX3|cn, buffer[0], buffer[1], buffer[2]);
	}
	if(data != SYSEX_EOX)
	  buffer[0] = data; // save status byte for next message - will be saved
      }else if(pos == 3){
	ret = MidiMessage(USB_COMMAND_SYSEX|cn, buffer[0], buffer[1], buffer[2]);	
      }else{
	status = INCOMPLETE_STATUS;
      }
      break;
    case SYSEX_EOX: // receiving SYSEX_EOX on its own is really an error
    default:
      error(RUNTIME_ERROR, "Invalid MIDI System Common message");
      clear();
      break;
    }
    break;
  default:
    if(pos == 1 && data < STATUS_BYTE && runningStatus >= STATUS_BYTE){
      // MIDI running status: this message is missing the status byte, re-use previous status
      buffer[pos++] = data;
      buffer[0] = runningStatus;
    }else{
      error(RUNTIME_ERROR, "Invalid MIDI message");
      clear();
    }
  }
  return ret;
}
