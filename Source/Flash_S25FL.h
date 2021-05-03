#ifndef __FLASH_S25FL_H
#define __FLASH_S25FL_H

#ifdef __cplusplus
extern "C" {
#endif
  
#include "device.h"

#define INST_ERASE_SECTOR			0x20
#define INST_ERASE_BLOCK			0xD8
#define INST_ERASE_HALF_BLOCK			0x52
#define INST_ERASE_CHIP				0xC7

#define ERASE_4KB                               INST_ERASE_SECTOR
#define ERASE_32KB                              INST_ERASE_HALF_BLOCK
#define ERASE_64KB                              INST_ERASE_BLOCK
	  
typedef struct __SPI_HandleTypeDef SPI_HandleTypeDef; // forward declaration
void Flash_S25FL_init(SPI_HandleTypeDef* spiconfig);
void Flash_read(uint32_t address, uint8_t* data, size_t length);
void Flash_write(uint32_t address, uint8_t* data, size_t length);
unsigned char Flash_readStatusReg (unsigned char reg);
void Flash_writeStatusReg (unsigned char reg, unsigned char data);
void Flash_erase (uint32_t address, uint8_t cmd);
void Flash_BulkErase (void);

#ifdef __cplusplus
}
#endif
#endif
