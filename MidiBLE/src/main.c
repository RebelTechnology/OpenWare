
/******************** (C) COPYRIGHT 2015 STMicroelectronics ********************
* File Name          : main.c 
* Author             : Central Labs
* Version            : V1.0.0
* Date               : May-2017
* Description        : Example code for STEVAL-BLUEMIC-1 
********************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
*
* Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
* You may not use this file except in compliance with the License.
* You may obtain a copy of the License at:
*
*        http://www.st.com/software_license_agreement_liberty_v2
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright notice,
*      this list of conditions and the following disclaimer in the documentation
*      and/or other materials provided with the distribution.
*   3. Neither the name of STMicroelectronics nor the names of its contributors
*      may be used to endorse or promote products derived from this software
*      without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "peripheral_mngr_app.h"
#include "BlueVoice_config.h"
/* #include "steval_bluemic1.h" */
/* #include "steval_bluemic1_audio_in.h" */
#include "BlueNRG1.h"
#include "BlueNRG1_conf.h"
#include "sleep.h"
#include "clock.h"

#define MODE_PIN        GPIO_Pin_4
#define SPI_NCS_PIN     GPIO_Pin_1
#define SPI_CLK_PIN     GPIO_Pin_0
#define SPI_MOSI_PIN    GPIO_Pin_3
#define SPI_MISO_PIN    GPIO_Pin_2

/* Private variables ---------------------------------------------------------*/ 
volatile uint32_t lSystickCounter = 0;
extern uint16_t MidiServiceHandle;
extern uint16_t MidiHandle;
extern volatile uint8_t APP_PER_state;

/* Private function prototypes -----------------------------------------------*/
void init(void);
/* Private functions ---------------------------------------------------------*/

void readSpiPacket(){
  uint16_t delay = 100, timeout = 10000;
			
  // Delay
  /* while(delay--){} */

  uint8_t rxbuf[4];
  uint8_t txbuf[4] = {APP_PER_state, APP_PER_state, APP_PER_state, APP_PER_state};

  rxbuf[0]= SPI_ReceiveData();
  SPI_SendData(txbuf[0]);
  while (SPI_GetFlagStatus(SPI_FLAG_RNE) == RESET && timeout--){}
  rxbuf[1] = SPI_ReceiveData();
  SPI_SendData(txbuf[1]);
  while (SPI_GetFlagStatus(SPI_FLAG_RNE) == RESET && timeout--){}
  rxbuf[2] = SPI_ReceiveData();
  SPI_SendData(txbuf[2]);
  while (SPI_GetFlagStatus(SPI_FLAG_RNE) == RESET && timeout--){}
  rxbuf[3] = SPI_ReceiveData();
  SPI_SendData(txbuf[3]);
  if(timeout)
    MIDI_APP_Passthrough(MidiServiceHandle, rxbuf);
  else
    SPI_ClearRXFIFO();
}
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
  /* System initialization function */
  SystemInit();
	
  // Initialise peripherals
  init();

  /* BlueNRG-1 stack init */
  BlueNRG_Stack_Initialization(&BlueNRG_Stack_Init_params);
  
  /* BLE Initialization */
  PER_APP_Init_BLE();
  
  /* BlueVoice profile Initialization */
  //BV_APP_profile_init(AUDIO_SAMPLING_FREQUENCY);  
   
  /* BLE Service Initialization*/
  PER_APP_Service_Init();
  
  //BSP_AUDIO_IN_Init(AUDIO_SAMPLING_FREQUENCY);
  
  /*Set module in advertise mode*/
  PER_APP_Advertise(); 

  // todo: probably need to do something like this to read MIDI data
  /* int ret = MODE_APP_DataRead(MidiServiceHandle); */
  /* int ret = MODE_APP_DataRead(MidiHandle); */

  /* Infinite loop */
  while(1) 
  { 
    /* BLE Stack Tick */
    BTLE_StackTick();
		
    // Process any new data
    if(SPI_GetFlagStatus(SPI_FLAG_RNE) == SET)
      readSpiPacket();
  }
}

void init(void)
{
  GPIO_InitType GPIO_InitStructure;
  SPI_InitType SPI_InitStructure;
	
  /* SysTick initialization 1ms */  
  Clock_Init(); 
	
  // Enable SPI and GPIO clocks
  SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_GPIO | CLOCK_PERIPH_SPI, ENABLE);   
   
  // Configure Mode pin
  GPIO_InitStructure.GPIO_Pin = MODE_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Output;
  GPIO_InitStructure.GPIO_Pull = ENABLE;
  GPIO_InitStructure.GPIO_HighPwr = ENABLE;
  GPIO_Init(&GPIO_InitStructure);
  
  // Configure SPI pins
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = SPI_MISO_PIN;
  GPIO_InitStructure.GPIO_Mode = Serial0_Mode;
  GPIO_InitStructure.GPIO_Pull = ENABLE;
  GPIO_InitStructure.GPIO_HighPwr = DISABLE;
  GPIO_Init(&GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = SPI_MOSI_PIN;
  GPIO_InitStructure.GPIO_Mode = Serial0_Mode;
  GPIO_Init(&GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = SPI_CLK_PIN;
  GPIO_InitStructure.GPIO_Mode = Serial0_Mode;
  GPIO_Init(&GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = SPI_NCS_PIN;
  GPIO_InitStructure.GPIO_Mode = Serial0_Mode;
  GPIO_InitStructure.GPIO_Pull = ENABLE;
  GPIO_Init(&GPIO_InitStructure);
  	
  // Configure SPI in slave mode
  SPI_StructInit(&SPI_InitStructure);
  SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  /* SPI_InitStructure.SPI_BaudRate = 1000000; */
  SPI_InitStructure.SPI_BaudRate = 656250; // 656.25Kbps
  SPI_Init(&SPI_InitStructure);

  /* Clear RX and TX FIFO */
  SPI_ClearTXFIFO();
  SPI_ClearRXFIFO();
  
  /* Enable SPI functionality */
  SPI_Cmd(ENABLE);
  SPI_SlaveModeOutputCmd(ENABLE);
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
