/******************** (C) COPYRIGHT 2015 STMicroelectronics ********************
* File Name          : sleep.c
* Author             : AMS - VMA
* Version            : V1.0.0
* Date               : 19-May-2015
* Description        : BlueNRG Sleep management
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "BlueNRG1.h"
#include "BlueNRG1_conf.h"
#include "sleep.h"
#include "misc.h"
#include "bluenrg1_stack.h"

#undef DEBUG_SLEEP_MODE
#define DEBUG_SLEEP_MODE 1

#define MIN(a,b) ((a) < (b) )? (a) : (b)

#define SHPR3_REG 0xE000ED20

#define WAKENED_FROM_BOR          0x03
#define WAKENED_FROM_POR          0x05
#define WAKENED_FROM_IO9          0x09
#define WAKENED_FROM_IO10         0x11
#define WAKENED_FROM_IO11         0x21
#define WAKENED_FROM_IO12         0x41
#define WAKENED_FROM_IO13         0x81
#define WAKENED_FROM_BLUE_TIMER1  0x101
#define WAKENED_FROM_BLUE_TIMER2  0x401

#define LOW_POWER_STANDBY  0x03

#define BLUE_CURRENT_TIME_REG 0x48000010

extern void InternalIdleSleep(void);
extern const intvec_elem __vector_table[];
uint32_t cStackPreamble[CSTACK_PREAMBLE_NUMBER];
volatile uint32_t* ptr ;
extern uint32_t savedCurrentTime;
extern uint32_t savedICSR;
extern uint32_t savedSHCSR;
extern uint32_t savedNVIC_ISPR;

#ifdef DEBUG_SLEEP_MODE
uint32_t sleepMode_selected[4]={0,};
#endif

WEAK_FUNCTION(SleepModes App_SleepMode_Check(SleepModes sleepMode))
{
  return SLEEPMODE_NOTIMER;
}

static void BlueNRG_InternalSleep(SleepModes sleepMode, uint8_t gpioWakeBitMask, uint8_t sleepTimerEnabled)
{
  uint8_t i;
  extern volatile  uint8_t wakeupFromSleepFlag;
  /* System Control saved */
  uint32_t SYS_Ctrl_saved;
  /* NVIC Information Saved */
  uint32_t NVIC_ISER_saved, NVIC_IPR_saved[8], PENDSV_SYSTICK_IPR_saved;
  /* CKGEN SOC Enabled */
  uint32_t CLOCK_EN_saved;
  /* GPIO Information saved */
  uint32_t GPIO_DATA_saved, GPIO_OEN_saved, GPIO_PE_saved, GPIO_DS_saved, GPIO_IS_saved, GPIO_IBE_save;
  uint32_t GPIO_IEV_saved, GPIO_IE_saved, GPIO_MODE0_saved, GPIO_MODE1_saved, GPIO_IOSEL_MFTX_saved;
  /* UART Information saved */
  uint32_t UART_TIMEOUT_saved, UART_LCRH_RX_saved, UART_IBRD_saved, UART_FBRD_saved;
  uint32_t UART_LCRH_TX_saved, UART_CR_saved, UART_IFLS_saved, UART_IMSC_saved;
  uint32_t UART_DMACR_saved, UART_XFCR_saved, UART_XON1_saved, UART_XON2_saved;
  uint32_t UART_XOFF1_saved, UART_XOFF2_saved;
  /* SPI Information saved */
  uint32_t SPI_CR0_saved, SPI_CR1_saved, SPI_CPSR_saved, SPI_IMSC_saved, SPI_DMACR_saved;
  uint32_t SPI_RXFRM_saved, SPI_CHN_saved, SPI_WDTXF_saved;
  /* I2C Information saved */
  uint32_t I2C_CR_saved[2], I2C_SCR_saved[2], I2C_TFTR_saved[2], I2C_RFTR_saved[2];
  uint32_t I2C_DMAR_saved[2], I2C_BRCR_saved[2], I2C_IMSCR_saved[2], I2C_THDDAT_saved[2];
  uint32_t I2C_THDSTA_FST_STD_saved[2], I2C_TSUSTA_FST_STD_saved[2];
  /* RNG Information saved */
  uint32_t RNG_CR_saved;
  /* SysTick Information saved */
  uint32_t SYST_CSR_saved, SYST_RVR_saved;
  /* RTC Information saved */
  uint32_t RTC_CWDMR_saved, RTC_CWDLR_saved, RTC_CWYMR_saved, RTC_CWYLR_saved, RTC_CTCR_saved;
  uint32_t RTC_IMSC_saved, RTC_TCR_saved, RTC_TLR1_saved, RTC_TLR2_saved, RTC_TPR1_saved;
  uint32_t RTC_TPR2_saved, RTC_TPR3_saved, RTC_TPR4_saved, RTC_TIN_saved;
  /* MFTX Information saved */
  uint32_t T1CRA_saved, T1CRB_saved, T1PRSC_saved, T1CKC_saved, T1MCTRL_saved, T1ICTRL_saved;
  uint32_t T2CRA_saved, T2CRB_saved, T2PRSC_saved, T2CKC_saved, T2MCTRL_saved, T2ICTRL_saved;
  /* WDT Information saved */
  uint32_t WDG_LR_saved, WDG_CR_saved, WDG_LOCK_saved;
  /* DMA channel [0..7] Information saved */
  uint32_t DMA_CCR_saved[8], DMA_CNDTR_saved[8], DMA_CPAR_saved[8], DMA_CMAR[8];
  /* ADC Information saved */
  uint32_t ADC_CTRL_saved, ADC_CONF_saved, ADC_IRQMASK_saved, ADC_OFFSET_saved;
  uint32_t ADC_THRESHOLD_HI_saved, ADC_THRESHOLD_LO_saved;
  /* FlASH Config saved */
  uint32_t FLASH_CONFIG_saved;


  /* Save the peripherals configuration */
  /* System Control */
  SYS_Ctrl_saved = SYSTEM_CTRL->CTRL;
  /* FLASH CONFIG */
  FLASH_CONFIG_saved = FLASH->CONFIG;
  /* NVIC */
  NVIC_ISER_saved = NVIC->ISER[0];

  // Issue with Atollic compiler
//  memcpy(NVIC_IPR_saved, (void const *)NVIC->IP, sizeof(NVIC_IPR_saved));
  for (i=0; i<8; i++) {
  	NVIC_IPR_saved[i] = NVIC->IP[i];
  }


  PENDSV_SYSTICK_IPR_saved = *(volatile uint32_t *)SHPR3_REG;
  /* CKGEN SOC Enabled */
  CLOCK_EN_saved = CKGEN_SOC->CLOCK_EN;
  /* GPIO */
  GPIO_DATA_saved = GPIO->DATA;
  GPIO_OEN_saved = GPIO->OEN;
  GPIO_PE_saved = GPIO->PE;
  GPIO_DS_saved = GPIO->DS;
  GPIO_IS_saved = GPIO->IS;
  GPIO_IBE_save = GPIO->IBE;
  GPIO_IEV_saved = GPIO->IEV;
  GPIO_IE_saved = GPIO->IE;
  GPIO_MODE0_saved = GPIO->MODE0;
  GPIO_MODE1_saved = GPIO->MODE1;
  GPIO_IOSEL_MFTX_saved = GPIO->MFTX;
  /* UART */
  UART_TIMEOUT_saved = UART->TIMEOUT;
  UART_LCRH_RX_saved = UART->LCRH_RX;
  UART_IBRD_saved = UART->IBRD;
  UART_FBRD_saved = UART->FBRD;
  UART_LCRH_TX_saved =  UART->LCRH_TX;
  UART_CR_saved = UART->CR;
  UART_IFLS_saved = UART->IFLS;
  UART_IMSC_saved = UART->IMSC;
  UART_DMACR_saved = UART->DMACR;
  UART_XFCR_saved = UART->XFCR;
  UART_XON1_saved = UART->XON1;
  UART_XON2_saved = UART->XON2;
  UART_XOFF1_saved = UART->XOFF1;
  UART_XOFF2_saved = UART->XOFF2;
  /* SPI */
  SPI_CR0_saved = SPI->CR0; 
  SPI_CR1_saved = SPI->CR1;
  SPI_CPSR_saved = SPI->CPSR;
  SPI_IMSC_saved = SPI->IMSC;
  SPI_DMACR_saved = SPI->DMACR;
  SPI_RXFRM_saved = SPI->RXFRM;
  SPI_CHN_saved = SPI->CHN;
  SPI_WDTXF_saved = SPI->WDTXF;
  /* I2C */
  for (i=0; i<2; i++) {
    I2C_Type *I2Cx = (I2C_Type*)(I2C2_BASE+ 0x100000*i);
    I2C_CR_saved[i] = I2Cx->CR;
    I2C_SCR_saved[i] = I2Cx->SCR;
    I2C_TFTR_saved[i] = I2Cx->TFTR;
    I2C_RFTR_saved[i] = I2Cx->RFTR;
    I2C_DMAR_saved[i] = I2Cx->DMAR;
    I2C_BRCR_saved[i] = I2Cx->BRCR;
    I2C_IMSCR_saved[i] = I2Cx->IMSCR;
    I2C_THDDAT_saved[i] = I2Cx->THDDAT;
    I2C_THDSTA_FST_STD_saved[i] = I2Cx->THDSTA_FST_STD;
    I2C_TSUSTA_FST_STD_saved[i] = I2Cx->TSUSTA_FST_STD;
  }
  /* RNG */
  RNG_CR_saved = RNG->CR;
  /* SysTick */
  SYST_CSR_saved = SysTick->CTRL;
  SYST_RVR_saved = SysTick->LOAD;
  /* RTC */
  RTC_CWDMR_saved = RTC->CWDMR;
  RTC_CWDLR_saved = RTC->CWDLR;
  RTC_CWYMR_saved = RTC->CWYMR;
  RTC_CWYLR_saved = RTC->CWYLR;
  RTC_CTCR_saved = RTC->CTCR;
  RTC_IMSC_saved = RTC->IMSC;
  RTC_TCR_saved = RTC->TCR;
  RTC_TLR1_saved = RTC->TLR1;
  RTC_TLR2_saved = RTC->TLR2;
  RTC_TPR1_saved = RTC->TPR1;
  RTC_TPR2_saved = RTC->TPR2;
  RTC_TPR3_saved = RTC->TPR3;
  RTC_TPR4_saved = RTC->TPR4; 
  RTC_TIN_saved = RTC->TIN;
  /* MFTX */
  T1CRA_saved = MFT1->TNCRA;
  T1CRB_saved = MFT1->TNCRB;
  T1PRSC_saved = MFT1->TNPRSC;
  T1CKC_saved = MFT1->TNCKC;
  T1MCTRL_saved = MFT1->TNMCTRL;
  T1ICTRL_saved = MFT1->TNICTRL;
  T2CRA_saved = MFT2->TNCRA;
  T2CRB_saved = MFT2->TNCRB;
  T2PRSC_saved = MFT2->TNPRSC;
  T2CKC_saved = MFT2->TNCKC;
  T2MCTRL_saved = MFT2->TNMCTRL;
  T2ICTRL_saved = MFT2->TNICTRL;
  /* WDT */
  WDG_LR_saved = WDG->LR;
  WDG_CR_saved = WDG->CR;
  if(WDG->LOCK == 0) {
    WDG_LOCK_saved = 0x1ACCE551;
  } else {
    WDG_LOCK_saved = 0;
  }
  /* DMA */
  for (i=0; i<8; i++) {
    DMA_CH_Type *DMAx = (DMA_CH_Type*)(DMA_CH0_BASE+ 0x14*i);
    DMA_CCR_saved[i] = DMAx->CCR;
    DMA_CNDTR_saved[i] = DMAx->CNDTR;
    DMA_CPAR_saved[i] = DMAx->CPAR;
    DMA_CMAR[i] = DMAx->CMAR;
  }
  /* ADC */
  ADC_CTRL_saved = ADC->CTRL;
  ADC_CONF_saved = ADC->CONF;
  ADC_IRQMASK_saved = ADC->IRQMASK;
  ADC_OFFSET_saved = ADC->OFFSET;
  ADC_THRESHOLD_HI_saved = ADC->THRESHOLD_HI;
  ADC_THRESHOLD_LO_saved = ADC->THRESHOLD_LO;
  // Enable the STANDBY mode 
  if (sleepMode == SLEEPMODE_NOTIMER) {
    BLUE_CTRL->TIMEOUT |= LOW_POWER_STANDBY<<28;
  }
  
  //Save the CSTACK number of words that will be restored at wakeup reset
  i = 0;
  ptr = __vector_table[0].__ptr ;
  ptr -= CSTACK_PREAMBLE_NUMBER;
  do {
    cStackPreamble[i] = *ptr;
    i++;
    ptr++;
  } while (i < CSTACK_PREAMBLE_NUMBER); 
     
  //Enable deep sleep
  SystemSleepCmd(ENABLE);
  wakeupFromSleepFlag = 0; // Flag to signal if a wakeup from standby or sleep occurred
  //The __disable_irq() used at the beginning of the BlueNRG_Sleep() function
  //masks all the interrupts. The interrupts will be enabled at the end of the 
  //context restore. Now induce a context save.
  void CS_contextSave(void);
  CS_contextSave();
    
  //Disable deep sleep, because if no reset occours for an interrrupt pending,
  //the register value remain set and if a simple CPU_HALT command is called from the
  //application the BlueNRG-1 enters in deep sleep without make a context save.
  //So, exiting from the deep sleep the context is restored with wrong random value.
  SystemSleepCmd(DISABLE);
  
  if (!wakeupFromSleepFlag) {
    
    if ((savedSHCSR != SCB->SHCSR)  ||                                                                //Verified if a SVCall Interrupt is pending
        ((savedNVIC_ISPR != NVIC->ISPR[0]) && ((savedNVIC_ISPR ^ NVIC->ISPR[0]) & NVIC->ISER[0]))  || //Verified if a NVIC Interrupt is pending
          ((savedICSR & 0x10000000) != (SCB->ICSR & 0x10000000)) ||                                     // Verified if a PendSV interrupt is pending
            (((savedICSR & 0x4000000) != (SCB->ICSR & 0x4000000)) && (SysTick->CTRL & 0x02))) {         // Verified if a SysTick interrupt is pending
              savedCurrentTime >>= 4;
              if (0xFFFFFF >= (savedCurrentTime+3)) { //Check if the counter are wrapping
                while ((savedCurrentTime+3) >= ((*(volatile uint32_t *)BLUE_CURRENT_TIME_REG) >> 4)); //Not Wrap
              } else {
                while (((*(volatile uint32_t *)BLUE_CURRENT_TIME_REG) >> 4) != (savedCurrentTime + 3 - 0xFFFFFF)); //Wrap
              }
            }
    
    // Disable the STANDBY mode 
    if (sleepMode == SLEEPMODE_NOTIMER) {
      BLUE_CTRL->TIMEOUT &= ~(LOW_POWER_STANDBY<<28);
    }
    
    // Disable the SLEEP Timer 
    if (sleepTimerEnabled) {
      HAL_VTimer_Stop(SLEEP_TIMER);
    }

  } else {

    /* Start a new calibration, needed to signal if the HS is ready */
    CKGEN_BLE->CLK32K_IT = 1;
    CKGEN_BLE->CLK32K_COUNT = 0;
    CKGEN_BLE->CLK32K_PERIOD = 0;

    /* Restore the CSTACK number of words that will be saved before the sleep */
    i = 0;
    ptr = __vector_table[0].__ptr ;
    ptr -= CSTACK_PREAMBLE_NUMBER;
    do {
      *ptr = cStackPreamble[i];
      i++;
      ptr++;
    } while (i < CSTACK_PREAMBLE_NUMBER); 
    
    /* Reset the Wakeup source */
    SYSTEM_CTRL->WKP_IO_IS = 0;
    SYSTEM_CTRL->WKP_IO_IE = 3; // Only wakeup source are GPIO9-GPIO10 that are also Jlink pins
    
    /* Restore the peripherals configuration */
    /* FLASH CONFIG */
    FLASH->CONFIG = FLASH_CONFIG_saved;
    /* NVIC */
    NVIC->ISER[0] = NVIC_ISER_saved;

    // Issue with Atollic compiler
//    memcpy((void *)NVIC->IP, (void*)NVIC_IPR_saved, sizeof(NVIC_IPR_saved));
    for (i=0; i<8; i++) {
    	NVIC->IP[i] = NVIC_IPR_saved[i];
    }


    *(volatile uint32_t *)SHPR3_REG = PENDSV_SYSTICK_IPR_saved;
    /* CKGEN SOC Enabled */
    CKGEN_SOC->CLOCK_EN = CLOCK_EN_saved;
    /* GPIO */
    GPIO->OEN = GPIO_OEN_saved;
    GPIO->PE = GPIO_PE_saved;
    GPIO->DS = GPIO_DS_saved;
    GPIO->IS = GPIO_IS_saved;
    GPIO->IBE = GPIO_IBE_save;
    GPIO->IEV = GPIO_IEV_saved;
    GPIO->IE = GPIO_IE_saved;
    GPIO->MODE0 = GPIO_MODE0_saved;
    GPIO->MODE1 = GPIO_MODE1_saved;
    GPIO->MFTX = GPIO_IOSEL_MFTX_saved;
    GPIO->DATA = GPIO_DATA_saved;
    /* UART */
    UART->TIMEOUT = UART_TIMEOUT_saved;
    UART->LCRH_RX = UART_LCRH_RX_saved;
    UART->IBRD = UART_IBRD_saved;
    UART->FBRD = UART_FBRD_saved;
    UART->LCRH_TX = UART_LCRH_TX_saved;
    UART->CR = UART_CR_saved;
    UART->IFLS = UART_IFLS_saved;
    UART->IMSC = UART_IMSC_saved;
    UART->DMACR = UART_DMACR_saved;
    UART->XFCR = UART_XFCR_saved;
    UART->XON1 = UART_XON1_saved;
    UART->XON2 = UART_XON2_saved;
    UART->XOFF1 = UART_XOFF1_saved;
    UART->XOFF2 = UART_XOFF2_saved;
    /* SPI */
    SPI->CR0 = SPI_CR0_saved; 
    SPI->CR1 = SPI_CR1_saved;
    SPI->CPSR = SPI_CPSR_saved;
    SPI->IMSC = SPI_IMSC_saved;
    SPI->DMACR = SPI_DMACR_saved;
    SPI->RXFRM = SPI_RXFRM_saved;
    SPI->CHN = SPI_CHN_saved;
    SPI->WDTXF = SPI_WDTXF_saved;
    /* I2C */
    for (i=0; i<2; i++) {
      I2C_Type *I2Cx = (I2C_Type*)(I2C2_BASE+ 0x100000*i);
      I2Cx->CR = I2C_CR_saved[i];
      I2Cx->SCR = I2C_SCR_saved[i];
      I2Cx->TFTR = I2C_TFTR_saved[i];
      I2Cx->RFTR = I2C_RFTR_saved[i];
      I2Cx->DMAR = I2C_DMAR_saved[i];
      I2Cx->BRCR = I2C_BRCR_saved[i];
      I2Cx->IMSCR = I2C_IMSCR_saved[i];
      I2Cx->THDDAT = I2C_THDDAT_saved[i];
      I2Cx->THDSTA_FST_STD = I2C_THDSTA_FST_STD_saved[i];
      I2Cx->TSUSTA_FST_STD = I2C_TSUSTA_FST_STD_saved[i];
    }
    /* RNG */
    RNG->CR = RNG_CR_saved;  
    /* SysTick */
    SysTick->CTRL = SYST_CSR_saved;
    SysTick->LOAD = SYST_RVR_saved;
    /* RTC */
    RTC->CWDMR = RTC_CWDMR_saved;
    RTC->CWDLR = RTC_CWDLR_saved;
    RTC->CWYMR = RTC_CWYMR_saved;
    RTC->CWYLR = RTC_CWYLR_saved;
    RTC->CTCR = RTC_CTCR_saved;
    RTC->IMSC = RTC_IMSC_saved;
    RTC->TLR1 = RTC_TLR1_saved;
    RTC->TLR2 = RTC_TLR2_saved;
    RTC->TPR1 = RTC_TPR1_saved;
    RTC->TPR2 = RTC_TPR2_saved;
    RTC->TPR3 = RTC_TPR3_saved;
    RTC->TPR4 = RTC_TPR4_saved; 
    RTC->TIN = RTC_TIN_saved;
    RTC->TCR = RTC_TCR_saved; /* Enable moved at the end of RTC configuration */
    /* MFTX */
    MFT1->TNCRA = T1CRA_saved;
    MFT1->TNCRB = T1CRB_saved;
    MFT1->TNPRSC = T1PRSC_saved;
    MFT1->TNCKC = T1CKC_saved;
    MFT1->TNMCTRL = T1MCTRL_saved;
    MFT1->TNICTRL = T1ICTRL_saved;
    MFT2->TNCRA = T2CRA_saved;
    MFT2->TNCRB = T2CRB_saved;
    MFT2->TNPRSC = T2PRSC_saved;
    MFT2->TNCKC = T2CKC_saved;
    MFT2->TNMCTRL = T2MCTRL_saved;
    MFT2->TNICTRL = T2ICTRL_saved;
    /* WDT */
    WDG->LR = WDG_LR_saved;
    WDG->CR = WDG_CR_saved;
    WDG->LOCK = WDG_LOCK_saved;
    /* DMA */
    for (i=0; i<8; i++) {
      DMA_CH_Type *DMAx = (DMA_CH_Type*)(DMA_CH0_BASE+ 0x14*i);
      DMAx->CCR = DMA_CCR_saved[i] ;
      DMAx->CNDTR = DMA_CNDTR_saved[i];
      DMAx->CPAR = DMA_CPAR_saved[i];
      DMAx->CMAR = DMA_CMAR[i];
    }
    /* ADC */
    ADC->CTRL = ADC_CTRL_saved;
    ADC->CONF = ADC_CONF_saved;
    ADC->IRQMASK = ADC_IRQMASK_saved;
    ADC->OFFSET = ADC_OFFSET_saved;
    ADC->THRESHOLD_HI = ADC_THRESHOLD_HI_saved;
    ADC->THRESHOLD_LO = ADC_THRESHOLD_LO_saved;
    //The five IRQs are linked to a real ISR. If any of the five IRQs
    //triggered, then pend their ISR
    //Capture the wake source from the BLE_REASON_RESET register
    if ((CKGEN_SOC->REASON_RST == 0) &&
        (CKGEN_BLE->REASON_RST >= WAKENED_FROM_IO9) && 
          (CKGEN_BLE->REASON_RST <= WAKENED_FROM_IO13) && 
            gpioWakeBitMask) {
              if ((((CKGEN_BLE->REASON_RST & WAKENED_FROM_IO9) == WAKENED_FROM_IO9) && (GPIO->IE & GPIO_Pin_9))   ||
                  (((CKGEN_BLE->REASON_RST & WAKENED_FROM_IO10) == WAKENED_FROM_IO10) && (GPIO->IE & GPIO_Pin_10)) ||
                    (((CKGEN_BLE->REASON_RST & WAKENED_FROM_IO11) == WAKENED_FROM_IO11) && (GPIO->IE & GPIO_Pin_11)) ||
                      (((CKGEN_BLE->REASON_RST & WAKENED_FROM_IO12) == WAKENED_FROM_IO12) && (GPIO->IE & GPIO_Pin_12)) ||
                        (((CKGEN_BLE->REASON_RST & WAKENED_FROM_IO13) == WAKENED_FROM_IO13) && (GPIO->IE & GPIO_Pin_13))) {
                          NVIC->ISPR[0] = 1<<GPIO_IRQn;
                        }
            }
    
    // Disable the STANDBY mode 
    if (sleepMode == SLEEPMODE_NOTIMER) {
      BLUE_CTRL->TIMEOUT &= ~(LOW_POWER_STANDBY<<28);
    }
    
    // Disable the SLEEP Timer 
    if (sleepTimerEnabled) {
      HAL_VTimer_Stop(SLEEP_TIMER);
    }

    /* Restore the System Control register to indicate which HS crystal is used */
    SYSTEM_CTRL->CTRL = SYS_Ctrl_saved;

    // Wait until the HS clock is ready.
    // If SLEEPMODE_NOTIMER is set, wait the LS clock is ready.  
    if (sleepMode == SLEEPMODE_NOTIMER) {
      DeviceConfiguration(FALSE, TRUE);
    } else {
      DeviceConfiguration(FALSE, FALSE);
    }

    /* If the HS is a 32 MHz */
    if (SYS_Ctrl_saved & 1) {
      /* AHB up converter command register write*/
      AHBUPCONV->COMMAND = 0x15;
    }
  }
  
  //We can clear PRIMASK to reenable global interrupt operation.
  __enable_irq();

}

uint8_t BlueNRG_Sleep(SleepModes sleepMode, 
                      uint8_t gpioWakeBitMask, 
                      uint8_t gpioWakeLevelMask, 
                      uint32_t sleep_time)
{
  uint8_t sleepTimerEnabled=FALSE;
  SleepModes app_sleepMode, ble_sleepMode, sleepMode_allowed;

  if (gpioWakeBitMask) {
    SYSTEM_CTRL->WKP_IO_IS = gpioWakeLevelMask;
    SYSTEM_CTRL->WKP_IO_IE = gpioWakeBitMask;
  } else {
    SYSTEM_CTRL->WKP_IO_IS = 0;
    SYSTEM_CTRL->WKP_IO_IE = 3; // Only wakeup source are GPIO9-GPIO10 that are also Jlink pins
  }
  
  if (sleep_time) {
    if (HAL_VTimerStart_ms(SLEEP_TIMER, sleep_time) != 0) {
      return BLUENRG_SLEEP_CONFIGURATION_ERROR;
    } else {
      /* The BTLE_StackTick() is necessary to activate the sleep timer */
      BTLE_StackTick();
      sleepTimerEnabled = TRUE;
    }
  }
  
  /* Mask all the interrupt */
  __disable_irq();

  ble_sleepMode = (SleepModes)BlueNRG_Stack_Perform_Deep_Sleep_Check();
  app_sleepMode = App_SleepMode_Check(sleepMode);
  sleepMode_allowed = MIN(app_sleepMode, sleepMode);
  sleepMode_allowed = MIN(ble_sleepMode, sleepMode_allowed);

#ifdef DEBUG_SLEEP_MODE
  sleepMode_selected[sleepMode_allowed]++;
#endif

  if (sleepMode_allowed == SLEEPMODE_RUNNING) {
    __enable_irq();
    if (sleep_time)
      HAL_VTimer_Stop(SLEEP_TIMER);
     return SUCCESS;
  }
  
  if (sleepMode_allowed == SLEEPMODE_CPU_HALT) {
    BlueNRG_IdleSleep();
    if (sleep_time)
      HAL_VTimer_Stop(SLEEP_TIMER);
    /* Unmask all the interrupt */
    __enable_irq();
    return SUCCESS;
  }
  
  BlueNRG_InternalSleep(sleepMode_allowed, gpioWakeBitMask, sleepTimerEnabled);
  
  return SUCCESS;
}

uint8_t BlueNRG_WakeupSource(void)
{
  if ((CKGEN_SOC->REASON_RST == 0) &&
      (CKGEN_BLE->REASON_RST >= WAKENED_FROM_IO9) && 
      (CKGEN_BLE->REASON_RST <= WAKENED_FROM_IO13)) {
    if ((CKGEN_BLE->REASON_RST & WAKENED_FROM_IO9) == WAKENED_FROM_IO9) {
      return WAKEUP_IO9;
    }
    if ((CKGEN_BLE->REASON_RST & WAKENED_FROM_IO10) == WAKENED_FROM_IO10) {
      return WAKEUP_IO10;
    }
    if ((CKGEN_BLE->REASON_RST & WAKENED_FROM_IO11) == WAKENED_FROM_IO11) {
      return WAKEUP_IO11;
    }
    if ((CKGEN_BLE->REASON_RST & WAKENED_FROM_IO12) == WAKENED_FROM_IO12) {
      return WAKEUP_IO12;
    }
    if ((CKGEN_BLE->REASON_RST & WAKENED_FROM_IO13) == WAKENED_FROM_IO13) {
      return WAKEUP_IO13;
    }
  }
  if ((CKGEN_SOC->REASON_RST == 0) &&
      ((CKGEN_BLE->REASON_RST & WAKENED_FROM_BLUE_TIMER1) == WAKENED_FROM_BLUE_TIMER1)) {
    return WAKEUP_SLEEP_TIMER1;
  }
  if ((CKGEN_SOC->REASON_RST == 0) &&
      ((CKGEN_BLE->REASON_RST & WAKENED_FROM_BLUE_TIMER2) == WAKENED_FROM_BLUE_TIMER2)) {
    return WAKEUP_SLEEP_TIMER2;
  }
  if ((CKGEN_SOC->REASON_RST == 0) &&
      ((CKGEN_BLE->REASON_RST & WAKENED_FROM_POR) == WAKENED_FROM_POR)) {
    return WAKEUP_POR;
  }
  if ((CKGEN_SOC->REASON_RST == 0) &&
      ((CKGEN_BLE->REASON_RST & WAKENED_FROM_BOR) == WAKENED_FROM_BOR)) {
    return WAKEUP_BOR;
  }
  if (CKGEN_SOC->REASON_RST == 2) {
    return WAKEUP_SYS_RESET_REQ;
  }
  if (CKGEN_SOC->REASON_RST == 4) {
    return WAKEUP_RESET_WDG;
  }

  return NO_WAKEUP_RESET;
}
