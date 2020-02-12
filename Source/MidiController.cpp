#include <string.h>
#include "device.h"
#include "message.h"
#include "midi.h"
#include "errorhandlers.h"
#include "MidiStatus.h"
#include "PatchRegistry.h"
#include "MidiController.h"
// #include "Codec.h"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"
#include "ProgramVector.h"
#include "ProgramManager.h"
#include "FlashStorage.h"
#include "Owl.h"
#include <math.h> /* for ceilf */
#ifdef USE_BLE_MIDI
#include "ble_midi.h"
#endif /* USE_BLE_MIDI */

void MidiController::sendPatchParameterValues(){
  sendCc(PATCH_PARAMETER_A, (uint8_t)(getParameterValue(PARAMETER_A)>>5) & 0x7f);
  sendCc(PATCH_PARAMETER_B, (uint8_t)(getParameterValue(PARAMETER_B)>>5) & 0x7f);
  sendCc(PATCH_PARAMETER_C, (uint8_t)(getParameterValue(PARAMETER_C)>>5) & 0x7f);
  sendCc(PATCH_PARAMETER_D, (uint8_t)(getParameterValue(PARAMETER_D)>>5) & 0x7f);
  sendCc(PATCH_PARAMETER_E, (uint8_t)(getParameterValue(PARAMETER_E)>>5) & 0x7f);
}

void MidiController::sendSettings(){
  // sendPc(settings.program_index); // TODO!
  sendPatchParameterValues();
  sendCc(PUSHBUTTON, getButtonValue(PUSHBUTTON) ? 127 : 0);
  // sendCc(LED, getLed() == NONE ? 0 : getLed() == GREEN ? 42 : 84);
  // sendCc(LEFT_INPUT_GAIN, codec.getInputGainLeft()<<2);
  // sendCc(RIGHT_INPUT_GAIN, codec.getInputGainRight()<<2);
  // sendCc(LEFT_OUTPUT_GAIN, codec.getOutputGainLeft());
  // sendCc(RIGHT_OUTPUT_GAIN, codec.getOutputGainRight());
  // sendCc(LEFT_INPUT_MUTE, codec.getInputMuteLeft() ? 127 : 0);
  // sendCc(RIGHT_INPUT_MUTE, codec.getInputMuteRight() ? 127 : 0);
  // sendCc(LEFT_OUTPUT_MUTE, codec.getOutputMuteLeft() ? 127 : 0);
  // sendCc(RIGHT_OUTPUT_MUTE, codec.getOutputMuteRight() ? 127 : 0);
  // sendCc(BYPASS, codec.getBypass() ? 127 : 0);

  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_AUDIO_RATE, settings.audio_samplingrate);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_AUDIO_BITDEPTH, settings.audio_bitdepth);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_AUDIO_DATAFORMAT, settings.audio_dataformat);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_AUDIO_BLOCKSIZE, settings.audio_blocksize);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_CODEC_BYPASS, settings.audio_codec_bypass);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_CODEC_SWAP, settings.audio_codec_swaplr);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_PC_BUTTON, settings.program_change_button);
}

void MidiController::sendPatchParameterNames(){
  // PatchProcessor* processor = patches.getActivePatchProcessor();
  // for(int i=0; i<NOF_ADC_VALUES; ++i){
  //   PatchParameterId pid = (PatchParameterId)i;
  //   const char* name = processor->getParameterName(pid);
  //   if(name != NULL)
  //     sendPatchParameterName(pid, name);
  //   else
  //     sendPatchParameterName(pid, "");
  // }
}

void MidiController::sendPatchParameterName(PatchParameterId pid, const char* name){
  uint8_t size = strnlen(name, 24);
  uint8_t buf[size+2];
  buf[0] = SYSEX_PARAMETER_NAME_COMMAND;
  buf[1] = pid;
  memcpy(buf+2, name, size);
  sendSysEx(buf, sizeof(buf));
}

void MidiController::sendPatchNames(){
  for(uint8_t i=0; i<registry.getNumberOfPatches(); ++i)
    sendPatchName(i, registry.getPatchName(i));
  sendPc(program.getProgramIndex());
}

void MidiController::sendPatchName(uint8_t index, const char* name){
  if(name != NULL){
    uint8_t size = strnlen(name, 24);
    uint8_t buf[size+2];
    buf[0] = SYSEX_PRESET_NAME_COMMAND;
    buf[1] = index;
    memcpy(buf+2, name, size);
    sendSysEx(buf, sizeof(buf));
  }
}

void MidiController::sendDeviceInfo(){
  sendFirmwareVersion();
  sendProgramMessage();
  //   sendProgramStats(); done by sendStatus() in case of no error
  sendDeviceStats();
  sendStatus();
}

void MidiController::sendDeviceStats(){
  char buf[80];
  buf[0] = SYSEX_DEVICE_STATS;
  char* p;
#ifdef DEBUG_STACK
  p = &buf[1];
  p = stpcpy(p, (const char*)"Program Stack ");
  p = stpcpy(p, msg_itoa(program.getProgramStackUsed(), 10));
  p = stpcpy(p, (const char*)"/");
  p = stpcpy(p, msg_itoa(program.getProgramStackAllocation(), 10));
  p = stpcpy(p, (const char*)" Manager ");
  p = stpcpy(p, msg_itoa(program.getManagerStackUsed(), 10));
  p = stpcpy(p, (const char*)"/");
  p = stpcpy(p, msg_itoa(program.getManagerStackAllocation(), 10));
  sendSysEx((uint8_t*)buf, p-buf);
#endif /* DEBUG_STACK */
#ifdef DEBUG_STORAGE
  p = &buf[1];
  p = stpcpy(p, (const char*)"Storage used ");
  p = stpcpy(p, msg_itoa(storage.getTotalUsedSize(), 10));
  p = stpcpy(p, (const char*)" deleted ");
  p = stpcpy(p, msg_itoa(storage.getDeletedSize(), 10));
  p = stpcpy(p, (const char*)" free ");
  p = stpcpy(p, msg_itoa(storage.getFreeSize(), 10));
  p = stpcpy(p, (const char*)" total ");
  p = stpcpy(p, msg_itoa(storage.getTotalAllocatedSize(), 10));
  sendSysEx((uint8_t*)buf, p-buf);
#endif /* DEBUG_STORAGE */
}

void MidiController::sendProgramStats(){
  char buf[64];
  buf[0] = SYSEX_PROGRAM_STATS;
  char* p = &buf[1];
#ifdef DEBUG_DWT
  p = stpcpy(p, (const char*)"CPU: ");
  float percent = (program.getCyclesPerBlock()/getProgramVector()->audio_blocksize) / (float)ARM_CYCLES_PER_SAMPLE;
  p = stpcpy(p, msg_itoa(ceilf(percent*100), 10));
  p = stpcpy(p, (const char*)"% ");
#endif /* DEBUG_DWT */
#ifdef DEBUG_STACK
  p = stpcpy(p, (const char*)"Stack: ");
  int stack = program.getProgramStackUsed();
  p = stpcpy(p, msg_itoa(stack, 10));
  p = stpcpy(p, (const char*)" Heap: ");
#else
  p = stpcpy(p, (const char*)"Memory: ");
#endif /* DEBUG_STACK */
  int mem = program.getHeapMemoryUsed();
  p = stpcpy(p, msg_itoa(mem, 10));
  sendSysEx((uint8_t*)buf, p-buf);
}

void MidiController::sendStatus(){
  char buf[64];
  buf[0] = SYSEX_PROGRAM_STATS;
  char* p = &buf[1];
  uint8_t err = getErrorStatus();
  if(err == NO_ERROR){
    sendProgramStats();
    return;
  }
  p = stpcpy(p, (const char*)"Error 0x");
  p = stpcpy(p, msg_itoa(err, 16));
  const char* msg = getErrorMessage();
  if(err != NO_ERROR && msg != NULL){
  // if(msg != NULL){
    p = stpcpy(p, (const char*)" ");
    p = stpcpy(p, msg);
  }
  sendSysEx((uint8_t*)buf, p-buf);
}

void MidiController::sendProgramMessage(){
  ProgramVector* pv = getProgramVector();
  if(pv != NULL && pv->message != NULL){
    char buf[64];
    buf[0] = SYSEX_PROGRAM_MESSAGE;
    char* p = &buf[1];
    p = stpncpy(p, pv->message, 62);
    sendSysEx((uint8_t*)buf, p-buf);
    pv->message = NULL;
  }
}

void MidiController::sendFirmwareVersion(){
  char buf[32];
  buf[0] = SYSEX_FIRMWARE_VERSION;
  char* p = &buf[1];
  p = stpcpy(p, getFirmwareVersion());
  sendSysEx((uint8_t*)buf, p-buf);
}

void MidiController::sendConfigurationSetting(const char* name, uint32_t value){
  char buf[16];
  buf[0] = SYSEX_CONFIGURATION_COMMAND;
  char* p = &buf[1];
  p = stpcpy(p, name);
  p = stpcpy(p, msg_itoa(value, 16));
  sendSysEx((uint8_t*)buf, p-buf);
}

void MidiController::sendDeviceId(){
  uint32_t* deviceId = (uint32_t*)UID_BASE;
  char buf[32];
  buf[0] = SYSEX_DEVICE_ID;
  char* p = &buf[1];
  p = stpcpy(p, msg_itoa(deviceId[0], 16, 8));
  p = stpcpy(p, ":");
  p = stpcpy(p, msg_itoa(deviceId[1], 16, 8));
  p = stpcpy(p, ":");
  p = stpcpy(p, msg_itoa(deviceId[2], 16, 8));
  sendSysEx((uint8_t*)buf, p-buf);
}

void MidiController::sendPc(uint8_t pc){
  if(midi_device_connected()){
    uint8_t packet[4] = { USB_COMMAND_PROGRAM_CHANGE,
			  (uint8_t)(PROGRAM_CHANGE | channel),
			  pc, 0 };
    write(packet, sizeof(packet));
  }
}

void MidiController::sendCc(uint8_t cc, uint8_t value){
  if(midi_device_connected()){
    uint8_t packet[4] = { USB_COMMAND_CONTROL_CHANGE,
			  (uint8_t)(CONTROL_CHANGE | channel),
			  cc, value };
    write(packet, sizeof(packet));
  }
}

void MidiController::sendNoteOff(uint8_t note, uint8_t velocity){
  if(midi_device_connected()){
    uint8_t packet[4] = { USB_COMMAND_NOTE_OFF,
			  (uint8_t)(NOTE_OFF | channel),
			  note, velocity };
    write(packet, sizeof(packet));
  }
}

void MidiController::sendNoteOn(uint8_t note, uint8_t velocity){
  if(midi_device_connected()){
    uint8_t packet[4] = { USB_COMMAND_NOTE_ON,
			  (uint8_t)(NOTE_ON | channel),
			  note, velocity };
    write(packet, sizeof(packet));
  }
}

void MidiController::sendPitchBend(uint16_t value){
  if(midi_device_connected()){
    uint8_t packet[4] = { USB_COMMAND_PITCH_BEND_CHANGE,
			  (uint8_t)(PITCH_BEND_CHANGE | channel),
			  (uint8_t)(value & 0x7f), (uint8_t)((value>>7) & 0x7f) };
    write(packet, sizeof(packet));
  }
}

/**
 * 
 */
void MidiController::sendSysEx(uint8_t* data, uint16_t size){
  /* USB-MIDI devices transmit sysex messages in 4-byte packets which
   * contain a status byte and up to 3 bytes of the message itself.
   * If the message ends with fewer than 3 bytes, a different code is
   * sent. Go through the sysex 3 bytes at a time, including the leading
   * 0xF0 and trailing 0xF7.
   */
  if(midi_device_connected()){
    uint8_t packet[4] = { USB_COMMAND_SYSEX, SYSEX, MIDI_SYSEX_MANUFACTURER,
			  uint8_t(MIDI_SYSEX_OWL_DEVICE | channel) };
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
}

void MidiController::write(uint8_t* data, uint16_t size){
#ifdef USE_MIDI_TX_BUFFER
  if(buffer.getWriteCapacity() >= size)
    buffer.push(data, size);
  else
    error(RUNTIME_ERROR, "usb tx overflow");
#else
  if(midi_device_ready()) // if not ready, packets will be missed
    midi_device_tx(data, size);
#ifdef USE_USB_HOST
  if(midi_host_ready())
    midi_host_tx(data, size);
#endif /* USE_USB_HOST */
#ifdef USE_BLE_MIDI
  ble_tx(data, size);
#endif /* USE_BLE_MIDI */
#endif
}

void MidiController::push(){
#ifdef USE_MIDI_TX_BUFFER
  size_t len = buffer.getContiguousReadCapacity() & ~0x0003;
  // round down to multiple of four
  while(len >= 4){
    if(midi_device_ready()) // if not ready, packets will be missed
      midi_device_tx(buffer.getReadHead(), len);
#ifdef USE_USB_HOST
    if(midi_host_ready())
      midi_host_tx(buffer.getReadHead(), len);
#endif /* USE_USB_HOST */
#ifdef USE_BLE_MIDI
    ble_tx(buffer.getReadHead(), len);
#endif /* USE_BLE_MIDI */
    buffer.incrementReadHead(len);
    len = buffer.getContiguousReadCapacity() & ~0x0003;
  }
#endif
}

// void MidiController::push(){
//   size_t len = buffer.getContiguousReadCapacity();
//   while(len >= 4){
//     if(midi_device_ready()) // if not ready, packets will be missed
//       midi_device_tx(buffer.getReadHead(), 4);
// #ifdef USE_USB_HOST
//     if(midi_host_ready())
//       midi_host_tx(buffer.getReadHead(), 4);
// #endif /* USE_USB_HOST */
// #ifdef USE_BLE_MIDI
//     ble_tx(buffer.getReadHead(), 4);
// #endif /* USE_BLE_MIDI */
//     buffer.incrementReadHead(4);
//     len = buffer.getContiguousReadCapacity();
//   }
// }
