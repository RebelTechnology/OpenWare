#include <string.h>
#include <algorithm>
#include <math.h> /* for ceilf */
#include "message.h"
#include "midi.h"
#include "errorhandlers.h"
#include "MidiStatus.h"
#include "PatchRegistry.h"
#include "MidiController.h"
#include "ApplicationSettings.h"
#include "ProgramVector.h"
#include "ProgramManager.h"
#include "Storage.h"
#include "Owl.h"
#include "BootloaderStorage.h"
#include "sysex.h"
#include "crc32.h"

#ifndef USE_BOOTLOADER_MODE
#include "cmsis_os.h"
#endif

void MidiController::sendPatchParameterValues(){
  sendCc(PATCH_PARAMETER_A, (uint8_t)(getParameterValue(PARAMETER_A)>>5) & 0x7f);
  sendCc(PATCH_PARAMETER_B, (uint8_t)(getParameterValue(PARAMETER_B)>>5) & 0x7f);
  sendCc(PATCH_PARAMETER_C, (uint8_t)(getParameterValue(PARAMETER_C)>>5) & 0x7f);
  sendCc(PATCH_PARAMETER_D, (uint8_t)(getParameterValue(PARAMETER_D)>>5) & 0x7f);
  sendCc(PATCH_PARAMETER_E, (uint8_t)(getParameterValue(PARAMETER_E)>>5) & 0x7f);
}

class SendSettingsTask : public BackgroundTask {
private:
  uint8_t state;
public:
  void begin(){
    state = 0;
  }
  void loop(){
    switch(state++){
    case 0:
      midi_tx.sendPc(program.getProgramIndex());
      midi_tx.sendPatchParameterValues();
      midi_tx.sendCc(PUSHBUTTON, getButtonValue(PUSHBUTTON) ? 127 : 0);
      break;
    case 1:
      midi_tx.sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_AUDIO_RATE, settings.audio_samplingrate);
      midi_tx.sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_AUDIO_BITDEPTH, settings.audio_bitdepth);
      midi_tx.sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_AUDIO_DATAFORMAT, settings.audio_dataformat);
      midi_tx.sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_AUDIO_BLOCKSIZE, settings.audio_blocksize);
      break;
    case 2:
      midi_tx.sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_CODEC_BYPASS, settings.audio_codec_bypass);
      midi_tx.sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_CODEC_SWAP, settings.audio_codec_swaplr);
      midi_tx.sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_PC_BUTTON, settings.program_change_button);
      midi_tx.sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_BOOTLOADER_LOCK, bool(bootloader.getWriteProtectedSectors()));
      break;
    case 3:
      midi_tx.sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_INPUT_OFFSET, settings.input_offset);
      midi_tx.sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_INPUT_SCALAR, settings.input_scalar);
      midi_tx.sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_OUTPUT_OFFSET, settings.output_offset);
      midi_tx.sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_OUTPUT_SCALAR, settings.output_scalar);
      break;
    default:
      owl.setBackgroundTask(NULL);
    }
  }
};

void MidiController::sendSettings(){
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
  static SendSettingsTask task;
  owl.setBackgroundTask(&task);
}

class SendPatchNamesTask : public BackgroundTask {
private:
  uint8_t state;
public:
  void begin(){
    state = 0;
  }
  void loop(){
    if(state < registry.getNumberOfPatches()){
      midi_tx.sendPatchName(state);
      state++;
    }else{
      midi_tx.sendPc(program.getProgramIndex());
      owl.setBackgroundTask(NULL); // end this task
    }
  }
};

class SendResourceNamesTask : public BackgroundTask {
private:
  uint8_t state;
public:
  void begin(){
    state = 0;
  }
  void loop(){
    if(state < registry.getNumberOfResources()){
      Resource* resource = registry.getResource(state);
      if(resource)
	midi_tx.sendName(SYSEX_RESOURCE_NAME_COMMAND, state+MAX_NUMBER_OF_PATCHES,
			 resource->getName(), resource->getDataSize(),
			 resource->getChecksum());
      state++;
    }else{
      owl.setBackgroundTask(NULL); // end this task
    }
  }
};

void MidiController::sendPatchName(uint8_t slot){
  if(slot == 0){
    PatchDefinition* def = registry.getPatchDefinition();
    if(def)
      sendName(SYSEX_PRESET_NAME_COMMAND, slot, def->getName(), def->getBinarySize(), 0);
  }else{
    Resource* resource = registry.getPatch(slot-1);
    if(resource)
      sendName(SYSEX_PRESET_NAME_COMMAND, slot, resource->getName(), resource->getDataSize(),
	       resource->getChecksum());
  }
}

void MidiController::sendPatchNames(){
  static SendPatchNamesTask task;
  owl.setBackgroundTask(&task);
}
      
void MidiController::sendResourceNames(){
  static SendResourceNamesTask task;
  owl.setBackgroundTask(&task);
}

class SendResourceTask : public BackgroundTask {
private:
  size_t index;
  uint32_t crc;
  Resource* resource;
  static constexpr size_t msgsize = 203; // number of resource bytes we send with each SysEx
public:
  void setResource(Resource* resource){
    this->resource = resource;
  }
  void begin(){
    index = 0;
    crc = 0;
  }
  void loop(){
    if(!resource){      
      owl.setBackgroundTask(NULL); // end this task
      return;
    }
    size_t len = resource->getDataSize();
    size_t offset = msgsize*(index-1);
    uint8_t data[msgsize];
    uint8_t msg[msgsize*8/7+6];
    msg[0] = SYSEX_FIRMWARE_UPLOAD;
    uint32_t word = __REV(index);
    data_to_sysex((uint8_t*)&word, msg+1, 4);
    setProgress(offset*4095/len, "Sending");
    if(index == 0){
      // prepare and send first message with zero index and resource size
      word = __REV(resource->getDataSize());
      data_to_sysex((uint8_t*)&word, msg+6, 4);
      midi_tx.sendSysEx(msg, 11);
    }else if(offset < len){
      // data message
      size_t sz = std::min(msgsize, len-offset);
      storage.readResource(resource->getHeader(), data, offset, sz);
      offset += sz;
      // crc = crc32(data, sz, crc);
      crc = resource->getChecksum();
      sz = data_to_sysex(data, msg+6, sz);
      midi_tx.sendSysEx(msg, sz+6);
    }else{
      // last message with CRC checksum
      word = __REV(crc);
      data_to_sysex((uint8_t*)&word, msg+6, 4);
      midi_tx.sendSysEx(msg, 11);
      owl.setBackgroundTask(NULL); // end this task
      resource = NULL; // done
    }
    index++;
  }
};

void MidiController::sendResource(Resource* resource){
  static SendResourceTask task;
  task.setResource(resource);
  owl.setBackgroundTask(&task);
}

void MidiController::sendName(uint8_t cmd, uint8_t index, const char* name, size_t datasize, uint32_t crc){
  if(name != NULL){
     // make the numbers big-endian
    datasize = __REV(datasize);
    crc = __REV(crc);
    size_t len = strnlen(name, sizeof(ResourceHeader::name));
    uint8_t buf[len+3+5+5];
    buf[0] = cmd;
    buf[1] = index;
    memcpy(buf+2, name, len);
    buf[len+2] = 0;
    data_to_sysex((uint8_t*)&datasize, buf+len+3, 4);
    data_to_sysex((uint8_t*)&crc, buf+len+3+5, 4);
    sendSysEx(buf, sizeof(buf));
  }
}

void MidiController::sendPatchParameterName(PatchParameterId pid, const char* name){
  uint8_t size = strnlen(name, 24);
  uint8_t buf[size+2];
  buf[0] = SYSEX_PARAMETER_NAME_COMMAND;
  buf[1] = pid;
  memcpy(buf+2, name, size);
  sendSysEx(buf, sizeof(buf));
}

void MidiController::sendDeviceInfo(){
  sendFirmwareVersion();
  sendBootloaderVersion();
  sendProgramMessage();
  sendDeviceStats();
  sendStatus();
}

void MidiController::sendDeviceStats(){
  char buf[80];
  (void)buf;
  buf[0] = SYSEX_DEVICE_STATS;
  char* p;
  (void)p;
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
  p = stpcpy(p, (const char*)"Storage ");
  p = stpcpy(p, msg_itoa(storage.getUsedSize(), 10));
  p = stpcpy(p, (const char*)"/");
  p = stpcpy(p, msg_itoa(storage.getTotalCapacity(), 10));
  sendSysEx((uint8_t*)buf, p-buf);
#endif /* DEBUG_STORAGE */
#ifdef DEBUG_BOOTLOADER
  p = &buf[1];
  p = stpcpy(p, (const char*)"Bootloader ");
  p = stpcpy(p, getBootloaderVersion());
  if (bootloader.getWriteProtectedSectors()){
    p = stpcpy(p, (const char*)" is locked");
    if (bootloader.isWriteProtected()){
      p = stpcpy(p, (const char*)" (all sectors)");
    }
    else {
      // We can get here in very weird situations, i.e. bootloader resizing
      p = stpcpy(p, (const char*)" (some sectors)");
    }
  }
  else {
    p = stpcpy(p, (const char*)" is unlocked");
  }
  sendSysEx((uint8_t*)buf, p-buf);
#endif /* DEBUG_BOOTLOADER */
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

void MidiController::sendErrorMessage(){
  uint8_t err = getErrorStatus();
  if(err != NO_ERROR){
    char buf[64];
    buf[0] = SYSEX_PROGRAM_ERROR;
    char* p = &buf[1];
    p = stpcpy(p, (const char*)"Error 0x");
    p = stpcpy(p, msg_itoa(err, 16));
    const char* msg = getErrorMessage();
    if(msg != NULL){
      p = stpcpy(p, (const char*)" ");
      p = stpcpy(p, msg);
    }
    sendSysEx((uint8_t*)buf, p-buf);
  }
}

void MidiController::sendStatus(){
  if(getErrorStatus() == NO_ERROR)
    sendProgramStats();
  else
    sendErrorMessage();
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

void MidiController::sendBootloaderVersion(){
  char buf[16];
  buf[0] = SYSEX_BOOTLOADER_VERSION;
  char* p = &buf[1];
  p = stpcpy(p, getBootloaderVersion());
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
  char buf[32];
  buf[0] = SYSEX_DEVICE_ID;
  char* p = &buf[1];
  p = stpcpy(p, msg_itoa(HAL_GetUIDw0(), 16, 8));
  p = stpcpy(p, ":");
  p = stpcpy(p, msg_itoa(HAL_GetUIDw1(), 16, 8));
  p = stpcpy(p, ":");
  p = stpcpy(p, msg_itoa(HAL_GetUIDw2(), 16, 8));
  sendSysEx((uint8_t*)buf, p-buf);
}
