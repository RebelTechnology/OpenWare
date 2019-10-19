/**
******************************************************************************
* @file    bluevoice_app.h
* @author  Central Labs
* @version V 1.0.0
* @date    May-2017
* @brief   Header for bluevoice_app.c module.
*******************************************************************************
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
********************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BLUEVOICE_APP_H
#define __BLUEVOICE_APP_H

/* Includes ------------------------------------------------------------------*/

#include "bluenrg1_stack.h"
#include "bluevoice_adpcm_bnrg1.h"

/** @addtogroup BLUEMIC_1_APP BLUEMIC_1_APP
 * @{
 */

/** @addtogroup BLUEMIC_1_BLUEVOICE_APP BLUEMIC_1_BLUEVOICE_APP
 * @{
 */

/** @defgroup BLUEVOICE_APP_Exported_Types BLUEVOICE_APP_Exported_Types
  * @{
  */

/**
  * @brief Audio Sampling Frequency enumerator
  */
typedef enum
{
  SAMPLING_FREQ_8000 = 8000,  
  SAMPLING_FREQ_16000 = 16000
} INPUT_FREQ_TypeDef;

/**
 * @brief BlueVoice application status
 */
typedef enum 
{
  BV_APP_SUCCESS = 0x00, /*!< BV_APP Success.*/
  BV_APP_ERROR = 0x10 /*!< BV_APP Error.*/
} BV_APP_Status;

/**
  * @}
  */

/** @defgroup BLUEVOICE_APP_Exported_Defines BLUEVOICE_APP_Exported_Defines
  * @{
  */
#define AUDIO_SAMPLING_FREQUENCY                (uint16_t)(SAMPLING_FREQ_8000)
#define PCM_BUFFER_SIZE                         AUDIO_SAMPLING_FREQUENCY/1000 * 2         /* 1ms */
/**
  * @}
  */

/* Exported variables --------------------------------------------------------*/

extern int16_t PCM_Buffer[];
extern volatile uint8_t tx_buffer_full;
extern BV_ADPCM_BNRG1_ProfileHandle_t tx_handle;
extern volatile uint8_t audio_streaming_active;

/* Exported functions ------------------------------------------------------- */

/** @defgroup BLUEVOICE_APP_Functions_Prototype BLUEVOICE_APP_Functions_Prototype
  * @{
  */
  
/**
 * @brief  This function is called to add BlueVoice characteristics.
 * @param  service_handle: handle of the service
 * @retval BV_APP_Status: BV_APP_SUCCESS if the configuration is ok, BV_ERROR otherwise.
 */
BV_APP_Status BV_APP_add_char(uint16_t service_handle);

/**
 * @brief  This function is called to init BlueVoice Profile.
 * @param  AudioFreq: audio sampling frequancy
 * @retval BV_APP_Status: BV_APP_SUCCESS if the configuration is ok, BV_ERROR otherwise.
 */
BV_APP_Status BV_APP_profile_init(uint32_t AudioFreq);

/**
 * @brief  This function is called to update audio characteristics.
 * @param  None
 * @retval None.
 */
void BV_APP_DataUpdate(void);

/**
 * @brief  Manage start and stop audio acquisition and streaming.
 * @param  None.
 * @retval None.
 */
void BV_APP_StartStop_ctrl(void);

/**
  * @}
  */
  
/**
  * @}
  */

/**
  * @}
  */


#endif /* __BLUEVOICE_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
