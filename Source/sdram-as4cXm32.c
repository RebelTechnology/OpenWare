#include <stm32h7xx_hal.h>
#include "sdram.h"

/**
 * Alliance Memory DRAM 256Mbits, 3.3V, 8Mx32 AS4C8M32SA-6BIN
 */

#define SDRAM_MODEREG_BURST_LENGTH_2               (1 << 0)
#define SDRAM_MODEREG_BURST_LENGTH_4               (1 << 1)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL        (0 << 3)
#define SDRAM_MODEREG_CAS_LATENCY_3                ((1 << 4) | (1 << 5))
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE       (1 << 9)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROG_BURST   (0 << 9)

void MPU_Config(void){
  /* Disable the MPU */
  HAL_MPU_Disable();

#ifdef STM32H7xx
#if defined USE_ICACHE || defined USE_DCACHE
  MPU_Region_InitTypeDef MPU_InitStruct;

  // D2 DMA buffers - cache off, no execute
  extern char _DMA_DATA;
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = (uint32_t)&_DMA_DATA;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_128KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  // SDRAM - write back with no write allocate, no execute
#ifdef USE_EXTERNAL_RAM
  extern char _EXTRAM;
  MPU_InitStruct.IsCacheable  = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsShareable  = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number       = MPU_REGION_NUMBER1;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.Size         = MPU_REGION_SIZE_32MB;
  MPU_InitStruct.BaseAddress  = (uint32_t)&_EXTRAM;
  MPU_InitStruct.DisableExec  = MPU_INSTRUCTION_ACCESS_DISABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
#endif

  // AXISRAM D1 (+RAM) - write back with no write allocate, execute
  extern char _PLUSRAM;
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = (uint32_t)&_PLUSRAM ;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
#endif /* USE_ICACHE || USE_DCACHE */
#endif /* STM32H7xx */

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram){
  FMC_SDRAM_CommandTypeDef Command;

  __IO uint32_t tmpmrd = 0;
  /* Step 3:  Configure a clock configuration enable command */
  Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, &Command, 0x1000);

  /* Step 4: Insert 100 ms delay */
  HAL_Delay(100);

  /* Step 5: Configure a PALL (precharge all) command */
  Command.CommandMode            = FMC_SDRAM_CMD_PALL;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, &Command, 0x1000);

  /* Step 6 : Configure a Auto-Refresh command */
  Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 4;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, &Command, 0x1000);

  /* Step 7: Program the external memory mode register */
  tmpmrd = SDRAM_MODEREG_BURST_LENGTH_4
         | SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL
         | SDRAM_MODEREG_CAS_LATENCY_3
         | SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;  
  Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = tmpmrd;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, &Command, 0x1000);

  /* Set refresh rate */
  HAL_SDRAM_ProgramRefreshRate(hsdram, 0x81A - 20);
}
