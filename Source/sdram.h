#ifndef SDRAM_H
#define SDRAM_H

#include "errorhandlers.h"

#ifdef USE_EXTERNAL_RAM
void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram);
#endif

void MPU_Config(void);

#if 0
int testram8(SDRAM_HandleTypeDef *hsdram){
  const int len = 128;
  int failures = 0;
  uint32_t* sram;
  sram = (uint32_t*)SDRAM_BANK2_ADDR;
  uint8_t src[len];
  uint8_t dst[len];
  for(int i=0; i<len; ++i){
    src[i] = rand();
    dst[i] = 0;
  }
  if(HAL_SDRAM_Write_8b(hsdram, sram, src, len) != HAL_OK)
    error(MEM_ERROR, "SDRAM write error");
  if(HAL_SDRAM_Read_8b(hsdram, sram, dst, len) != HAL_OK)
    error(MEM_ERROR, "SDRAM read error");
  for(int i=0; i<len; ++i){
    if(dst[i] != src[i])
      failures++;
  }
  return failures;
}

int testram16(SDRAM_HandleTypeDef *hsdram){
  const int len = 128;
  int failures = 0;
  uint32_t* sram;
  sram = (uint32_t*)SDRAM_BANK2_ADDR;
  uint16_t src[len];
  uint16_t dst[len];
  for(int i=0; i<len; ++i){
    src[i] = rand();
    dst[i] = 0;
  }
  if(HAL_SDRAM_Write_16b(hsdram, sram, src, len) != HAL_OK)
    error(MEM_ERROR, "SDRAM write error");
  if(HAL_SDRAM_Read_16b(hsdram, sram, dst, len) != HAL_OK)
    error(MEM_ERROR, "SDRAM read error");
  for(int i=0; i<len; ++i){
    if(dst[i] != src[i])
      failures++;
  }
  return failures;
}

int testram32(SDRAM_HandleTypeDef *hsdram){
  const int len = 128;
  int failures = 0;
  uint32_t* sram;
  sram = (uint32_t*)SDRAM_BANK2_ADDR;
  uint32_t src[len];
  uint32_t dst[len];
  for(int i=0; i<len; ++i){
    src[i] = rand();
    dst[i] = 0;
  }
  if(HAL_SDRAM_Write_32b(hsdram, sram, src, len) != HAL_OK)
    error(MEM_ERROR, "SDRAM write error");
  if(HAL_SDRAM_Read_32b(hsdram, sram, dst, len) != HAL_OK)
    error(MEM_ERROR, "SDRAM read error");
  for(int i=0; i<len; ++i){
    if(dst[i] != src[i])
      failures++;
  }
  return failures;
}
#endif

#endif /* SDRAM_H */

