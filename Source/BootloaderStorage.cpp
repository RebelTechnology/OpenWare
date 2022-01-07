#include "device.h"
#include "eepromcontrol.h"
#include "BootloaderStorage.h"
#include "message.h"

extern char _BOOTLOADER, _BOOTLOADER_END;

BootloaderStorage::BootloaderStorage(){
    bootloader_sectors = OB_WRP_SECTOR_0;
    if (ADDR_FLASH_SECTOR_1 < (uint32_t)&_BOOTLOADER_END)
        bootloader_sectors |= OB_WRP_SECTOR_1;
    if (ADDR_FLASH_SECTOR_2 < (uint32_t)&_BOOTLOADER_END)
        bootloader_sectors |= OB_WRP_SECTOR_2;
    if (ADDR_FLASH_SECTOR_3 < (uint32_t)&_BOOTLOADER_END)
        bootloader_sectors |= OB_WRP_SECTOR_3;
}

bool BootloaderStorage::erase(){
    eeprom_unlock();
    // We expect bootloader to occupy 1 - 4 sectors. Currently OWL1 uses 2 sectors, OWL2 uses 4.
    int ret = eeprom_erase_sector(FLASH_SECTOR_0, FLASH_BANK_1);
    if (bootloader_sectors & OB_WRP_SECTOR_1)
        ret |= eeprom_erase_sector(FLASH_SECTOR_1, FLASH_BANK_1);
    if (bootloader_sectors & OB_WRP_SECTOR_2)
        ret |= eeprom_erase_sector(FLASH_SECTOR_2, FLASH_BANK_1);
    if (bootloader_sectors & OB_WRP_SECTOR_3)
        ret |= eeprom_erase_sector(FLASH_SECTOR_3, FLASH_BANK_1);
    eeprom_lock();
    return ret == 0;
}

bool BootloaderStorage::store(void* source, size_t size){
    eeprom_unlock();
    int ret = eeprom_write_block((uint32_t)&_BOOTLOADER, source, size);
    eeprom_lock();
    return ret == 0;
}

bool BootloaderStorage::unlock(){
    eeprom_unlock();
    int ret = eeprom_write_unlock(bootloader_sectors);
    eeprom_lock();
    return ret == HAL_OK;
}

bool BootloaderStorage::lock(){
    eeprom_unlock();
    int ret = eeprom_write_lock(bootloader_sectors);
    eeprom_lock();
    return ret == HAL_OK;
}

uint32_t BootloaderStorage::getWriteProtectedSectors() const {
    return eeprom_write_protection(bootloader_sectors);
}

bool BootloaderStorage::isWriteProtected() const {
    return eeprom_write_protection(bootloader_sectors) == bootloader_sectors;
}
