#include <string.h>
#include "device.h"
#include "MidiStatus.h"
#include "MidiWriter.h"
#include "OpenWareMidiControl.h"
#if defined USE_USBD_MIDI || defined USE_USBH_MIDI
#include "midi.h"
#endif
#ifdef USE_BLE_MIDI
#include "ble_midi.h"
#endif /* USE_BLE_MIDI */
#ifdef USE_UART_MIDI_TX
#include "uart_midi.h"
#endif
#include "SerialBuffer.hpp"

void MidiWriter::sendErrorMessage(const char* msg){
  char buf[64];
  buf[0] = 0x30; // error message
  char* p = &buf[1];
  p = stpncpy(p, msg, 62);
  sendSysEx((uint8_t*)buf, p-buf);
}

void MidiWriter::sendPc(uint8_t pc){
  send(MidiMessage::pc(channel, pc));
}

void MidiWriter::sendCc(uint8_t cc, uint8_t value){
  send(MidiMessage::cc(channel, cc, value));
}

void MidiWriter::sendNote(uint8_t note, uint8_t velocity){
  send(MidiMessage::note(channel, note, velocity));
}

void MidiWriter::sendPitchBend(uint16_t value){
  send(MidiMessage::pb(channel, value));
}

void MidiWriter::sendSysEx(uint8_t* data, uint16_t size){
  /* USB-MIDI devices transmit sysex messages in 4-byte packets which
   * contain a status byte and up to 3 bytes of the message itself.
   * If the message ends with fewer than 3 bytes, a different code is
   * sent. Go through the sysex 3 bytes at a time, including the leading
   * 0xF0 and trailing 0xF7.
   */
  MidiMessage packet(USB_COMMAND_SYSEX, SYSEX, MIDI_SYSEX_MANUFACTURER,
		     uint8_t(MIDI_SYSEX_OWL_DEVICE | channel));
  send(packet);
  int count = size/3;
  uint8_t* src = data;
  while(count-- > 0){
    packet.data[1] = (*src++ & 0x7f);
    packet.data[2] = (*src++ & 0x7f);
    packet.data[3] = (*src++ & 0x7f);
    send(packet);
  }
  count = size % 3;
  switch(count){
  case 0:
    packet.data[0] = USB_COMMAND_SYSEX_EOX1;
    packet.data[1] = SYSEX_EOX;
    packet.data[2] = 0;
    packet.data[3] = 0;
    break;
  case 1:
    packet.data[0] = USB_COMMAND_SYSEX_EOX2;
    packet.data[1] = (*src++ & 0x7f);
    packet.data[2] = SYSEX_EOX;
    packet.data[3] = 0;
    break;
  case 2:
    packet.data[0] = USB_COMMAND_SYSEX_EOX3;
    packet.data[1] = (*src++ & 0x7f);
    packet.data[2] = (*src++ & 0x7f);
    packet.data[3] = SYSEX_EOX;
    break;
  }
  send(packet);
}

class MidiTransmitter {
public:
  virtual void write(MidiMessage msg){};
  virtual void transmit(){};
};

#ifdef USE_USBD_MIDI
class UsbdMidiTransmitter : public MidiTransmitter {
private:
  SerialBuffer<MIDI_OUTPUT_BUFFER_SIZE> buffer;
public:
  void write(MidiMessage msg){
    if(buffer.getContiguousWriteCapacity() <= 4)
      transmit();
    buffer.push(msg.data, 4);
  }
  void transmit(){
    size_t len = buffer.getContiguousReadCapacity();
    if(len >= 4 && usbd_midi_ready()){
      usbd_midi_tx(buffer.getReadHead(), len);
      buffer.incrementReadHead(len);
      // len = buffer.getContiguousReadCapacity();
    }
  }
};
UsbdMidiTransmitter usbd_midi;
#endif

#ifdef USE_USBH_MIDI
class UsbhMidiTransmitter : public MidiTransmitter {
private:
  SerialBuffer<MIDI_OUTPUT_BUFFER_SIZE> buffer;
public:
  void write(MidiMessage msg){
    buffer.push(msg.data, 4);
  }
  void transmit(){
    size_t len = buffer.getContiguousReadCapacity();
    if(len >= 4 && usbh_midi_ready()){
      usbh_midi_tx(buffer.getReadHead(), len);
      buffer.incrementReadHead(len);
      // len = buffer.getContiguousReadCapacity();
    }
  }
};
UsbhMidiTransmitter usbh_midi;
#endif

#ifdef USE_BLE
class BleMidiTransmitter : public MidiTransmitter {
private:
  SerialBuffer<MIDI_OUTPUT_BUFFER_SIZE> buffer;
public:
  void write(MidiMessage msg){
    buffer.push(msg.data, 4);
  }
  void transmit(){
    size_t len = buffer.getContiguousReadCapacity();
    if(len >= 4){
      ble_tx(buffer.getReadHead(), len);
      buffer.incrementReadHead(len);
    }
  }
};
BleMidiTransmitter ble_midi;
#endif

#ifdef USE_UART_MIDI_TX
class UartMidiTransmitter : public MidiTransmitter {
private:
  SerialBuffer<MIDI_OUTPUT_BUFFER_SIZE> buffer;
public:
  void write(MidiMessage msg){
    buffer.push(msg.data+1, 3); // skip USB byte
  }
  void transmit(){
    size_t len = buffer.getContiguousReadCapacity();
    if(len && uart_ready()){
      uart_tx(buffer.getReadHead(), len);
      buffer.incrementReadHead(len);
    }
  }
};
UartMidiTransmitter uart_midi DMA_RAM;
#endif

void MidiWriter::send(MidiMessage msg){
#ifdef USE_USBD_MIDI
  usbd_midi.write(msg);
#endif
#ifdef USE_USBH_MIDI
  usbh_midi.write(msg);
#endif
#ifdef USE_BLE
  ble_midi.write(msg);
#endif
#ifdef USE_UART_MIDI_TX
  uart_midi.write(msg);
#endif
// #ifdef USE_DIGITALBUS
//   bus_tx_frame(msg.data);
// #endif /* USE_DIGITALBUS */
}

void MidiWriter::transmit(){
#ifdef USE_USBD_MIDI
  usbd_midi.transmit();
#endif
#ifdef USE_USBH_MIDI
  usbh_midi.transmit();
#endif
#ifdef USE_BLE
  ble_midi.transmit();
#endif
#ifdef USE_UART_MIDI_TX
  uart_midi.transmit();
#endif
}
