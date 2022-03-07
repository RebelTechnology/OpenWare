#ifndef __FLASH_H
#define __FLASH_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
 extern "C" {
#endif

   void flash_init(void* handle);
   int flash_read(uint32_t address, uint8_t* data, size_t length);
   int flash_write(uint32_t address, const uint8_t* data, size_t length);
   int flash_erase(uint32_t address, size_t size);

#ifdef USE_QSPI_FLASH
#define QSPI_FLASH_BASE              0x90000000
   int flash_status();
   int flash_memory_map(int mode);
   int flash_wait();
#endif
   
#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_CONTROL_H */
