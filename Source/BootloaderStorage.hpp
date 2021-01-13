#ifndef __BOOTLOADER_STORAGE_H__
#define __BOOTLOADER_STORAGE_H__

#include <inttypes.h>
#include "eepromcontrol.h"

extern char _BOOTLOADER, _BOOTLOADER_END;

class BootloaderStorage {
public:
    bool erase(){
        eeprom_unlock();
        // We expect bootloader to occupy 1 - 4 sectors. Currently OWL1 uses 2 sectors, OWL2 uses 4.
        int ret = eeprom_erase_sector(FLASH_SECTOR_0);
        if (ADDR_FLASH_SECTOR_1 < (uint32_t)&_BOOTLOADER_END)
            ret |= eeprom_erase_sector(FLASH_SECTOR_1);
        if (ADDR_FLASH_SECTOR_2 < (uint32_t)&_BOOTLOADER_END)
            ret |= eeprom_erase_sector(FLASH_SECTOR_2);
        if (ADDR_FLASH_SECTOR_3 < (uint32_t)&_BOOTLOADER_END)
            ret |= eeprom_erase_sector(FLASH_SECTOR_3);
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