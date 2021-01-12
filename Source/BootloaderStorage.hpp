#ifndef __BOOTLOADER_STORAGE_H__
#define __BOOTLOADER_STORAGE_H__

#include <inttypes.h>
#include "eepromcontrol.h"

extern char _BOOTLOADER;

class BootloaderStorage {
public:
    bool erase(){
        eeprom_unlock();
        int ret = eeprom_erase_sector(FLASH_SECTOR_0);
        ret |= eeprom_erase_sector(FLASH_SECTOR_1);
#if !defined(OWL_MODULAR) && !defined(OWL_PEDAL)
        ret |= eeprom_erase_sector(FLASH_SECTOR_2);
        ret |= eeprom_erase_sector(FLASH_SECTOR_3);
#endif
        eeprom_lock();
        return ret == 0;
    }

    bool store(void* source, size_t size){
        eeprom_unlock();
        int ret = eeprom_write_block((uint32_t)&_BOOTLOADER, source, size);
        eeprom_lock();
        return ret == 0;
    }
};

#endif