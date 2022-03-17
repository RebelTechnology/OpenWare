#include "sdram.h"

void MPU_Config(void){
  HAL_MPU_Disable();
  MPU_Region_InitTypeDef MPU_InitStruct;

  // TEX=0 C=0 B=1 S=1: Shared Device
  // TEX=0 C=0 B=0 S=1: Strongly Ordered
  // TEX=1 C=0 B=0 S=1: Normal Non-cacheable
  // TEX=2 C=0 B=0 S=0: Non-shareable Device
  // Priority of overlapping regions is in ascending order

  size_t region_number = 0;

  /* Configure the MPU attributes for DMA RAM */
#if defined USE_ICACHE || defined USE_DCACHE
  // D2 DMA buffers: cache off
  // TEX=1 C=0 B=0 S=1: Normal Non-cacheable
  extern char _DMA_DATA;
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = 1 << region_number++; // MPU_REGION_NUMBER0
  MPU_InitStruct.BaseAddress = (uint32_t)&_DMA_DATA;
  MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
#endif /* USE_ICACHE || USE_DCACHE */

  /* Configure the MPU attributes for SDRAM */
#if 0 // using defaults instead
// #ifdef USE_EXTERNAL_RAM
  // special configuration is probably not required for SDRAM
  // if enabled, make sure that REGION_SIZE is correct
  // TEX=1 C=1 B=1 S=1: Normal Write-back, write and read allocate
  extern char _EXTRAM;
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = 1 << region_number++;
  MPU_InitStruct.BaseAddress = (uint32_t)&_EXTRAM;
  MPU_InitStruct.Size = MPU_REGION_SIZE_8MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
#endif

  /* Configure the MPU attributes for D1 PLUSRAM */
#if 0 // using defaults instead
// #ifdef USE_PLUS_RAM  
  // AXISRAM D1 (+RAM) - write back with no write allocate, execute
  extern char _PLUSRAM;
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = 1 << region_number++;
  MPU_InitStruct.BaseAddress      = (uint32_t)&_PLUSRAM ;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
#endif

  /* Configure the MPU attributes for memory mapped QSPI flash */
#ifdef USE_QSPI_FLASH
  // Strongly Ordered: TEX=0, C=0, B=0, Shareable=Yes
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = 1 << region_number++;
  MPU_InitStruct.BaseAddress = 0x90000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_8MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
#endif
  
  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

  /* Differentiate MemManage, BusFault and UsageFault */
  SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_USGFAULTENA_Msk;

  /* ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk); */
}
