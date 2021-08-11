#ifndef _MidiMessage_h_
#define _MidiMessage_h_

#include "MidiStatus.h"

class MidiMessage {
 public:
  union {
    uint32_t packed;
    uint8_t data[4];
  };
  MidiMessage():packed(0){}
  MidiMessage(uint32_t msg): packed(msg){}
  MidiMessage(uint8_t port, uint8_t d0, uint8_t d1, uint8_t d2){
    data[0] = port;
    data[1] = d0;
    data[2] = d1;
    data[3] = d2;
  }
  uint8_t getPort(){
    return data[0] >> 4;
  }
  uint8_t getChannel(){
    return (data[1] & MIDI_CHANNEL_MASK);
  }
  uint8_t getStatus(){
    return (data[1] & MIDI_STATUS_MASK);
  }
  uint8_t getSize(){
    uint8_t sz;
    switch(data[0] & 0x0f){
    case USB_COMMAND_SINGLE_BYTE:
    case USB_COMMAND_SYSEX_EOX1:
      sz = 1;
      break;
    case USB_COMMAND_PROGRAM_CHANGE:
    case USB_COMMAND_CHANNEL_PRESSURE:
    case USB_COMMAND_2BYTE_SYSTEM_COMMON:
    case USB_COMMAND_SYSEX_EOX2:
      sz = 2;
      break;
    case USB_COMMAND_NOTE_OFF:
    case USB_COMMAND_NOTE_ON:
    case USB_COMMAND_POLY_KEY_PRESSURE:
    case USB_COMMAND_CONTROL_CHANGE:
    case USB_COMMAND_PITCH_BEND_CHANGE:
    case USB_COMMAND_SYSEX:
    case USB_COMMAND_SYSEX_EOX3:
    case USB_COMMAND_3BYTE_SYSTEM_COMMON:
      sz = 3;
      break;
    case USB_COMMAND_MISC:
    case USB_COMMAND_CABLE_EVENT:
    default:
      sz = 0;
      break;
    }
    return sz;
  }
  uint8_t getNote(){
    return data[2];
  }
  uint8_t getVelocity(){
    return data[3];
  }
  uint8_t getControllerNumber(){
    return data[2];
  }
  uint8_t getControllerValue(){
    return data[3];
  }
  uint8_t getChannelPressure(){
    return data[2];
  }
  uint8_t getProgramChange(){
    return data[1];
  }
  int16_t getPitchBend(){
    int16_t pb = (data[2] | (data[3]<<7)) - 8192;
    return pb;
  }
  bool isNoteOn(){
    return ((data[1] & MIDI_STATUS_MASK) == NOTE_ON) && getVelocity() != 0;
  }
  bool isNoteOff(){
    return ((data[1] & MIDI_STATUS_MASK) == NOTE_OFF) || (((data[1] & MIDI_STATUS_MASK) == NOTE_ON) && getVelocity() == 0);
  }
  bool isSysEx(){
    return data[0] == USB_COMMAND_SYSEX ||
      data[0] == USB_COMMAND_SYSEX_EOX1 ||
      data[0] == USB_COMMAND_SYSEX_EOX2 ||
      data[0] == USB_COMMAND_SYSEX_EOX3;
  }      
  bool isControlChange(){
    return (data[1] & MIDI_STATUS_MASK) == CONTROL_CHANGE;
  }
  bool isProgramChange(){
    return (data[1] & MIDI_STATUS_MASK) == PROGRAM_CHANGE;
  }
  bool isChannelPressure(){
    return (data[1] & MIDI_STATUS_MASK) == CHANNEL_PRESSURE;
  }
  bool isPitchBend(){
    return (data[1] & MIDI_STATUS_MASK) == PITCH_BEND_CHANGE;
  }
  static MidiMessage cc(uint8_t ch, uint8_t cc, uint8_t value){
    return MidiMessage(USB_COMMAND_CONTROL_CHANGE, CONTROL_CHANGE|(ch&0xf), cc&0x7f, value&0x7f);
  }
  static MidiMessage pc(uint8_t ch, uint8_t pc){
    return MidiMessage(USB_COMMAND_PROGRAM_CHANGE, PROGRAM_CHANGE|(ch&0xf), pc&0x7f, 0);
  }
  static MidiMessage pb(uint8_t ch, int16_t bend){
    bend += 8192;
    return MidiMessage(USB_COMMAND_PITCH_BEND_CHANGE, PITCH_BEND_CHANGE|(ch&0xf), bend&0x7f, (bend>>7)&0x7f);
  }
  static MidiMessage note(uint8_t ch, uint8_t note, uint8_t velocity){
    if(velocity == 0)
      return MidiMessage(USB_COMMAND_NOTE_OFF, NOTE_OFF|(ch&0xf), note&0x7f, velocity&0x7f);
    else
      return MidiMessage(USB_COMMAND_NOTE_ON, NOTE_ON|(ch&0xf), note&0x7f, velocity&0x7f);
  }
  static MidiMessage cp(uint8_t ch, uint8_t value){
    return MidiMessage(USB_COMMAND_CHANNEL_PRESSURE, CHANNEL_PRESSURE|(ch&0xf), value&0x7f, 0);
  }
};

#endif /* _MidiMessage_h_ */
