#ifndef _MIDIWRITER_H_
#define _MIDIWRITER_H_

#include <inttypes.h>
// #include <stdio.h>
// #include "usart.h"
#include "MidiStatus.h"
#include <stdio.h>

class MidiWriter { // : public MidiInterface {
public:

  MidiWriter(int ch) : channel(ch) {}

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

  void controlChange(uint8_t channel, uint8_t cc, uint8_t value){
    write(0xb0 | channel);
    write(cc & 0x7f);
    write(value & 0x7f);
  }

  void programChange(uint8_t channel, uint8_t pg){
    write(0xc0 | channel);
    write(pg & 0x7f);
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

  void sysex(uint8_t manufacturer, uint8_t *data, uint16_t length){
    write(SYSEX);
    write(manufacturer & 0x7f);
    for(int i=0; i<length; ++i)
      write(data[i] & 0x7f);
    write(SYSEX_EOX);
  }

  void write(uint8_t *data, uint8_t length){
    // Serial1.write(data, length);
    for(int i=0; i<length; ++i)
      write(data[i]);
    // uart_transmit_bytes(data, length );
  }

  void write(uint8_t data){
    /* Wait until end of previous transmit */
    // while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    // /* send data */
    // USART_SendData(USART_PERIPH, data);
    serialputchar(data);
  }

private:
  uint8_t channel;

};


#endif /* _MIDIWRITER_H_ */
