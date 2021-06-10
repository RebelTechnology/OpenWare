#ifndef _MIDIWRITER_H_
#define _MIDIWRITER_H_

#include <inttypes.h>
#include "MidiStatus.h"
#include "uart_midi.h"
#include "midi.h"
#include "device.h"

class MidiWriter { // : public MidiInterface {
public:

  MidiWriter() {}

  void controlChange(uint8_t channel, uint8_t cc, uint8_t value){
    uint8_t packet[4] = { USB_COMMAND_CONTROL_CHANGE,
    			  (uint8_t)(CONTROL_CHANGE | channel),
    			  cc, value };
    write(packet, sizeof(packet));
  }

  void programChange(uint8_t channel, uint8_t pc){
    uint8_t packet[4] = { USB_COMMAND_PROGRAM_CHANGE,
    			  (uint8_t)(PROGRAM_CHANGE | channel),
    			  pc, 0 };
    write(packet, sizeof(packet));
  }

  void sysex(uint8_t manufacturer, uint8_t device, uint8_t *data, uint16_t size){
    uint8_t packet[4] = { USB_COMMAND_SYSEX, SYSEX, manufacturer, device };
    write(packet, sizeof(packet));
    int count = size/3;
    uint8_t* src = data;
    while(count-- > 0){
      packet[1] = (*src++ & 0x7f);
      packet[2] = (*src++ & 0x7f);
      packet[3] = (*src++ & 0x7f);
      write(packet, sizeof(packet));
    }
    count = size % 3;
    switch(count){
    case 0:
      packet[0] = USB_COMMAND_SYSEX_EOX1;
      packet[1] = SYSEX_EOX;
      packet[2] = 0;
      packet[3] = 0;
      break;
    case 1:
      packet[0] = USB_COMMAND_SYSEX_EOX2;
      packet[1] = (*src++ & 0x7f);
      packet[2] = SYSEX_EOX;
      packet[3] = 0;
      break;
    case 2:
      packet[0] = USB_COMMAND_SYSEX_EOX3;
      packet[1] = (*src++ & 0x7f);
      packet[2] = (*src++ & 0x7f);
      packet[3] = SYSEX_EOX;
      break;
    }
    write(packet, sizeof(packet));
  }

  void write(uint8_t *data, uint8_t length){
// #ifdef USE_USB
//     midi_tx_usb_buffer(data, length);
// #endif
#ifdef USE_DIGITALBUS
    bus_write(data, length);
#endif
  }

#if 0

  void channelPressure(uint8_t channel, uint8_t value){
    write(0xd0 | channel);
    write(value & 0x7f);
  }

  void startSong(){
    write(0xfa);
  }

  void stopSong(){
    write(0xfc);
  }

  void continueSong(){
    write(0xfb);
  }

  void midiClock(){
    write(0xf8);
  }

  void pitchBend(uint8_t channel, uint16_t value){
    write(0xe0 | channel);
    write(value & 0x7f);
    write((value>>7) & 0x7f);
  }

  void noteOff(uint8_t channel, uint8_t note, uint8_t velocity){
    write(0x80 | channel);
    write(note & 0x7f);
    write(velocity & 0x7f);
  }

  void noteOn(uint8_t channel, uint8_t note, uint8_t velocity){
    write(0x90 | channel);
    write(note & 0x7f);
    write(velocity & 0x7f);
  }

  void afterTouch(uint8_t channel, uint8_t note, uint8_t value){
    write(POLY_KEY_PRESSURE | channel);
    write(note & 0x7f);
    write(value & 0x7f);
  }

  void pitchBend(uint16_t value){
    write(0xe0 | channel);
    write(value & 0x7f);
    write((value>>7) & 0x7f);
  }

  void controlChange(uint8_t cc, uint8_t value){
    write(0xb0 | channel);
    write(cc & 0x7f);
    write(value & 0x7f);
  }

  void programChange(uint8_t pg){
    write(0xc0 | channel);
    write(pg & 0x7f);
  }

  void noteOff(uint8_t note, uint8_t velocity){
    write(0x80 | channel);
    write(note & 0x7f);
    write(velocity & 0x7f);
  }

  void noteOn(uint8_t note, uint8_t velocity){
    write(0x90 | channel);
    write(note & 0x7f);
    write(velocity & 0x7f);
  }

  void afterTouch(uint8_t note, uint8_t value){
    write(POLY_KEY_PRESSURE | channel);
    write(note & 0x7f);
    write(value & 0x7f);
  }

  void allNotesOff(){
    controlChange(channel, 0x7b, 00);
  }
#endif // 0

};


#endif /* _MIDIWRITER_H_ */
