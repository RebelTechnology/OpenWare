#include "device.h"
#include "midi.h"
#include "MidiReader.h"
#include "MidiStatus.h"
#include "OpenWareMidiControl.h"
#include "FirmwareLoader.hpp"
#include "errorhandlers.h"
#include "eepromcontrol.h"
#include "MidiController.h"

static SystemMidiReader midi_rx;
MidiController midi_tx;
FirmwareLoader loader;
ProgramManager program;

extern "C" int testButton();

MidiHandler::MidiHandler(){}
ProgramManager::ProgramManager(){}
void ProgramManager::exitProgram(bool isr){}
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

void sendMessage(){
  if(getErrorStatus() != NO_ERROR)
    message = getErrorMessage() == NULL ? "Error" : getErrorMessage();
  if(message != NULL){
    char buffer[64];
    buffer[0] = SYSEX_PROGRAM_MESSAGE;
    char* p = &buffer[1];
    p = stpncpy(p, message, 62);
    midi_tx.sendSysEx((uint8_t*)buffer, p-buffer);
    message = NULL;
  }
}

void eraseFromFlash(uint8_t sector){
  eeprom_unlock();
  if(sector == 0xff){
    extern char _FLASH_STORAGE_BEGIN, _FLASH_STORAGE_END;
    uint32_t address = (uint32_t)&_FLASH_STORAGE_BEGIN;
    uint32_t sector = FLASH_SECTOR_4;
    while (address < (uint32_t)&_FLASH_STORAGE_END) {
      eeprom_erase_sector(sector++, FLASH_BANK_1);
      address += FLASH_SECTOR_SIZE;
    }
    setMessage("Erased patch storage");
    led_green();
  }else{
    eeprom_erase_sector(sector, FLASH_BANK_1);
    setMessage("Erased flash sector");
    led_green();
  }
  eeprom_lock();
}

void saveToFlash(uint8_t sector, void* data, uint32_t length){
  // TODO!
  if(sector == FIRMWARE_SECTOR && length <= (3*128)*1024){
    eeprom_unlock();
    eeprom_erase_sector(FLASH_SECTOR_1, FLASH_BANK_1);
    if(length > 128*1024){
      eeprom_erase_sector(FLASH_SECTOR_2, FLASH_BANK_1);
      if(length > (128+128)*1024){
        eeprom_erase_sector(FLASH_SECTOR_3, FLASH_BANK_1);
      }
    }
    extern char _BOOTLOADER_END;
    eeprom_write_block((uint32_t)&_BOOTLOADER_END, data, length);
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

  void setup(){
    led_green();
    midi_tx.setOutputChannel(MIDI_OUTPUT_CHANNEL);
    midi_rx.setInputChannel(MIDI_INPUT_CHANNEL);
    setMessage("OWL Bootloader Ready");
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
    midi_tx.transmit();
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

void device_reset(){
  *OWLBOOT_MAGIC_ADDRESS = 0;
  NVIC_SystemReset();
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
    error(RUNTIME_ERROR, "Bootloader OK");
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
      sendMessage();
    case SYSEX_FIRMWARE_VERSION:
      midi_tx.sendFirmwareVersion();
      break;
    case SYSEX_DEVICE_ID:
      midi_tx.sendDeviceId();
      break;
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
