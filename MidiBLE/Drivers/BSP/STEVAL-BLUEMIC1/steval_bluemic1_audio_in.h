/**
  ******************************************************************************
  * @file    steval_bluemic1_audio_in.h
  * @author  Central Lab
  * @version V1.0.0
  * @date    16-May-2017
  * @brief   This file contains definitions for steval_bluemic1_audio_in.c file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STEVAL_BLUEMIC1_AUDIO_IN_H
#define __STEVAL_BLUEMIC1_AUDIO_IN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "steval_bluemic1.h"
  
/** @addtogroup BSP BSP
 * @{
 */

/** @addtogroup BLUEMIC1_AUDIO_IN BLUEMIC1_AUDIO_IN
 * @{
 */
 
/** @defgroup AUDIO_IN_FunctionPrototypes AUDIO_IN_FunctionPrototypes
 * @{
 */

void BSP_AUDIO_IN_Init(uint32_t AudioFreq);
void BSP_AUDIO_IN_Record(int16_t* pbuf, uint32_t buf_size);   
void BSP_AUDIO_IN_Stop(void);  
void BSP_AUDIO_IN_SetVolume(uint8_t Volume);
void BSP_AUDIO_IN_AudioProcess(uint16_t* PCM_Buffer);
void HT_IT_Callback(void);
void TC_IT_Callback(void);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */


#ifdef __cplusplus
}
#endif

#endif /* __STEVAL_BLUEMIC1_AUDIO_IN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
