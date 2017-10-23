#include "eepromcontrol.h"
#include <string.h> /* for memcpy */
#include "device.h"

#define EEPROM_SECTOR_MASK               ((uint32_t)0xFFFFFF07)

int eeprom_get_status(){
  if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET)
    return FLASH_FLAG_BSY;
  return HAL_FLASH_GetError();
  /* uint32_t status = HAL_FLASH_GetError(); */
  /* if(status == HAL_FLASH_ERROR_NONE) */
  /*   return HAL_FLASH_ERROR_NONE; */
  /* else */
  /*   return FLASH_ERROR_OPERATION; */
}

void eeprom_lock(){
  HAL_FLASH_Lock();
}

int eeprom_wait(){ 
  /* FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE); */
  return FLASH_WaitForLastOperation(HAL_MAX_DELAY);
}

int eeprom_erase_sector(uint32_t sector) {
  FLASH_EraseInitTypeDef cfg;
  cfg.TypeErase = FLASH_TYPEERASE_SECTORS;
#ifndef OWL_ARCH_F7
  cfg.Banks = FLASH_BANK_1;
#endif
  cfg.Sector = sector;
  cfg.NbSectors = 1;
  cfg.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  uint32_t error;
  HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&cfg, &error);
  return status;
}

int eeprom_write_word(uint32_t address, uint32_t data){
  HAL_StatusTypeDef status =  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
  return status;
}

int eeprom_write_byte(uint32_t address, uint8_t data){
  HAL_StatusTypeDef status =  HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, data);
  return status;
}

int eeprom_write_block(uint32_t address, void* data, uint32_t size){
  uint32_t* p32 = (uint32_t*)data;
  uint32_t i=0; 
  for(;i<size-3; i+=4)
    eeprom_write_word(address+i, *p32++);
  uint8_t* p8 = (uint8_t*)p32;
  for(;i<size; i++)
    eeprom_write_byte(address+i, *p8++);
  return eeprom_wait() == HAL_FLASH_ERROR_NONE ? 0 : -1;
}

void eeprom_unlock(){
  HAL_FLASH_Unlock();
}

void* eeprom_read(uint32_t address){
  while(eeprom_get_status() == FLASH_FLAG_BSY);
  return (void*)address;
}

uint8_t eeprom_read_byte(uint32_t address){
  while(eeprom_get_status() == FLASH_FLAG_BSY);
  return *(uint8_t*)address;
}

uint32_t eeprom_read_word(uint32_t address){
  while(eeprom_get_status() == FLASH_FLAG_BSY);
  return *(uint32_t*)address;
}

int eeprom_read_block(uint32_t address, void* data, uint32_t size){
  while(eeprom_get_status() == FLASH_FLAG_BSY);
  memcpy(data, (const void*)address, size);
  return eeprom_get_status();
}

int eeprom_erase(uint32_t address){
  int ret = -1;
  if(address < ADDR_FLASH_SECTOR_1)
    ret = -1;  // protect boot sector
    /* eeprom_erase_sector(FLASH_SECTOR_0, VoltageRange_3); */
  else if(address < ADDR_FLASH_SECTOR_2)
    ret = eeprom_erase_sector(FLASH_SECTOR_1);
  else if(address < ADDR_FLASH_SECTOR_3)
    ret = eeprom_erase_sector(FLASH_SECTOR_2);
  else if(address < ADDR_FLASH_SECTOR_4)
    ret = eeprom_erase_sector(FLASH_SECTOR_3);
  else if(address < ADDR_FLASH_SECTOR_5)
    ret = eeprom_erase_sector(FLASH_SECTOR_4);
  else if(address < ADDR_FLASH_SECTOR_6)
    ret = eeprom_erase_sector(FLASH_SECTOR_5);
  else if(address < ADDR_FLASH_SECTOR_7)
    ret = eeprom_erase_sector(FLASH_SECTOR_6);
  else if(address < ADDR_FLASH_SECTOR_8)
    ret = eeprom_erase_sector(FLASH_SECTOR_7);
#ifndef OWL_ARCH_F7
  else if(address < ADDR_FLASH_SECTOR_9)
    ret = eeprom_erase_sector(FLASH_SECTOR_8);
  else if(address < ADDR_FLASH_SECTOR_10)
    ret = eeprom_erase_sector(FLASH_SECTOR_9);
  else if(address < ADDR_FLASH_SECTOR_11)
    ret = eeprom_erase_sector(FLASH_SECTOR_10);
  else if(address < 0x08100000)
    ret = eeprom_erase_sector(FLASH_SECTOR_11);
#endif
  else
    ret = -1;
  return ret;
}
