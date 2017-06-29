#include "stm32f4xx_hal.h"
#include "HAL_ToggleSwitches.h"

uint16_t usiTogCheck;


void HAL_Toggles_reset(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// Set all lines low (to stop a slow discharge into a high impedance input)
	SWL1_OUT(GPIO_PIN_RESET);
	SWL2_OUT(GPIO_PIN_RESET);
	SWL3_OUT(GPIO_PIN_RESET);
	SWL4_OUT(GPIO_PIN_RESET);
	SWL5_OUT(GPIO_PIN_RESET);
	
		// Set all lines high
	SWL1_OUT(GPIO_PIN_SET);
	SWL2_OUT(GPIO_PIN_SET);
	SWL3_OUT(GPIO_PIN_SET);
	SWL4_OUT(GPIO_PIN_SET);
	SWL5_OUT(GPIO_PIN_SET);
	
	// Make all switch lines inputs
	GPIO_InitStruct.Pin = SW1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SW1_GPIO_Port, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = SW2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SW2_GPIO_Port, &GPIO_InitStruct);
	
  GPIO_InitStruct.Pin = SW3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(SW3_GPIO_Port, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = SW4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(SW4_GPIO_Port, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = SW5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(SW5_GPIO_Port, &GPIO_InitStruct);
}

// 16-bit Result Format: [0|0|0|0|SW6H|SW6L|SW5H|SW5L|SW4H|SW4L|SW3H|SW3L|SW2H|SW2L|SW1H|SW1L]
uint16_t HAL_Toggles_read(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	uint16_t usiTogMask = 0;
	
	// Reset all toggle lines
	HAL_Toggles_reset();
	
	// Make switch line 1 an output
	GPIO_InitStruct.Pin = SW1_Pin; GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW1_GPIO_Port, &GPIO_InitStruct);
	__nop(); __nop(); __nop(); __nop(); __nop();

	// Read Switches 1, 3 & 6 highs
	usiTogMask |= (HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin)<<1);
	usiTogMask |= (HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin)<<4);
	usiTogMask |= (HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin)<<11);

	// Reset all toggle lines
	HAL_Toggles_reset();
	
	// Make switch line 2 an output
	GPIO_InitStruct.Pin = SW2_Pin; GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW1_GPIO_Port, &GPIO_InitStruct);
	__nop(); __nop(); __nop(); __nop(); __nop();

	// Read Switches 1, 3 & 6 lows
	usiTogMask |= (HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin)<<0);
	usiTogMask |= (HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin)<<5);
	usiTogMask |= (HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin)<<10);

	// Reset all toggle lines
	HAL_Toggles_reset();

	// Make switch line 3 an output
	GPIO_InitStruct.Pin = SW3_Pin; GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW3_GPIO_Port, &GPIO_InitStruct);
	__nop(); __nop(); __nop(); __nop(); __nop();

	// Read Switch 2 & 4high
	usiTogMask |= (HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin)<<3);
	usiTogMask |= (HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin)<<7);
	
	// Reset all toggle lines
	HAL_Toggles_reset();
	
	// Make switch line 4 an output
	GPIO_InitStruct.Pin = SW4_Pin; GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW4_GPIO_Port, &GPIO_InitStruct);
	__nop(); __nop(); __nop(); __nop(); __nop();

	// Read Switch 2 & 5 low
	usiTogMask |= (HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin)<<2);
	usiTogMask |= (HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin)<<8);
	
	// Reset all toggle lines
	HAL_Toggles_reset();
	
	// Make switch line 5 an output
	GPIO_InitStruct.Pin = SW5_Pin; GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW5_GPIO_Port, &GPIO_InitStruct);
	__nop(); __nop(); __nop(); __nop(); __nop();
	
	// Read Switchs 4 low & 5 high
	usiTogMask |= (HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin)<<6);
	usiTogMask |= (HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin)<<9);

	// Reset all toggle lines
	HAL_Toggles_reset();

	// Return result mask
	return usiTogMask;
}

void HAL_Toggles_Scan(void)
{
	// Read current state
	uint16_t usiTogState = HAL_Toggles_read();
	
	// Compare to previous result and call callback if there has been a change
	if (usiTogCheck != usiTogState) HAL_Toggles_ChangeCallback(usiTogState);
	
	// Update check variable to compare to next check
	usiTogCheck = usiTogState;
}

void HAL_Toggles_ChangeCallback(uint16_t usiTogMask)
{
	
	__nop();
}
