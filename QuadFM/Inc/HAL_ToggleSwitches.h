#ifndef __HAL_TOGGLESWITCHES_H
#define __HAL_TOGGLESWITCHES_H

#ifdef __cplusplus
	extern "C" {
#endif

#include "stm32f4xx_hal.h"

#define SWL1_OUT(pinstate)  HAL_GPIO_WritePin(SW1_GPIO_Port, SW1_Pin, pinstate)
#define SWL2_OUT(pinstate)  HAL_GPIO_WritePin(SW2_GPIO_Port, SW2_Pin, pinstate)
#define SWL3_OUT(pinstate)  HAL_GPIO_WritePin(SW3_GPIO_Port, SW3_Pin, pinstate)
#define SWL4_OUT(pinstate)  HAL_GPIO_WritePin(SW4_GPIO_Port, SW4_Pin, pinstate)
#define SWL5_OUT(pinstate)  HAL_GPIO_WritePin(SW5_GPIO_Port, SW5_Pin, pinstate)
		
void HAL_Toggles_reset(void);
uint16_t HAL_Toggles_read(void);
void HAL_Toggles_Scan(void);
void HAL_Toggles_ChangeCallback(uint16_t usiTogMask);

		
#ifdef __cplusplus
}
#endif
#endif
