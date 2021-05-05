#include "HAL_Encoders.h"
#include "device.h"
#include <string.h>

SPI_HandleTypeDef* Encoders_SPIConfig;

// Pin Control
#define pbarCS(state)		HAL_GPIO_WritePin(ENC_nCS_GPIO_Port,  ENC_nCS_Pin,  (GPIO_PinState)state)
#define pbarRST(state)		HAL_GPIO_WritePin(ENC_NRST_GPIO_Port,  ENC_NRST_Pin,  (GPIO_PinState)state)

static int16_t rgENC_Values[7] = {0};

#ifdef STM32H7xx
static volatile uint16_t NOP_CNT = 600;
#else
static uint16_t NOP_CNT = 250; // 150 doesn't work in Release build
#endif

//_____ Functions _____________________________________________________________________________________________________
// Port and Chip Setup
void Encoders_readAll(void)
{ 
	volatile uint16_t x  = NOP_CNT;
	
	pbarCS(0);
	while(--x){__NOP();} // TODO: microsecond delay using CYCCNT
	// *** The minimum NOP delay for proper operation seems to be 150 ***
	HAL_SPI_Receive(Encoders_SPIConfig, (uint8_t*)rgENC_Values, 14, 100);
	pbarCS(1);
}

void Encoders_readSwitches(void)
{ 
	volatile uint16_t x  = NOP_CNT;
	
	pbarCS(0);
	while(--x){__NOP();}
	// *** The minimum NOP delay for proper operation seems to be 150 ***
	HAL_SPI_Receive(Encoders_SPIConfig, (uint8_t*)rgENC_Values, 2, 100);
	pbarCS(1);
}

int16_t* Encoders_get()
{
  return rgENC_Values;
}

//_____ Initialisaion _________________________________________________________________________________________________
void Encoders_init (SPI_HandleTypeDef *spiconfig)
{
	Encoders_SPIConfig = spiconfig;
	pbarRST(1);
}

