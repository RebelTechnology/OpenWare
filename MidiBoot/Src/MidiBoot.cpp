#include "device.h"
#include "midi.h"
#include "MidiReader.h"
#include "MidiStatus.h"
#include "OpenWareMidiControl.h"
#include "FirmwareLoader.hpp"
#include "errorhandlers.h"
#include "eepromcontrol.h"
#include "MidiController.h"

static MidiReader mididevice;
MidiController midi;
FirmwareLoader loader;
ProgramManager program;

MidiHandler::MidiHandler(){}
ProgramManager::ProgramManager(){}
void ProgramManager::exitProgram(bool isr){}
void setParameterValue(uint8_t ch, int16_t value){}
void MidiReader::reset(){}

const char* getFirmwareVersion(){ 
  return (const char*)(HARDWARE_VERSION " " FIRMWARE_VERSION) ;
}

#define FIRMWARE_SECTOR 0xff

void led_off(){
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
}

void led_green(){
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
}

void led_red(){
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
}

void led_toggle(){
  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
}

const char* message = NULL;
void setMessage(const char* msg){
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
    midi.sendSysEx((uint8_t*)buffer, p-buffer);
    message = NULL;
  }
}

void eraseFromFlash(uint8_t sector){
  eeprom_unlock();
  if(sector == 0xff){
    eeprom_erase_sector(FLASH_SECTOR_7);
    eeprom_erase_sector(FLASH_SECTOR_8);
    eeprom_erase_sector(FLASH_SECTOR_9);
    eeprom_erase_sector(FLASH_SECTOR_10);
    eeprom_erase_sector(FLASH_SECTOR_11);
    setMessage("Erased patch storage");
    led_green();
  }else{
    eeprom_erase_sector(sector);
    setMessage("Erased flash sector");
    led_green();
  }
  eeprom_lock();
}

void saveToFlash(uint8_t sector, void* data, uint32_t length){
  if(sector == FIRMWARE_SECTOR && length <= (64+3*128)*1024){
    eeprom_unlock();
    eeprom_erase_sector(FLASH_SECTOR_4);
    if(length > 64*1024){
      eeprom_erase_sector(FLASH_SECTOR_5);
      if(length > (64+128)*1024){
	eeprom_erase_sector(FLASH_SECTOR_6);
      }
    }
    eeprom_write_block(ADDR_FLASH_SECTOR_4, data, length);
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
    midi.setOutputChannel(MIDI_OUTPUT_CHANNEL);
    mididevice.setInputChannel(MIDI_INPUT_CHANNEL);
    midi.sendFirmwareVersion();
    setMessage("OWL bootloader ready");
    for(int i=0; i<3; i++){
      HAL_Delay(600);
      led_red();
      HAL_Delay(600);
      led_green();
    }
  }

  void loop(void){
    midi.push();
    IWDG->KR = 0xaaaa; // reset the watchdog timer (if enabled)
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
      saveToFlash(FIRMWARE_SECTOR, loader.getData(), loader.getSize());
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

void jump_to_bootloader(void){
#ifdef USE_USB_HOST
  HAL_GPIO_WritePin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin, GPIO_PIN_RESET);
#endif
  *OWLBOOT_MAGIC_ADDRESS = OWLBOOT_MAGIC_NUMBER;
  /* Disable all interrupts */
  __disable_irq();
  NVIC_SystemReset();
  /* Shouldn't get here */
  while(1);
}

void device_reset(){
  NVIC_SystemReset();
  /* Shouldn't get here */
  while(1);
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
    jump_to_bootloader();
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

bool MidiReader::readMidiFrame(uint8_t* frame){
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
      midi.sendFirmwareVersion();
      break;
    case SYSEX_DEVICE_ID:
      midi.sendDeviceId();
      break;
    case SYSEX_PROGRAM_MESSAGE:
      sendMessage();
      break;
    }
    break;
  }
}

void midi_device_rx(uint8_t *buffer, uint32_t length){
  for(uint16_t i=0; i<length; i+=4){
    if(!mididevice.readMidiFrame(buffer+i))
      mididevice.reset();
  }
}
