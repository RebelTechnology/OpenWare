#include "stm32f4xx_hal.h"
#include "HAL_ToggleSwitches.h"

uint16_t usiTogCheck;
uint8_t rgSwitches[7], rgSwitchResult[7];

void HAL_Toggles_ClearPullups(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
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
	GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SW3_GPIO_Port, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = SW4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SW4_GPIO_Port, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = SW5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SW5_GPIO_Port, &GPIO_InitStruct);
}

void HAL_Toggles_reset(void)
{
	uint16_t x;

	SWL1_OUT(GPIO_PIN_RESET);
	SWL2_OUT(GPIO_PIN_RESET);
	SWL3_OUT(GPIO_PIN_RESET);
	SWL4_OUT(GPIO_PIN_RESET);
	SWL5_OUT(GPIO_PIN_RESET);
	
	for(x=1000; x; x--){}
}

// 16-bit Result Format: [0|0|0|0|SW6H|SW6L|SW5H|SW5L|SW4H|SW4L|SW3H|SW3L|SW2H|SW2L|SW1H|SW1L]
uint16_t HAL_Toggles_read(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	uint16_t usiTogMask = 0, x;
	
	//___ First Group ________________________________________________________________________________________
	// Reset Toggles
	HAL_Toggles_reset();
	
	// Clear Pullups
	HAL_Toggles_ClearPullups();
	
	// Make SWL5 low
	GPIO_InitStruct.Pin = SW5_Pin; 	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW5_GPIO_Port, &GPIO_InitStruct);
	SWL5_OUT(GPIO_PIN_RESET);
	
	// Pull up SWL1
	GPIO_InitStruct.Pin = SW1_Pin; GPIO_InitStruct.Mode = GPIO_MODE_INPUT; GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW1_GPIO_Port, &GPIO_InitStruct);
	for(x=10000; x; x--){}

	// Read Switch 1 High
	rgSwitches[1]  = (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin)<<1);
		
	// Clear Pullups
	HAL_Toggles_ClearPullups();
		
	// Make SWL5 low
	GPIO_InitStruct.Pin = SW5_Pin; 	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW5_GPIO_Port, &GPIO_InitStruct);
	SWL5_OUT(GPIO_PIN_RESET);
		
	// Pull up SWL2	
	GPIO_InitStruct.Pin = SW2_Pin;  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW2_GPIO_Port, &GPIO_InitStruct);
	for(x=10000; x; x--){}
	
	// Read Switch 1 Low
	rgSwitches[1] |= (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin)<<0);
		
	// Clear Pullups
	HAL_Toggles_ClearPullups();
	
	// Make SWL5 low
	GPIO_InitStruct.Pin = SW5_Pin; 	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW5_GPIO_Port, &GPIO_InitStruct);
	SWL5_OUT(GPIO_PIN_RESET);
		
	// Pull up SWL3
	GPIO_InitStruct.Pin = SW3_Pin;  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW3_GPIO_Port, &GPIO_InitStruct);
	for(x=10000; x; x--){}
			
	// Read Switch 2 High
	rgSwitches[2] = (HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin)<<1);
		
	// Clear Pullups
	HAL_Toggles_ClearPullups();
		
	// Make SWL5 low
	GPIO_InitStruct.Pin = SW5_Pin; 	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW5_GPIO_Port, &GPIO_InitStruct);
	SWL5_OUT(GPIO_PIN_RESET);
		
	// Pull up SWL4	
	GPIO_InitStruct.Pin = SW4_Pin;  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW4_GPIO_Port, &GPIO_InitStruct);
	for(x=10000; x; x--){}
		
	// Read Switch 2 Low
	rgSwitches[2] |= (HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin)<<0);	
		
	// Clear Pullups
	HAL_Toggles_ClearPullups();
/*	
	//___ Second Group ________________________________________________________________________________________
	// Reset Toggles
	HAL_Toggles_reset();
		
	// Make SWL4 low
	HAL_Toggles_reset();
	GPIO_InitStruct.Pin = SW4_Pin; 	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW4_GPIO_Port, &GPIO_InitStruct);
	
	// Pull up SWL1
	GPIO_InitStruct.Pin = SW1_Pin; GPIO_InitStruct.Mode = GPIO_MODE_INPUT; GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW1_GPIO_Port, &GPIO_InitStruct);
	for(x=10000; x; x--){}

	// Read Switch 3 Low
	rgSwitches[3]  = (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin)<<0);
		
	// Clear Pullups
	HAL_Toggles_ClearPullups();
		
	// Pull up SWL2	
	GPIO_InitStruct.Pin = SW2_Pin;  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW2_GPIO_Port, &GPIO_InitStruct);
	for(x=10000; x; x--){}
	
	// Read Switch 1 High
	rgSwitches[3]  |= (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin)<<1);
		
	// Clear Pullups
	HAL_Toggles_ClearPullups();
		
	// Pull up SWL3
	GPIO_InitStruct.Pin = SW3_Pin;  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW3_GPIO_Port, &GPIO_InitStruct);
	for(x=10000; x; x--){}
			
	// Read Switch 4 High
	rgSwitches[4]  = (HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin)<<1);
		
	// Clear Pullups
	HAL_Toggles_ClearPullups();
		
	// Pull up SWL5	
	GPIO_InitStruct.Pin = SW5_Pin;  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW5_GPIO_Port, &GPIO_InitStruct);
	for(x=10000; x; x--){}
		
	// Read Switch 4 Low
	rgSwitches[4] |= (HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin)<<0);	
		
/*		
	for(x=100; x; x--){}
		
	// Make SWL4 low
	HAL_Toggles_reset();
	GPIO_InitStruct.Pin = SW4_Pin; 	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW4_GPIO_Port, &GPIO_InitStruct);
	SWL4_OUT(GPIO_PIN_RESET);
	
	// Pull up SWL1 and SWL2
	GPIO_InitStruct.Pin = SW1_Pin; GPIO_InitStruct.Mode = GPIO_MODE_INPUT; GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW1_GPIO_Port, &GPIO_InitStruct);
		
	GPIO_InitStruct.Pin = SW2_Pin;  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW2_GPIO_Port, &GPIO_InitStruct);
	
	for(x=100; x; x--){}

	// Read Switch 3
	rgSwitches[3]  = (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin)<<0);
	rgSwitches[3] |= (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin)<<1);
		
	// Make SWL4 low
	HAL_Toggles_reset();
	GPIO_InitStruct.Pin = SW4_Pin; 	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW4_GPIO_Port, &GPIO_InitStruct);
	SWL4_OUT(GPIO_PIN_RESET);
		
	// Pull up SWL3 and SWL5
	GPIO_InitStruct.Pin = SW3_Pin;  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW3_GPIO_Port, &GPIO_InitStruct);
		
	GPIO_InitStruct.Pin = SW5_Pin;  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW5_GPIO_Port, &GPIO_InitStruct);
	
	for(x=100; x; x--){}
	
	// Read Switch 4
	rgSwitches[4]  = (HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin)<<1);
	rgSwitches[4] |= (HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin)<<0);	
		
/*
	for(x=10; x; x--){}
			
	// Make switch line 4 low
	HAL_Toggles_reset();
	GPIO_InitStruct.Pin = SW4_Pin; 
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW4_GPIO_Port, &GPIO_InitStruct);
	SWL4_OUT(GPIO_PIN_RESET);
	
	for(x=10; x; x--){}

	// Read Switches 3 & 4
	rgSwitches[3]  = (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin)<<0);
	rgSwitches[3] |= (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin)<<1);
		
	// Make switch line 4 low
	HAL_Toggles_reset();
	GPIO_InitStruct.Pin = SW4_Pin; 
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW4_GPIO_Port, &GPIO_InitStruct);
	SWL4_OUT(GPIO_PIN_RESET);
	
	for(x=10; x; x--){}
		
	rgSwitches[4]  = (HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin)<<1);
	rgSwitches[4] |= (HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin)<<0);	
			
	for(x=10; x; x--){}
		
	// Make switch line 3 low
	HAL_Toggles_reset();
	GPIO_InitStruct.Pin = SW3_Pin; 
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW3_GPIO_Port, &GPIO_InitStruct);
	SWL3_OUT(GPIO_PIN_RESET);
	
	for(x=10; x; x--){}

	// Read Switches 5 & 6
	rgSwitches[5]  = (HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin)<<1);
	rgSwitches[5] |= (HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin)<<0);

	// Make switch line 3 low
	HAL_Toggles_reset();
	GPIO_InitStruct.Pin = SW3_Pin; 
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(SW3_GPIO_Port, &GPIO_InitStruct);
	SWL3_OUT(GPIO_PIN_RESET);
	
	for(x=10; x; x--){}
		
	rgSwitches[6]  = (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin)<<1);
	rgSwitches[6] |= (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin)<<0);	
		
	for(x=10; x; x--){}
*/
	// Update result	
	rgSwitchResult[1] = 3-rgSwitches[1];
	rgSwitchResult[2] = 3-rgSwitches[2];
	rgSwitchResult[3] = 3-rgSwitches[3];
	rgSwitchResult[4] = 3-rgSwitches[4];
	rgSwitchResult[5] = 3-rgSwitches[5];
	rgSwitchResult[6] = 3-rgSwitches[6];
	
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
