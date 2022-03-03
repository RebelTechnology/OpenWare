#ifndef __FLASH_S25FL_H
#define __FLASH_S25FL_H

#include <stdint.h>
#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif
  
  void flash_init(void* spiconfig);
  int flash_read(uint32_t address, uint8_t* data, size_t length);
  int flash_write(uint32_t address, uint8_t* data, size_t length);
  int flash_erase(uint32_t address, size_t size);

#ifdef __cplusplus
}
#endif
#endif
