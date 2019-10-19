/**
 ******************************************************************************
 * @file    steval_bluemic1_audio_in.c
 * @author  Central Lab
 * @version V1.0.0
 * @date    16-May-2017
 * @brief   This file provides the Audio In driver for the STEVAL-BLUEMIC-1
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
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
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "steval_bluemic1_audio_in.h"

/** @addtogroup BSP BSP
 * @{
 */

/** @defgroup BLUEMIC1_AUDIO_IN BLUEMIC1_AUDIO_IN
 * @{
 */
 
#define SaturaLH(N, L, H) (((N)<(L))?(L):(((N)>(H))?(H):(N)))
#define DEFAULT_VOLUME          0x08 

/** @defgroup AUDIO_IN_PrivateTypedef AUDIO_IN_PrivateTypedef
 * @{
 */

/**
 * @brief   HP filter state structure definition
 */
typedef struct 
{
  int32_t Z; 
  int32_t oldOut; 
  int32_t oldIn; 
}HP_FilterState_TypeDef;

/**
* @brief   Microphone internal structure definition
*/
typedef struct 
{
  uint32_t Sampling_Freq;                    /*!< Specifies the desired sampling frequency */
  
  uint8_t AudioInVolume;                     /*!< Specifies the desired sampling frequency */
  
  HP_FilterState_TypeDef HP_Filter;          /*!< HP filter state for each channel*/
  
  int16_t * PCM_Data;                        /*!< Takes track of the external PCM data buffer as passed by the user in the start function*/
  
} 
BLUEMIC1_AudioIn_HandlerTypeDef;

/**
  * @}
  */

/* Private variables ---------------------------------------------------------*/
 
static BLUEMIC1_AudioIn_HandlerTypeDef BLUEMIC1_AudioIn_Handler; 

/** @defgroup AUDIO_IN_Private_Functions AUDIO_IN_Private_Functions
 * @{
 */

/**
 * @brief  Transfer complete callback.
 * @param  None
 * @retval None
 */
void TC_IT_Callback(void);

/**
 * @brief  Half Transfer callback.
 * @param  None
 * @retval None
 */
void HT_IT_Callback(void);

/**
  * @}
  */

/** @defgroup AUDIO_IN_Exported_Functions AUDIO_IN_Exported_Functions
 * @{
 */

/**
* @brief  Initializes audio acquisition.
* @param  AudioFreq: Audio frequency to be configured for the peripherals.
* 		  Possible values are 8000, 16000
* @retval None
*/
void BSP_AUDIO_IN_Init(uint32_t AudioFreq) 
{
  /* GPIO Initialization */
  /* Configure GPIO_Pin_6 and GPIO_Pin_7 as PDM_DATA and PDM_CLOCK */
  GPIO_InitPdmDataPin6();
  GPIO_InitPdmClockPin7();
  
  ADC_InitType xADC_InitType;
  
  /* ADC Initialization */
  /* Enable ADC clock */
  SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_ADC, ENABLE);

  if(AudioFreq == 16000) 
  {
    xADC_InitType.ADC_DecimationRate = ADC_DecimationRate_100;
  }
  else if(AudioFreq == 8000)  
  {
    xADC_InitType.ADC_DecimationRate = ADC_DecimationRate_200;
  }
  
  xADC_InitType.ADC_Input = ADC_Input_Microphone;
  xADC_InitType.ADC_ConversionMode = ADC_ConversionMode_Continuous;
  xADC_InitType.ADC_ReferenceVoltage = ADC_ReferenceVoltage_0V;
  xADC_InitType.ADC_Attenuation = ADC_Attenuation_0dB;
  ADC_Init(&xADC_InitType);

  /* Enable ADC round converted data */  
  ADC_RoundConvertedData(ENABLE);
  
  BLUEMIC1_AudioIn_Handler.Sampling_Freq = AudioFreq;
  
  BSP_AUDIO_IN_SetVolume(DEFAULT_VOLUME);
}

/**
  * @brief  Start audio acquisition.
  * @param  pbuf: PCM buffer where the audio is stored
  * @param  buf_size: PCM buffer size
  * @retval None
  */
void BSP_AUDIO_IN_Record(int16_t* pbuf, uint32_t buf_size) 
{
  /* DMA configuration */ 
  DMA_InitType DMA_InitStructure;
  NVIC_InitType NVIC_InitStructure;
  
  /* Configure DMA peripheral */
  SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_DMA, ENABLE);
    
  /* Configure DMA TX channel */
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC->DATA_CONV);
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pbuf;  
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)(buf_size);  
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;    
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    
  DMA_Init(DMA_CH0, &DMA_InitStructure);
  
  /* Enable DMA_CH0 Transfer and half tranfer Complete interrupt */
  DMA_FlagConfig(DMA_CH0, DMA_FLAG_TC | DMA_FLAG_HT, ENABLE);

  /* Select DMA ADC CHANNEL 0 */
  DMA_SelectAdcChannel(DMA_ADC_CHANNEL0, ENABLE);
 
  /* Enable the DMA Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = DMA_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = HIGH_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);   
    
  /* Start new conversion */
  ADC_Cmd(ENABLE);
  
  /* Enable ADC DMA requests */
  DMA_Cmd(DMA_CH0, ENABLE);
  ADC_DmaCmd(ENABLE); 
  
  BLUEMIC1_AudioIn_Handler.PCM_Data = pbuf;
}

/**
  * @brief  Stop audio acquisition.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_Stop(void) 
{
  /* Stop new conversion */
  ADC_Cmd(DISABLE);
  
  /* Disable ADC DMA requests */
  DMA_Cmd(DMA_CH0, DISABLE);
  ADC_DmaCmd(DISABLE); 
}

/**
* @brief  Controls the audio in volume level.
* @param  Volume: Volume level to be set. This value has the same behaviour of the 	
Volume parameter of the PDM to PCM software decimation library. Other
strategies are possible in order to control the volume, for example
to act on the right bit shift amount of the DFSDM peripheral
Values must be in the range from 0 to 64
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise 
*/
void BSP_AUDIO_IN_SetVolume(uint8_t Volume) 
{
  BLUEMIC1_AudioIn_Handler.AudioInVolume = Volume;
}

/**
 * @brief  Transfer complete callback.
 * @param  None
 * @retval None
 */
void TC_IT_Callback(void)
{
  /* PCM data filtering */
  for (uint8_t i = (BLUEMIC1_AudioIn_Handler.Sampling_Freq/1000); i < (BLUEMIC1_AudioIn_Handler.Sampling_Freq/1000)*2; i++)
  {
    BLUEMIC1_AudioIn_Handler.HP_Filter.Z = BLUEMIC1_AudioIn_Handler.PCM_Data[i] * BLUEMIC1_AudioIn_Handler.AudioInVolume;
    BLUEMIC1_AudioIn_Handler.HP_Filter.oldOut = (0xFC * (BLUEMIC1_AudioIn_Handler.HP_Filter.oldOut +  BLUEMIC1_AudioIn_Handler.HP_Filter.Z - BLUEMIC1_AudioIn_Handler.HP_Filter.oldIn)) / 256;
    BLUEMIC1_AudioIn_Handler.HP_Filter.oldIn = BLUEMIC1_AudioIn_Handler.HP_Filter.Z;
    BLUEMIC1_AudioIn_Handler.PCM_Data[i] = SaturaLH(BLUEMIC1_AudioIn_Handler.HP_Filter.oldOut, -32760, 32760);
  }		  
  
  BSP_AUDIO_IN_AudioProcess((uint16_t*)&BLUEMIC1_AudioIn_Handler.PCM_Data[BLUEMIC1_AudioIn_Handler.Sampling_Freq/1000]);
}

/**
 * @brief  Half Transfer callback.
 * @param  None
 * @retval None
 */
void HT_IT_Callback(void)
{
  /* PCM data filtering */
  for (uint8_t i = 0; i < (BLUEMIC1_AudioIn_Handler.Sampling_Freq/1000); i++)
  {
    BLUEMIC1_AudioIn_Handler.HP_Filter.Z = BLUEMIC1_AudioIn_Handler.PCM_Data[i] * BLUEMIC1_AudioIn_Handler.AudioInVolume;
    BLUEMIC1_AudioIn_Handler.HP_Filter.oldOut = (0xFC * (BLUEMIC1_AudioIn_Handler.HP_Filter.oldOut +  BLUEMIC1_AudioIn_Handler.HP_Filter.Z - BLUEMIC1_AudioIn_Handler.HP_Filter.oldIn)) / 256;
    BLUEMIC1_AudioIn_Handler.HP_Filter.oldIn = BLUEMIC1_AudioIn_Handler.HP_Filter.Z;
    BLUEMIC1_AudioIn_Handler.PCM_Data[i] = SaturaLH(BLUEMIC1_AudioIn_Handler.HP_Filter.oldOut, -32760, 32760);
  }
    
  BSP_AUDIO_IN_AudioProcess((uint16_t*)BLUEMIC1_AudioIn_Handler.PCM_Data);
}

/**
* @brief  User callback when record buffer is filled.
* @param  None
* @retval None
*/
__weak void BSP_AUDIO_IN_AudioProcess(uint16_t* PCM_Buffer) {
  /* This function should be implemented by the user application.
  It is called into this driver when the current buffer is filled
  to prepare the next buffer pointer and its size. */
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
