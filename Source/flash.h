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
   int flash_write(uint32_t address, uint8_t* data, size_t length);
   int flash_erase(uint32_t address, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_CONTROL_H */
