/**
 ******************************************************************************
 * @file    bluevoice_app.c
 * @author  Central Labs
 * @version V 1.0.0
 * @date    May-2017
 * @brief   This file contains definitions for BlueVoice application.
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

/* Includes ------------------------------------------------------------------*/
#include "bluevoice_app.h"
#include "BlueNRG1_conf.h"
#include "sleep.h"  
#include "ble_const.h" 
#include "steval_bluemic1_audio_in.h"

void HT_IT_Callback(void)
{}

void TC_IT_Callback(void)
{}

/** @addtogroup BLUEMIC_1_APP BLUEMIC_1_APP
 * @{
 */

/** @defgroup BLUEMIC_1_BLUEVOICE_APP BLUEMIC_1_BLUEVOICE_APP
 * @{
 */

/** @defgroup BLUEVOICE_APP_Private_Defines BLUEVOICE_APP_Private_Defines
 * @{
 */
#define NAME_WEAR 'O', 'W', 'L', '-', 'B', 'L', 'E' 
#define PCM_SAMPLES_PER_MS                              (AUDIO_SAMPLING_FREQUENCY/1000)
/**
  * @}
  */ 

/** @defgroup BLUEVOICE_APP_Private_Variables BLUEVOICE_APP_Private_Variables
 * @{
 */
BV_ADPCM_BNRG1_uuid_t uuid;
volatile uint8_t ready = 0;
/**
  * @}
  */

/** @defgroup BLUEVOICE_APP_Private_Constant BLUEVOICE_APP_Private_Constant
 * @{
 */
static const uint8_t audio_adpcm_char_uuid[16] =
{
  0x1b,0xc5,0xd5,0xa5,0x02,0x00,0x36,0xac,0xe1,0x11,0x01,0x00,0x00,0x00,0x00,0x08
};
static const uint8_t audio_adpcm_sync_char_uuid[16] =
{
  0x1b,0xc5,0xd5,0xa5,0x02,0x00,0x36,0xac,0xe1,0x11,0x01,0x00,0x00,0x00,0x00,0x40
};
/**
  * @}
  */

/** @defgroup BLUEVOICE_APP_Exported_Variables BLUEVOICE_APP_Exported_Variables
 * @{
 */
int16_t PCM_Buffer[PCM_BUFFER_SIZE]; 
volatile uint8_t tx_buffer_full = 0;
BV_ADPCM_BNRG1_ProfileHandle_t tx_handle;
volatile uint8_t audio_streaming_active = 0;
/**
  * @}
  */

/** @defgroup BLUEVOICE_APP_Private_Functions BLUEVOICE_APP_Private_Functions
 * @{
 */

/**
 * @brief  This function is called to add BlueVoice characteristics.
 * @param  service_handle: handle of the service
 * @retval BV_APP_Status: BV_APP_SUCCESS if the configuration is ok, BV_ERROR otherwise.
 */
BV_APP_Status BV_APP_add_char(uint16_t service_handle)
{       
  BV_BNRG1_Status status;
  
  memcpy(uuid.CharAudioUUID,
              audio_adpcm_char_uuid,
              sizeof(audio_adpcm_char_uuid));
  memcpy(uuid.CharAudioSyncUUID,
              audio_adpcm_sync_char_uuid,
              sizeof(audio_adpcm_sync_char_uuid)); 
      
  status = BluevoiceADPCM_BNRG1_AddChar(uuid, service_handle, &tx_handle);
  if(status != BV_SUCCESS)
  {
    return BV_APP_ERROR;
  }

  return BV_APP_SUCCESS;
}

/**
 * @brief  This function is called to init BlueVoice Profile.
 * @param  AudioFreq: audio sampling frequancy
 * @retval BV_APP_Status: BV_APP_SUCCESS if the configuration is ok, BV_ERROR otherwise.
 */
BV_APP_Status BV_APP_profile_init(uint32_t AudioFreq)
{
  BV_ADPCM_BNRG1_Config_t BLUEVOICE_Config;
  BV_BNRG1_Status status;
  
  if(AudioFreq == SAMPLING_FREQ_8000)
  {
    BLUEVOICE_Config.sampling_frequency = FR_8000;   
  }
  else if(AudioFreq == SAMPLING_FREQ_16000)
  {
    BLUEVOICE_Config.sampling_frequency = FR_16000;    
  }
  BLUEVOICE_Config.channel_in = 1;
  BLUEVOICE_Config.channel_tot = 1;
  status = BluevoiceADPCM_BNRG1_SetConfig(&BLUEVOICE_Config);
  if(status != BV_SUCCESS)
  {
    return BV_APP_ERROR;
  }
  
  return BV_APP_SUCCESS;
}

/**
 * @brief  Manage start and stop audio acquisition and streaming.
 * @param  None.
 * @retval None.
 */
void BV_APP_StartStop_ctrl(void)
{
  if(audio_streaming_active)
  {   
    /* BSP_AUDIO_IN_Stop();     */
    audio_streaming_active = 0;
  }
  else
  {
    /* BSP_AUDIO_IN_Record(PCM_Buffer, PCM_BUFFER_SIZE);  */
    audio_streaming_active = 1;
  }
}

/**
* @brief  Audio Process function: BlueVoice buffer filling.
* @param  PCM_Buffer: PCM input buffer.
* @retval None.
*/
void BSP_AUDIO_IN_AudioProcess(uint16_t* PCM_Buffer)
{  
  BV_BNRG1_Status status;
  
  /*BlueVoice data filling*/
  if (BluevoiceADPCM_BNRG1_IsProfileConfigured())
  {
    status = BluevoiceADPCM_BNRG1_AudioIn((uint16_t*) PCM_Buffer, PCM_SAMPLES_PER_MS);
    if(status==BV_OUT_BUF_READY)
    {
      ready=1;
    }
  }
}

/**
 * @brief  This function is called to update audio characteristics.
 * @param  None
 * @retval None.
 */
void BV_APP_DataUpdate(void)
{
  if(audio_streaming_active)
  {
    if(ready && !tx_buffer_full)
    {            
      if(BluevoiceADPCM_BNRG1_SendData()==BV_INSUFFICIENT_RESOURCES)
      {
        tx_buffer_full = 1;
      }
      else
      {            
        ready = 0;
      }
    }     
  }    
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
