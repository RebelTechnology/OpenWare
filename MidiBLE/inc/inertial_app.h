/**
******************************************************************************
* @file    inertial_app.h
* @author  Central Labs
* @version V 1.0.0
* @date    May-2017
* @brief   Header for inertial_app.c module.
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
#ifndef __INERTIAL_APP_H
#define __INERTIAL_APP_H

/* Includes ------------------------------------------------------------------*/

#include "bluenrg1_stack.h"

typedef enum 
{
  SCUDO_APP_SUCCESS = 0x00,  	/*!< Success.*/
  SCUDO_APP_ERROR  	= 0x10  	/*!< Error.*/
} SCUDO_APP_Status;

/* Exported variables --------------------------------------------------------*/
extern volatile uint16_t ClassificationHandle;
extern volatile uint16_t ModeHandle;

SCUDO_APP_Status CLASSIFICATION_APP_Init(void);

SCUDO_APP_Status CLASSIFICATION_APP_add_char(uint16_t service_handle);
SCUDO_APP_Status MODE_APP_add_char(uint16_t service_handle);
SCUDO_APP_Status MIDI_APP_add_char(uint16_t service_handle);

SCUDO_APP_Status CLASSIFICATION_APP_DataUpdate(uint16_t service_handle, uint8_t value);
uint8_t MODE_APP_DataRead(uint16_t service_handle);
SCUDO_APP_Status MIDI_APP_DataUpdate(uint16_t service_handle, uint8_t channel, uint8_t note, uint8_t vector);
SCUDO_APP_Status MIDI_APP_Passthrough(uint16_t service_handle, uint8_t*);

#endif /* __INERTIAL_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
