#include "stm32f4xx_hal.h"
#include "HAL_Encoders.h"

#include <string.h>

SPI_HandleTypeDef* Encoders_SPIConfig;

// Pin Control
#define pbarCS(state)		HAL_GPIO_WritePin(ENC_nCS_GPIO_Port,  ENC_nCS_Pin,  (GPIO_PinState)state)

uint16_t rgENC_Values[7];
	 
//_____ Functions _____________________________________________________________________________________________________
// Port and Chip Setup
void Encoders_readAll(void)
{ 
	uint16_t x  = 150;
	
	pbarCS(0);
	while(--x){__NOP();}
	// *** The minimum NOP delay for proper operation seems to be 150 ***
	HAL_SPI_Receive(Encoders_SPIConfig, (uint8_t*)rgENC_Values, 14, 100);
	pbarCS(1);
}

void Encoders_readSwitches(void)
{ 
	uint16_t x  = 150;
	
	pbarCS(0);
	while(--x){__NOP();}
	// *** The minimum NOP delay for proper operation seems to be 150 ***
	HAL_SPI_Receive(Encoders_SPIConfig, (uint8_t*)rgENC_Values, 2, 100);
	pbarCS(1);
}


//_____ Initialisaion _________________________________________________________________________________________________
void Encoders_init (SPI_HandleTypeDef *spiconfig)
{
	Encoders_SPIConfig = spiconfig;
}

