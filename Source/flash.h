#ifndef __FLASH_H
#define __FLASH_H

#include <stdbool.h>
#include <stdint.h>

#define QSPI_FLASH_BASE                      0x90000000
#define QSPI_FLASH_SIZE                      (8*1024*1024) // 8M / 64Mbit

#ifdef __cplusplus
 extern "C" {
#endif

   void flash_init();
   int flash_status();
   int flash_quad_mode(uint8_t, uint8_t);
   int flash_qpi_mode(bool qpi);
   int flash_write_block(uint32_t address, void* data, uint32_t size);
   int flash_read_block(int mode, uint32_t address, void* data, uint32_t size);
   int flash_wait();
   int flash_erase(uint32_t address, uint32_t size);
   int flash_memory_map(int mode);

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_CONTROL_H */
