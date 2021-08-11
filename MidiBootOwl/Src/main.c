/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "device.h"
#include "errorhandlers.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;

SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FSMC_Init(void);
static void MX_IWDG_Init(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

static void SDRAM_Init(void){
    /*##-1- Configure the SRAM device ##########################################*/
  /* SRAM device configuration */ 
  hsram1.Instance  = FSMC_NORSRAM_DEVICE;
  hsram1.Extended  = FSMC_NORSRAM_EXTENDED_DEVICE;

  FSMC_NORSRAM_TimingTypeDef Timing;
  
  Timing.AddressSetupTime       = 2;
  Timing.AddressHoldTime        = 1;
  Timing.DataSetupTime          = 2;
  Timing.BusTurnAroundDuration  = 1;
  Timing.CLKDivision            = 2;
  Timing.DataLatency            = 2;
  Timing.AccessMode             = FSMC_ACCESS_MODE_A;

  hsram1.Init.NSBank             = FSMC_NORSRAM_BANK3;
  hsram1.Init.DataAddressMux     = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType         = FSMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth    = FSMC_NORSRAM_MEM_BUS_WIDTH_16;// = SRAM_MEMORY_WIDTH;
  hsram1.Init.BurstAccessMode    = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WrapMode           = FSMC_WRAP_MODE_DISABLE;
  hsram1.Init.WaitSignalActive   = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation     = FSMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal         = FSMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode       = FSMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait   = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst         = FSMC_WRITE_BURST_DISABLE;

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    Error_Handler();
  }
}

void SystemInit_ExtMemCtl(void)
{
  __HAL_RCC_FSMC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;

  /* GPIOD configuration */
  GPIO_InitStruct.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_4  | GPIO_PIN_5  | 
    GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_11 |
    GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* GPIOE configuration */
  GPIO_InitStruct.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_3 |  
    GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_6  | GPIO_PIN_7 |
    GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_11|
    GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* GPIOF configuration */
  GPIO_InitStruct.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_3  | 
    GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_12 | GPIO_PIN_13 |
    GPIO_PIN_14 | GPIO_PIN_15;      
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /* GPIOG configuration */
  GPIO_InitStruct.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_3 | 
    GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_10;      
  /*                                 GPIO_Pin_4  | GPIO_Pin_5  |GPIO_Pin_9;       */
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

void setup();
void loop(void);
void setMessage(const char* msg); 

typedef  void (*pFunction)(void);

static int testMagic(){
  return *OWLBOOT_MAGIC_ADDRESS == OWLBOOT_MAGIC_NUMBER;
}

static int testButton(){
#ifdef USE_BOOT1_PIN
  return !(BOOT1_GPIO_Port->IDR & BOOT1_Pin);
#else
  return 0;
#endif
}

static int testNoProgram(){
  return ((*(__IO uint32_t*)(APPLICATION_ADDRESS + 4)) & 0xFFF00000 ) != 0x08000000;
  /* Check Vector Table: Test if valid Reset_Handler pointer is programmed after APPLICATION_ADDRESS */
}

static int testWatchdogReset(){
  return __HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET;
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  SystemInit_ExtMemCtl();
  MX_GPIO_Init();
  MX_IWDG_Init();

  if(testMagic()){
    // we're going to boot
    setMessage("Bootloader starting");
  }else if(testButton()){
    setMessage("Bootloader requested");
  }else if(testWatchdogReset()){
    error(RUNTIME_ERROR, "Watchdog reset");
  }else if(testNoProgram()){
    error(RUNTIME_ERROR, "No valid firmware");
  }else{
    // jump to application code

    /* Disable all interrupts */
    RCC->CIR = 0x00000000;

    /* Jump to user application */
    uint32_t JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
    pFunction jumpToApplication = (pFunction) JumpAddress;
    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
    jumpToApplication();
    for(;;);
  }
  /* Clear reset flags */
  __HAL_RCC_CLEAR_RESET_FLAGS();

  /* Clear magic */
  *OWLBOOT_MAGIC_ADDRESS = 0;

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_FSMC_Init();
  MX_IWDG_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */

  SDRAM_Init();

  // Initialise
  setup();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    loop();
#ifdef USE_IWDG
    IWDG->KR = 0xaaaa; // reset the watchdog timer
#endif
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */
#ifdef USE_IWDG
  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_128;
  hiwdg.Init.Reload = 8*(30000/128);
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */
#endif
  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, LED2_Pin|LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4|FLASH_nCS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PUSHBUTTON_Pin */
  GPIO_InitStruct.Pin = PUSHBUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : LED2_Pin LED1_Pin */
  GPIO_InitStruct.Pin = LED2_Pin|LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PE6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PC13 PC14 PC15 PC0 
                           PC1 PC2 PC3 PC6 
                           PC7 PC8 PC9 PC10 
                           PC11 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0 
                          |GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_6 
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10 
                          |GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PF6 PF7 PF8 PF9 
                           PF10 PF11 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9 
                          |GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 
                           PA4 PA5 PA6 PA7 
                           PA8 PA10 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7 
                          |GPIO_PIN_8|GPIO_PIN_10|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC4 FLASH_nCS_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_4|FLASH_nCS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB10 PB11 
                           PB12 PB13 PB14 PB15 
                           PB4 PB5 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11 
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PG6 PG7 PG8 PG9 
                           PG11 PG12 PG13 PG14 
                           PG15 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9 
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14 
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : PD2 PD3 PD6 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 10, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

}

/* FSMC initialization function */
static void MX_FSMC_Init(void)
{

  /* USER CODE BEGIN FSMC_Init 0 */

  /* USER CODE END FSMC_Init 0 */

  FSMC_NORSRAM_TimingTypeDef Timing = {0};

  /* USER CODE BEGIN FSMC_Init 1 */

  /* USER CODE END FSMC_Init 1 */

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FSMC_NORSRAM_DEVICE;
  hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FSMC_NORSRAM_BANK3;
  hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  hsram1.Init.PageSize = FSMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 2;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 2;
  Timing.BusTurnAroundDuration = 1;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FSMC_Init 2 */

  /* USER CODE END FSMC_Init 2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
#ifdef DEBUG
  __asm__("BKPT");
#else
  NVIC_SystemReset();
#endif
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
#ifdef DEBUG
  __asm__("BKPT");
#else
  NVIC_SystemReset();
#endif
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
