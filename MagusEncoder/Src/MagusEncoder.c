#include "stm32f0xx_hal.h"
#include "MagusEncoder.h"
#include "main.h"

extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

// Variables
uint8_t bSwitch_ENC[7];
uint8_t bSwitch_ENC_Prev[7];
uint8_t seqA[7];
uint8_t seqB[7];

uint16_t rgENC_Data[7];

void Encoders_Init (void)
{
	// Reset encoder values
	rgENC_Data[1]  = 0x3FFF;
	rgENC_Data[2]  = 0x3FFF;
	rgENC_Data[3]  = 0x3FFF;
	rgENC_Data[4]  = 0x3FFF;
	rgENC_Data[5]  = 0x3FFF;
	rgENC_Data[6]  = 0x3FFF;
	
	// Reset switch values
	bSwitch_ENC[1] = 0;
	bSwitch_ENC[2] = 0;
	bSwitch_ENC[3] = 0;
	bSwitch_ENC[4] = 0;
	bSwitch_ENC[5] = 0;
	bSwitch_ENC[6] = 0;
	
		// Configure and start TIM Encoders
  __HAL_TIM_SET_COUNTER(&htim1, INT16_MAX/2);
  __HAL_TIM_SET_COUNTER(&htim2, INT16_MAX/2);
	__HAL_TIM_SET_COUNTER(&htim3, INT16_MAX/2);
	HAL_TIM_Encoder_Start_IT(&htim1, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start_IT(&htim3, TIM_CHANNEL_ALL);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	// Determine which encoder has changed and set value
  if		  (htim == &htim1) rgENC_Data[1] = __HAL_TIM_GET_COUNTER(&htim1); 
  else if (htim == &htim2) rgENC_Data[2] = __HAL_TIM_GET_COUNTER(&htim2);
	else if (htim == &htim3) rgENC_Data[4] = __HAL_TIM_GET_COUNTER(&htim3);
}

void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
	switch(pin)
	{
		case ENC3_A_Pin: 
		case ENC3_B_Pin:	Encoder_Interrupt(3); break;
		
		case ENC5_A_Pin: 
		case ENC5_B_Pin:	Encoder_Interrupt(5); break;
		
		case ENC6_A_Pin: 
		case ENC6_B_Pin:	Encoder_Interrupt(6); break;
		
		case SPI_NCS_Pin:	send_SPI(); 					break;
	}
}
	
void Encoder_Interrupt(uint8_t ENC_i)
{
	uint8_t ucA_Val, ucB_Val;
	
	// Read Encoder pins
	switch(ENC_i)
	{	
		// Read encoder pins
		case 3: ucA_Val = HAL_GPIO_ReadPin(ENC3_A_GPIO_Port, ENC3_A_Pin);
						ucB_Val = HAL_GPIO_ReadPin(ENC3_B_GPIO_Port, ENC3_B_Pin);		break;

		case 5: ucA_Val = HAL_GPIO_ReadPin(ENC5_A_GPIO_Port, ENC5_A_Pin);
						ucB_Val = HAL_GPIO_ReadPin(ENC5_B_GPIO_Port, ENC5_B_Pin);		break;
	
		case 6: ucA_Val = HAL_GPIO_ReadPin(ENC6_A_GPIO_Port, ENC6_A_Pin);
						ucB_Val = HAL_GPIO_ReadPin(ENC6_B_GPIO_Port, ENC6_B_Pin);		break;
	}
	
	// Record the A and B sequences
	seqA[ENC_i] <<= 1;	
	seqA[ENC_i]  |= ucA_Val; 

	seqB[ENC_i] <<= 1;	
	seqB[ENC_i]  |= ucB_Val; 

	// Mask the MSB four bits
	seqA[ENC_i] &= 0x0F;
	seqB[ENC_i]	&= 0x0F;

	// Check for a turn
	if (seqA[ENC_i] == 0x09 && seqB[ENC_i] == 0x03) {rgENC_Data[ENC_i]-=2;}
	if (seqA[ENC_i] == 0x03 && seqB[ENC_i] == 0x09) {rgENC_Data[ENC_i]+=2;}
}

	
void Encoders_Main (void)
{
	uint16_t usiSW_States;
	
	// Clear ChangeReady Pin
	HAL_GPIO_WritePin(CHANGE_RDY_GPIO_Port, CHANGE_RDY_Pin, GPIO_PIN_RESET);
	
	// Read switches for a change
	usiSW_States  = (1-HAL_GPIO_ReadPin(ENC1_SW_GPIO_Port, ENC1_SW_Pin))<<0;
	usiSW_States |= (1-HAL_GPIO_ReadPin(ENC2_SW_GPIO_Port, ENC2_SW_Pin))<<1;
	usiSW_States |= (1-HAL_GPIO_ReadPin(ENC3_SW_GPIO_Port, ENC3_SW_Pin))<<2;
	usiSW_States |= (1-HAL_GPIO_ReadPin(ENC4_SW_GPIO_Port, ENC4_SW_Pin))<<3;
	usiSW_States |= (1-HAL_GPIO_ReadPin(ENC5_SW_GPIO_Port, ENC5_SW_Pin))<<4;
	usiSW_States |= (1-HAL_GPIO_ReadPin(ENC6_SW_GPIO_Port, ENC6_SW_Pin))<<5;
	
	// Update Switch states
	rgENC_Data[0] = usiSW_States;
}

void send_SPI(void)
{	
	// Send data
  /* HAL_SPI_Transmit(&hspi1, (uint8_t*)&rgENC_Data, 14, 100); */
  HAL_SPI_Transmit_DMA(&hspi1, (uint8_t*)&rgENC_Data, 14);
}

