#include "device.h"
#include "midi.h"
#include "MidiReader.h"
#include "MidiStatus.h"
#include "OpenWareMidiControl.h"
#include "FirmwareLoader.hpp"
#include "errorhandlers.h"
#include "eepromcontrol.h"
#include "MidiController.h"
#include "Storage.h"
#include "usb_device.h"
#include "ProgramVector.h"

static SystemMidiReader midi_rx;
MidiController midi_tx;
FirmwareLoader loader;
ProgramManager program;
ProgramVector staticVector;

MidiHandler::MidiHandler(){}
ProgramManager::ProgramManager(){}
void ProgramManager::exitProgram(bool isr){}
ProgramVector* getProgramVector() { return &staticVector; }
void setParameterValue(uint8_t ch, int16_t value){}
void SystemMidiReader::reset(){}
void Owl::setOperationMode(uint8_t mode){}

const char* getFirmwareVersion(){ 
  return (const char*)(HARDWARE_VERSION " " FIRMWARE_VERSION) ;
}

#define FIRMWARE_SECTOR 0xff

void led_off(){
#ifdef USE_LED
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
#endif
}

void led_green(){
#ifdef USE_LED
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
#endif
}

void led_red(){
#ifdef USE_LED
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
#endif
}

void led_toggle(){
#ifdef USE_LED
  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
#endif
}

const char* message = NULL;
extern "C" void setMessage(const char* msg){
  message = msg;
}

void sendMessage(uint8_t cmd, const char* msg){  
  char buffer[64];
  buffer[0] = cmd;
  char* p = &buffer[1];
  p = stpncpy(p, msg, 62);
  midi_tx.sendSysEx((uint8_t*)buffer, p-buffer);
}

void sendMessage(){
  if(getErrorStatus() != NO_ERROR){
    message = getErrorMessage() == NULL ? "Error" : getErrorMessage();
  }else if(getProgramVector()->message != NULL){
    message = getProgramVector()->message;
    getProgramVector()->message = NULL;
  }
  if(message != NULL){
    char buffer[64];
    buffer[0] = SYSEX_PROGRAM_MESSAGE;
    char* p = &buffer[1];
    p = stpncpy(p, message, 62);
    midi_tx.sendSysEx((uint8_t*)buffer, p-buffer);
    message = NULL;
  }
}

void setProgress(uint16_t value, const char* msg){
  // debugMessage(msg, (int)(100*value/4095));
  led_toggle();
}

Resource* getResource(uint8_t slot){
  size_t len = storage.getNumberOfResources();
  size_t resource_index = MAX_NUMBER_OF_PATCHES;
  for(size_t i=0; i<len; ++i){
    Resource* resource = storage.getResource(i);
    if(resource->isValid()){
      if(resource->isPatch()){
	if(resource->getSlot() == slot)
	  return resource;
      }else if(resource_index++ == slot){
	return resource;
      }
    }
  }
  return NULL;
}

void eraseFromFlash(uint8_t sector){
  if(sector == 0xff){
#ifdef USE_NOR_FLASH
    storage.erase(RESOURCE_PORT_MAPPED);
#endif
    storage.erase(RESOURCE_MEMORY_MAPPED);
    sendMessage(SYSEX_PROGRAM_MESSAGE, "Erased storage");
    led_green();
  }else{
    Resource* resource = getResource(sector);
    if(resource != NULL){
      storage.eraseResource(resource);
      if(sector < MAX_NUMBER_OF_PATCHES)
	sendMessage(SYSEX_PROGRAM_MESSAGE, "Erased patch");
      else
	sendMessage(SYSEX_PROGRAM_MESSAGE, "Erased resource");      
    }
  }
}

void saveToFlash(uint8_t sector, void* data, uint32_t length){
  if(sector == FIRMWARE_SECTOR && length <= (3*128)*1024){
    eeprom_unlock();
    extern char _BOOTLOADER_END;
    uint32_t addr = (uint32_t)&_BOOTLOADER_END;
    eeprom_erase(addr, length);
    eeprom_write_block(addr, data, length);
    eeprom_lock();
  }else{
    error(RUNTIME_ERROR, "Firmware too big");
  }
}

extern "C" {

  volatile int8_t errorcode = NO_ERROR;
  static char* errormessage = NULL;
  int8_t getErrorStatus(){
    return errorcode;
  }

  void setErrorStatus(int8_t err){
    errorcode = err;
    if(err == NO_ERROR)
      errormessage = NULL;
    else
      led_red();
  }

  void error(int8_t err, const char* msg){
    setErrorStatus(err);
    errormessage = (char*)msg;
  }

  const char* getErrorMessage(){
    return errormessage;
  }

  bool midi_error(const char* str){
    error(PROGRAM_ERROR, str);
    return false;
  }

  void sendResourceNames(){
    size_t len = storage.getNumberOfResources();
    size_t resource_index = MAX_NUMBER_OF_PATCHES;
    for(size_t i=0; i<len; ++i){
      Resource* resource = storage.getResource(i);
      if(resource->isValid()){
	if(resource->isPatch()){
	  midi_tx.sendName(SYSEX_PRESET_NAME_COMMAND, resource->getSlot(), resource->getName(),
			   resource->getDataSize(), storage.getChecksum(resource));
	}else{
	  midi_tx.sendName(SYSEX_RESOURCE_NAME_COMMAND, resource_index++, resource->getName(),
			   resource->getDataSize(), storage.getChecksum(resource));
	}
      }
      midi_tx.transmit();
      HAL_Delay(10);
    }
  }

  static volatile bool send_resource_names = false;

  void setup(){
    MX_USB_DEVICE_Init();
    led_green();
    midi_tx.setOutputChannel(MIDI_OUTPUT_CHANNEL);
    midi_rx.setInputChannel(MIDI_INPUT_CHANNEL);
    storage.init();
    getProgramVector()->message = NULL;
    sendMessage(SYSEX_PROGRAM_MESSAGE, "OWL Bootloader Ready");
  }

  void loop(void){
#ifdef USE_LED
    // flash 3 times on startup
    static uint32_t counter = 3*1200;
    if(counter){
      switch(counter-- % 1200){
      case 600:
	led_red();
	break;
      case 0:
	led_green();
	break;
      default:
	HAL_Delay(1);
	break;
      }
    }
#endif
    if(send_resource_names){
      send_resource_names = false;
      sendResourceNames();
    }
    midi_tx.transmit();
    device_watchdog_tickle();
  }

}

void MidiHandler::handleFirmwareUploadCommand(uint8_t* data, uint16_t size){
  int32_t ret = loader.handleFirmwareUpload(data, size);
  if(ret > 0){
    setMessage("Firmware upload complete");
    // firmware upload complete: wait for run or store
    // setLed(NONE); todo!
    led_green();
  }else if(ret == 0){
    setMessage("Firmware upload in progress");
    led_toggle();
    // toggleLed(); todo!
  }else{
    error(RUNTIME_ERROR, "Firmware upload error");
  }
  // setParameterValue(PARAMETER_A, loader.index*4095/loader.size);
}

void MidiHandler::handleFlashEraseCommand(uint8_t* data, uint16_t size){
  if(size == 5){
    uint32_t sector = loader.decodeInt(data);
    eraseFromFlash(sector);
  }else if(size == 0){
    eraseFromFlash(0xff);
  }else{
    error(PROGRAM_ERROR, "Invalid FLASH ERASE command");
  }
}

void MidiHandler::handleFirmwareFlashCommand(uint8_t* data, uint16_t size){
  if(loader.isReady() && size == 5){
    uint32_t checksum = loader.decodeInt(data);
    if(checksum == loader.getChecksum()){
      saveToFlash(FIRMWARE_SECTOR, loader.getData(), loader.getDataSize());
      loader.clear();
    }else{
      error(PROGRAM_ERROR, "Invalid FLASH checksum");
    }
  }else{
    error(PROGRAM_ERROR, "Invalid FLASH command");
  }
}

void MidiHandler::handleFirmwareStoreCommand(uint8_t* data, uint16_t size){
  error(RUNTIME_ERROR, "Invalid STORE command");
}

void MidiHandler::handleSysEx(uint8_t* data, uint16_t size){
  if(size < 5 || data[1] != MIDI_SYSEX_MANUFACTURER)     
    return;
  if(data[2] != MIDI_SYSEX_OMNI_DEVICE && data[2] != (MIDI_SYSEX_OWL_DEVICE | channel))
    // not for us
    return;
  switch(data[3]){
  // case SYSEX_CONFIGURATION_COMMAND:
  //   handleConfigurationCommand(data+4, size-5);
  //   break;
  case SYSEX_DEVICE_RESET_COMMAND:
    device_reset();
    break;
  case SYSEX_BOOTLOADER_COMMAND:
#ifdef USE_DFU_BOOTLOADER
    sendMessage(SYSEX_PROGRAM_MESSAGE, "Enter DFU bootloader");
    device_dfu();
#else
    error(RUNTIME_ERROR, "Bootloader OK");
    setErrorStatus(NO_ERROR);
#endif
    break;
  case SYSEX_FIRMWARE_UPLOAD:
    handleFirmwareUploadCommand(data+1, size-2);
    break;
  // case SYSEX_FIRMWARE_RUN:
  //   handleFirmwareRunCommand(data+4, size-5);
  //   break;
  case SYSEX_FIRMWARE_STORE:
    handleFirmwareStoreCommand(data+4, size-5);
    break;
  case SYSEX_FIRMWARE_FLASH:
    handleFirmwareFlashCommand(data+4, size-5);
    break;
  case SYSEX_FLASH_ERASE:
    handleFlashEraseCommand(data+4, size-5);
    break;
  default:
    error(PROGRAM_ERROR, "Invalid SysEx Message");
    break;
  }
}

bool SystemMidiReader::readMidiFrame(uint8_t* frame){
  switch(frame[0] & 0x0f){ // accept any cable number /  port
  case USB_COMMAND_SINGLE_BYTE:
    // Single Byte: in some special cases, an application may prefer not to use parsed MIDI events. Using CIN=0xF, a MIDI data stream may be transferred by placing each individual byte in one 32 Bit USB-MIDI Event Packet. This way, any MIDI data may be transferred without being parsed.
    if(frame[1] == 0xF7 && pos > 2){
      // suddenly found the end of our sysex as a Single Byte Unparsed
      buffer[pos++] = frame[1];
      handleSysEx(buffer, pos);
      pos = 0;
    }else if(frame[1]&0x80){
      // handleSystemRealTime(frame[1]);
    }else if(pos > 2){
      // we are probably in the middle of a sysex
      buffer[pos++] = frame[1];
    }else{
      return false;
    }
    break;
  case USB_COMMAND_SYSEX_EOX1:
    if(pos < 3 || buffer[0] != SYSEX || frame[1] != SYSEX_EOX){
      return midi_error("Invalid SysEx");
    }else if(pos >= size){
      return midi_error("SysEx buffer overflow");
    }else{
      buffer[pos++] = frame[1];
      handleSysEx(buffer, pos);
    }
    pos = 0;
    break;
  case USB_COMMAND_SYSEX_EOX2:
    if(pos < 3 || buffer[0] != SYSEX || frame[2] != SYSEX_EOX){
      return midi_error("Invalid SysEx");
    }else if(pos+2 > size){
      return midi_error("SysEx buffer overflow");
    }else{
      buffer[pos++] = frame[1];
      buffer[pos++] = frame[2];
      handleSysEx(buffer, pos);
    }
    pos = 0;
    break;
  case USB_COMMAND_SYSEX_EOX3:
    if(pos < 3 || buffer[0] != SYSEX || frame[3] != SYSEX_EOX){
      return midi_error("Invalid SysEx");
    }else if(pos+3 > size){
      return midi_error("SysEx buffer overflow");
    }else{
      buffer[pos++] = frame[1];
      buffer[pos++] = frame[2];
      buffer[pos++] = frame[3];
      handleSysEx(buffer, pos);
    }
    pos = 0;
    break;
  case USB_COMMAND_SYSEX:
    if(pos+3 > size){
      return midi_error("SysEx buffer overflow");
    }else{
      buffer[pos++] = frame[1];
      buffer[pos++] = frame[2];
      buffer[pos++] = frame[3];
    }
    break;
  case USB_COMMAND_CONTROL_CHANGE:
    if((frame[1]&0xf0) != CONTROL_CHANGE)
      return false;
    handleControlChange(frame[1], frame[2], frame[3]);
    break;
  }
  return true;
}

void MidiHandler::handleControlChange(uint8_t status, uint8_t cc, uint8_t value){
  // if(channel != MIDI_OMNI_CHANNEL && channel != getChannel(status))
  //   return;
  switch(cc){
  case REQUEST_SETTINGS:
    switch(value){
    case 0:
    case 127:
      sendMessage();
      // midi_tx.sendDeviceInfo();
      break;
    case SYSEX_RESOURCE_NAME_COMMAND:
      // midi_tx.sendResourceNames();
      // break;
    case SYSEX_PRESET_NAME_COMMAND:
      // midi_tx.sendPatchNames();
      send_resource_names = true;
      break;
    case SYSEX_FIRMWARE_VERSION:
      midi_tx.sendFirmwareVersion();
      break;
    case SYSEX_DEVICE_ID:
      midi_tx.sendDeviceId();
      break;
    case SYSEX_DEVICE_STATS:
      midi_tx.sendDeviceStats();
      break;
    // case SYSEX_BOOTLOADER_VERSION:
    //   midi_tx.sendBootloaderVersion();
    //   break;
    case SYSEX_PROGRAM_MESSAGE:
      sendMessage();
      break;
    }
    break;
  }
}

void usbd_midi_rx(uint8_t *buffer, uint32_t length){
  for(uint16_t i=0; i<length; i+=4){
    if(!midi_rx.readMidiFrame(buffer+i))
      midi_rx.reset();
  }
}
