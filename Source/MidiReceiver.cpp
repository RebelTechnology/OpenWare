#include "MidiReceiver.h"
#include "MidiReader.h"
#include "MidiStreamReader.h"
#include "ApplicationSettings.h"
#include "SerialBuffer.hpp"
#include "errorhandlers.h"
#ifdef USE_UART_MIDI_RX
#include "uart_midi.h"
#endif /* USE_UART_MIDI_RX */
#ifdef USE_USB_HOST
#include "usbh_midi.h"
#endif /* USE_USB_HOST */
#ifdef USE_DIGITALBUS
#include "bus.h"
#endif

static SystemMidiReader midiSystemDevice;
static PerformanceMidiReader midiPerformanceDevice;

#ifdef USE_UART_MIDI_RX
static MidiStreamReader midiuart(4); // use cable number 4 for serial midi
#endif /* USE_UART_MIDI_RX */

static SerialBuffer<MIDI_INPUT_BUFFER_SIZE, MidiMessage> midi_rx_buffer DMA_RAM;

void MidiReceiver::init(){
#if defined USE_UART_MIDI_RX
  uart_init();
#endif /* USE_UART_MIDI_RX */
}

static void handleMessage(MidiMessage msg){
  midiPerformanceDevice.read(msg);
  midi_rx_buffer.push(msg);
}

void MidiReceiver::setCallback(void *callback){
  midiCallback = (void (*)(uint8_t, uint8_t, uint8_t, uint8_t))callback;
}

void MidiReceiver::setInputChannel(int8_t channel){
  settings.midi_input_channel = channel;
  midiSystemDevice.setInputChannel(channel);
  midiPerformanceDevice.setInputChannel(channel);
#ifdef USE_DIGITALBUS
  bus_set_input_channel(channel);
#endif
}

void MidiReceiver::receive(){
#ifdef USE_USB_HOST
  usbh_midi_push();
#endif
  while(midi_rx_buffer.notEmpty()){
      // send MIDI from all destinations to program callback
    MidiMessage msg = midi_rx_buffer.pull();
#ifdef USE_MIDI_CALLBACK
  if(midiCallback != NULL && (msg.data[0]&0x0f) >= USB_COMMAND_NOTE_OFF)
    midiCallback(msg.data[0], msg.data[1], msg.data[2], msg.data[3]);
#endif
  }
}

extern "C" {
#ifdef USE_USBD_MIDI
  // incoming data from USB device interface
  void usbd_midi_rx(uint8_t *buffer, uint32_t length){
    for(size_t i=0; i<length; i+=4){
      if(midiSystemDevice.readMidiFrame(buffer+i)){
#ifdef USE_DIGITALBUS
        bus_tx_frame(buffer+i);
#endif /* USE_DIGITALBUS */
        midi_rx_buffer.push(MidiMessage(buffer[i], buffer[i+1], buffer[i+2], buffer[i+3]));
      }
      else{
	      midiSystemDevice.reset();
      }
    }
  }
#endif

#ifdef USE_USB_HOST
  void usbh_midi_rx(uint8_t *buffer, uint32_t len){
    for(size_t i=0; i<len; i+=4) {
      handleMessage(MidiMessage(buffer[i], buffer[i+1], buffer[i+2], buffer[i+3]));
#ifdef USE_DIGITALBUS
      bus_tx_frame(buffer+i);
#endif /* USE_DIGITALBUS */
    }
  }
#endif /* USE_USB_HOST */

  
#ifdef USE_UART_MIDI_RX
  void uart_rx_callback(uint8_t* data, size_t len){
    for(size_t i=0; i<len; ++i){
      MidiMessage msg = midiuart.read(data[i]);
      if(msg.packed != 0)
        handleMessage(msg);
    }
  }
#endif /* USE_UART_MIDI_RX */
}
