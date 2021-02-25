#include "eepromcontrol.h"
#include <string.h> /* for memcpy */
#include "device.h"

extern void device_reset(void);

void eeprom_lock(){
  HAL_FLASH_Lock();
}

#ifndef STM32H743xx // todo: fix for H7!
int eeprom_wait(){ 
  return FLASH_WaitForLastOperation(5000);
}

int eeprom_get_error() {
  return HAL_FLASH_GetError();
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
  HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
  return status;
}

int eeprom_write_byte(uint32_t address, uint8_t data){
  HAL_StatusTypeDef status =  HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, data);
  return status;
}

int eeprom_write_block(uint32_t address, void* data, uint32_t size){
  uint32_t* p32 = (uint32_t*)data;
  uint32_t i=0; 
  for(;i+4<=size; i+=4)
    eeprom_write_word(address+i, *p32++);
  uint8_t* p8 = (uint8_t*)p32;
  for(;i<size; i++)
    eeprom_write_byte(address+i, *p8++);
  return eeprom_wait() == HAL_FLASH_ERROR_NONE ? 0 : -1;
}

void eeprom_unlock(){
  HAL_FLASH_Unlock();
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
#ifdef FLASH_SECTOR_8
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

int eeprom_write_unlock(uint32_t wrp_sectors){
  int ret = HAL_OK;
  FLASH_OBProgramInitTypeDef OptionBytes;
  /* Disable write protected pages */
  OptionBytes.OptionType = OPTIONBYTE_WRP;
  OptionBytes.WRPState = OB_WRPSTATE_DISABLE;
  OptionBytes.Banks = FLASH_BANK_1;
  OptionBytes.WRPSector = wrp_sectors;
  HAL_FLASH_OB_Unlock();
  ret = HAL_FLASHEx_OBProgram(&OptionBytes);
  /*
  * This would perform system reset here unless an error is returned
  */
  if (ret == HAL_OK) {
    ret |= HAL_FLASH_OB_Launch();
    device_reset();
  }
  HAL_FLASH_OB_Lock();
  return ret;
}

int eeprom_write_lock(uint32_t wrp_sectors){
  int ret = HAL_OK;
  FLASH_OBProgramInitTypeDef OptionBytes;
  /* Enable write protection */
  OptionBytes.OptionType = OPTIONBYTE_WRP;
  OptionBytes.WRPState = OB_WRPSTATE_ENABLE;
  OptionBytes.Banks = FLASH_BANK_1;
  OptionBytes.WRPSector = wrp_sectors;
  HAL_FLASH_OB_Unlock();
  ret = HAL_FLASHEx_OBProgram(&OptionBytes);
  /*
  * We perform system reset here unless an error is returned
  */
  if (ret == HAL_OK) {
    ret |= HAL_FLASH_OB_Launch();
    device_reset();
  }
  HAL_FLASH_OB_Lock();
    return ret;
}

/*
 * Checks which sectors have write protection option byte set
 */
uint32_t eeprom_write_protection(uint32_t wrp_sectors){
  FLASH_OBProgramInitTypeDef OptionBytes;
  HAL_FLASHEx_OBGetConfig(&OptionBytes);
  return ~OptionBytes.WRPSector & wrp_sectors;
}

#endif
