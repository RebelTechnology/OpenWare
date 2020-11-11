#include "Owl.h"

extern "C"{
  void eeprom_unlock(){}
  int eeprom_write_block(uint32_t address, void* data, uint32_t size)
  {return 0;}
   int eeprom_write_word(uint32_t address, uint32_t data)
  {return 0;}
   int eeprom_write_byte(uint32_t address, uint8_t data)
  {return 0;}
   int eeprom_erase(uint32_t address)
  {return 0;}

   int eeprom_wait()
  {return 0;}
   int eeprom_erase_sector(uint32_t sector)
  {return 0;}

void setPortMode(uint8_t index, uint8_t mode){}
uint8_t getPortMode(uint8_t index){
  return 0;
}
}  
