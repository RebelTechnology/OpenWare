#ifndef SDRAM_H
#define SDRAM_H

#include "errorhandlers.h"

#define SDRAM_BANK1_ADDR                 ((uint32_t)0xC0000000)
#define SDRAM_BANK2_ADDR                 ((uint32_t)0xD0000000)
#define SDRAM_MEMORY_WIDTH               FMC_SDRAM_MEM_BUS_WIDTH_32
/* #define SDCLOCK_PERIOD                   FMC_SDRAM_CLOCK_PERIOD_2 */
#define SDCLOCK_PERIOD                FMC_SDRAM_CLOCK_PERIOD_3
#define SDRAM_TIMEOUT                    ((uint32_t)0xFFFF)
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0003) // fixed: was 0x0004
#define SDRAM_MODEREG_BURST_LENGTH_FULL_PAGE     ((uint16_t)0x0007) // added
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram){
  FMC_SDRAM_CommandTypeDef command;
  __IO uint32_t tmpmrd =0;
  /* Step 1:  Configure a clock configuration enable command */
  command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  command.AutoRefreshNumber = 1;
  command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(hsdram, &command, 0x1000) != HAL_OK)
    error(MEM_ERROR, "SDRAM cmd1 error");

  /* Step 2: Insert 100 us minimum delay */
  /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
  HAL_Delay(1);

  /* Step 3: Configure a PALL (precharge all) command */
  command.CommandMode = FMC_SDRAM_CMD_PALL;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  command.AutoRefreshNumber = 1;
  command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(hsdram, &command, 0x1000) != HAL_OK)
    error(MEM_ERROR, "SDRAM cmd2 error");

  /* Step 4 : Configure a Auto-Refresh command */
  command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  command.AutoRefreshNumber = 8;
  command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(hsdram, &command, 0x1000) != HAL_OK)
    error(MEM_ERROR, "SDRAM cmd3 error");

  /* Step 5: Program the external memory mode register */
  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                     SDRAM_MODEREG_CAS_LATENCY_3           |
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  command.AutoRefreshNumber = 1;
  command.ModeRegisterDefinition = tmpmrd;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(hsdram, &command, 0x1000) != HAL_OK)
    error(MEM_ERROR, "SDRAM cmd4 error");

  /* Step 6: Set the refresh rate counter */
  /* (15.62 us x Freq) - 20 */
  /* Set the device refresh counter */
  hsdram->Instance->SDRTR |= ((uint32_t)((1292)<< 1));
}
  
void MPU_Config(void){
  MPU_Region_InitTypeDef MPU_InitStruct;
  
  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as WB for SDRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = SDRAM_BANK2_ADDR;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

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

#endif /* SDRAM_H */

