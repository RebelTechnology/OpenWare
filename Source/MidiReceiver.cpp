#include "MidiReceiver.h"
#include "MidiReader.h"
#include "MidiStreamReader.h"
#include "ApplicationSettings.h"
#include "SerialBuffer.hpp"
#include "errorhandlers.h"
#ifdef USE_UART_MIDI
#include "uart.h"
#endif /* USE_UART_MIDI */
#ifdef USE_USB_HOST
#include "usbh_midi.h"
#endif /* USE_USB_HOST */

#ifdef USE_USBD_MIDI
static MidiReader mididevice;
#endif
#ifdef USE_UART_MIDI
static MidiStreamReader midiuart(4); // use cable number 4 for serial midi
#endif /* USE_UART_MIDI */

static SerialBuffer<MIDI_INPUT_BUFFER_SIZE, MidiMessage> midi_rx_buffer;

void MidiReceiver::init(){
#ifdef USE_UART_MIDI
  uart_init();
#endif /* USE_UART_MIDI */
}

void MidiReceiver::handleMidiMessage(MidiMessage msg){
  // process MIDI from usbd
}

void MidiReceiver::forwardMidiMessage(MidiMessage msg){
  // send MIDI from all destinations to program callback
#ifdef USE_MIDI_CALLBACK
  if(midiCallback != NULL && (msg.data[0]&0x0f) >= USB_COMMAND_NOTE_OFF)
    midiCallback(msg.data[0], msg.data[1], msg.data[2], msg.data[3]);
#endif
}

void MidiReceiver::setCallback(void *callback){
  midiCallback = (void (*)(uint8_t, uint8_t, uint8_t, uint8_t))callback;
}

void MidiReceiver::setInputChannel(int8_t channel){
  settings.midi_input_channel = channel;
  mididevice.setInputChannel(channel);
#ifdef USE_DIGITALBUS
  bus_set_input_channel(channel);
#endif
}

void MidiReceiver::receive(){
#ifdef USE_USB_HOST
  usbh_midi_push();
#endif
  while(midi_rx_buffer.notEmpty())
    forwardMidiMessage(midi_rx_buffer.pull());
}

extern "C" {
#ifdef USE_USBD_MIDI
  // incoming data from USB device interface
  void usbd_midi_rx(uint8_t *buffer, uint32_t length){
    for(size_t i=0; i<length; i+=4){
      if(mididevice.readMidiFrame(buffer+i)){
#ifdef USE_DIGITALBUS
	bus_tx_frame(buffer+i);
#endif /* USE_DIGITALBUS */
	midi_rx_buffer.push(MidiMessage(buffer[i], buffer[i+1], buffer[i+2], buffer[i+3]));
      }else{
	mididevice.reset();
      }
    }
  }
#endif

#ifdef USE_USB_HOST
  void usbh_midi_reset(void){
    // midihost.reset();
    // ledstatus ^= 0x3ff003ff;
  }
  void usbh_midi_rx(uint8_t *buffer, uint32_t len){
    for(size_t i=0; i<len; i+=4)
      midi_rx_buffer.push(MidiMessage(buffer[i], buffer[i+1], buffer[i+2], buffer[i+3]));
  }
#endif /* USE_USB_HOST */

  
#ifdef USE_UART_MIDI
  void uart_rx_callback(uint8_t* data, size_t len){
    for(size_t i=0; i<len; ++i){
      MidiMessage msg = midiuart.read(data[i]);
      if(msg.packed != 0)
	midi_rx_buffer.push(msg);
    }
  }
#endif /* USE_UART_MIDI */
}
