#include "stm32f4xx_hal.h"
#include "HAL_Encoders.h"

#include <string.h>

SPI_HandleTypeDef* Encoders_SPIConfig;
 
// Variables
#define STATE_Idle				0
#define STATE_ContConv		1
#define STATE_Busy				1

#define TASK_readADC			0
#define TASK_setDAC				1

// SPI Read/Write bit
#define SPI_Read        	1
#define SPI_Write       	0

// Pin Control
#define pbarCS(state)		HAL_GPIO_WritePin(ENC_nCS_GPIO_Port,  ENC_nCS_Pin,  (GPIO_PinState)state)

uint8_t rgEncoderData[13];
uint16_t rgENC_Values[7];
	 
//_____ Functions _____________________________________________________________________________________________________
// Port and Chip Setup
void Encoders_readAll(void)
{
	pbarCS(0);
	HAL_Delay(1);
	HAL_SPI_Receive(Encoders_SPIConfig, rgEncoderData, 13, 1000);
	pbarCS(1);
	
	rgENC_Values[1] = (rgEncoderData[1]<<8)  | rgEncoderData[2];
	rgENC_Values[2] = (rgEncoderData[3]<<8)  | rgEncoderData[4];
	rgENC_Values[3] = (rgEncoderData[5]<<8)  | rgEncoderData[6];
	rgENC_Values[4] = (rgEncoderData[7]<<8)  | rgEncoderData[8];
	rgENC_Values[5] = (rgEncoderData[9]<<8)  | rgEncoderData[10];
	rgENC_Values[6] = (rgEncoderData[11]<<8) | rgEncoderData[12];
}


//_____ Initialisaion _________________________________________________________________________________________________
void Encoders_init (SPI_HandleTypeDef *spiconfig)
{
	Encoders_SPIConfig = spiconfig;
}

