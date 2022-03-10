#include "support.h"

void device_dfu(){
  /* Set the address of the entry point to bootloader */
#ifdef STM32H7xx
  const uint32_t BootAddr = 0x1FF09800;
#else
  const uint32_t BootAddr = 0x1FF00000;
#endif
  device_reset_to(BootAddr);
} 

void device_bootloader(){
  *OWLBOOT_MAGIC_ADDRESS = OWLBOOT_MAGIC_NUMBER;
  device_reset_to(0);
}

void device_reset(){
  *OWLBOOT_MAGIC_ADDRESS = 0;
  device_reset_to(0);
}

void device_reset_to(uint32_t address){
  // https://community.st.com/s/article/STM32H7-bootloader-jump-from-application
  volatile uint32_t BootAddr = address;
  /* Disable all interrupts */
  __disable_irq();
  /* Disable Systick timer */
  SysTick->CTRL = 0;
  /* Set the clock to the default state */
  HAL_RCC_DeInit();
  HAL_DeInit();

  #ifdef USE_ICACHE
  SCB_InvalidateICache();
#endif
#ifdef USE_DCACHE
  SCB_CleanInvalidateDCache();
#endif

  /* Clear Interrupt Enable Register & Interrupt Pending Register */
  for(uint32_t i=0; i<8; i++){
    NVIC->ICER[i] = 0xFFFFFFFF;
    NVIC->ICPR[i] = 0xFFFFFFFF;
  }

  /* Re-enable interrupts */
  __enable_irq();

  /* __DSB(); __ISB(); // memory and instruction barriers */

  if(BootAddr == 0){
    NVIC_SystemReset();
  }else{
    /* Set up the jump to booloader address + 4 */
    void (*SysMemBootJump)(void);
    SysMemBootJump = (void (*)(void)) (*((uint32_t *) ((BootAddr + 4))));
    /* Set the main stack pointer to the bootloader stack */
    __set_MSP(*(uint32_t *)BootAddr);
    /* Call the function to jump to bootloader location */
    SysMemBootJump();
  }
  /* Shouldn't get here */
  for(;;);
}

void device_cache_invalidate(){
#ifdef USE_ICACHE
  SCB_InvalidateICache();
#endif
#ifdef USE_DCACHE
  SCB_CleanInvalidateDCache();
#endif
}

void device_watchdog(){
#ifdef USE_IWDG
#ifdef STM32H7xx
  IWDG1->KR = 0xaaaa; // reset the watchdog timer (if enabled)
#else
  IWDG->KR = 0xaaaa; // reset the watchdog timer (if enabled)
#endif
#endif
}
