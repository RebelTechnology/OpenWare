#include "StorageBlock.h"
#include "device.h"
#include <string.h>
#include "message.h"
#include "eepromcontrol.h"
// #include "stm32f4xx.h"

extern char _FLASH_STORAGE_END;
#define EEPROM_PAGE_END   ((uint32_t)&_FLASH_STORAGE_END)

StorageBlock::StorageBlock() : header(nullptr){}

uint32_t StorageBlock::getBlockSize(){
  uint32_t size = 4 + getDataSize();
  // size = (size + (32 - 1)) & -32;  // Round up to 32-byte boundary
  // size = (size + (8 - 1)) & -8;  // Round up to 8-byte boundary
  size = (size + (4 - 1)) & -4;  // Round up to 4-byte boundary
  return size;
}

bool StorageBlock::isValidSize(){
  return header != nullptr && getDataSize() > 0 && 
    (uint32_t)header + getBlockSize() < EEPROM_PAGE_END;
}

bool StorageBlock::verify(){
  return isValidSize() && isWritten() && !isDeleted();
}

bool StorageBlock::write(void* data, uint32_t size){
  if((uint32_t)header+4+size > EEPROM_PAGE_END)
    return false;
  eeprom_unlock();
  eeprom_wait(); // clear flash flags
  bool status = eeprom_write_word((uint32_t)getBlock(), 0xcf000000 | size); // set magick and size
  status = status || eeprom_write_block((uint32_t)getData(), data, size);
  eeprom_lock();
  if(status){
    error(FLASH_ERROR, "Flash write failed");
    return false;
  }
  if(size != getDataSize()){
    error(FLASH_ERROR, "Size verification failed");
    return false;
  }
  if(memcmp(data, getData(), size) != 0){
    error(FLASH_ERROR, "Data verification failed");
    return false;
  }
  return true;
}

bool StorageBlock::setDeleted(){
  uint32_t size = getDataSize();
  if(size){
    eeprom_unlock();
    bool status = eeprom_write_word((uint32_t)header, 0xc0000000 | size);
    eeprom_lock();
    if(status){
      error(FLASH_ERROR, "Flash delete failed");
      return false;
    }
    if(size != getDataSize() || !isDeleted()){
      error(FLASH_ERROR, "Flash delete error");
      return false;
    }
    return true;
  }else{
    return false;
  }
}
