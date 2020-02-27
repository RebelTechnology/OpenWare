#include "MidiReceiver.h"
#include "MidiReader.h"
#include "MidiStreamReader.h"
#include "ApplicationSettings.h"
#include "errorhandlers.h"
#ifdef USE_UART_MIDI
#include "uart.h"
#endif /* USE_UART_MIDI */

#ifdef USE_USBD_MIDI
MidiReader mididevice;
#endif
#ifdef USE_USB_HOST
MidiReader midihost;
#endif /* USE_USB_HOST */
#ifdef USE_UART_MIDI
MidiStreamReader midiuart;
#endif /* USE_UART_MIDI */

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
#ifdef USE_USB_HOST
  midihost.setInputChannel(channel);
#endif
#ifdef USE_UART_MIDI
  midiuart.setInputChannel(channel);
#endif
#ifdef USE_DIGITALBUS
  bus_set_input_channel(channel);
#endif
}

void MidiReceiver::receive(){}

extern "C" {
#ifdef USE_USBD_MIDI
  // incoming data from USB device interface
  void usbd_midi_rx(uint8_t *buffer, uint32_t length){
    for(uint16_t i=0; i<length; i+=4){
      if(!mididevice.readMidiFrame(buffer+i))
	mididevice.reset();
#ifdef USE_DIGITALBUS
      else
	bus_tx_frame(buffer+i);
#endif /* USE_DIGITALBUS */
    }
  }
  // void midi_tx_usb_buffer(uint8_t* buffer, uint32_t length);
#endif

#ifdef USE_USB_HOST
  void usbh_midi_reset(void){
    midihost.reset();
    // ledstatus ^= 0x3ff003ff;
  }
  void usbh_midi_rx(uint8_t *buffer, uint32_t len){
    for(uint16_t i=0; i<len; i+=4){
      if(!midihost.readMidiFrame(buffer+i)){
	midihost.reset();
      // }else{
      // 	ledstatus ^= 0x000ffc00;
      }
    }
  }
#endif /* USE_USB_HOST */

  
#ifdef USE_UART_MIDI
  void uart_rx_callback(uint8_t* data, size_t len){
    for(uint16_t i=0; i<len; ++i){
      if(!midiuart.read(data[i])){
	error(RUNTIME_ERROR, "MIDI rx error");
	midiuart.clear();
	return;
      }
    }    
  }
#endif /* USE_UART_MIDI */
}
