#include "HAL_Encoders.h"
#include "device.h"
#include <string.h>

SPI_HandleTypeDef* Encoders_SPIConfig;

// Pin Control
#define pbarCS(state)		HAL_GPIO_WritePin(ENC_nCS_GPIO_Port,  ENC_nCS_Pin,  (GPIO_PinState)state)
#define pbarRST(state)		HAL_GPIO_WritePin(ENC_NRST_GPIO_Port,  ENC_NRST_Pin,  (GPIO_PinState)state)

static int16_t rgENC_Values[7] = {0};

#define ENCODER_CS_DELAY_US 8

__STATIC_INLINE void DWT_Delay_us(volatile uint32_t microseconds)
{
 uint32_t clk_cycle_start = DWT->CYCCNT;
 /* Go to number of cycles for system */
 microseconds *= (HAL_RCC_GetHCLKFreq() / 1000000);
 /* Delay till end */
 while ((DWT->CYCCNT - clk_cycle_start) < microseconds);
}

//_____ Functions _____________________________________________________________________________________________________
// Port and Chip Setup
void Encoders_readAll(void)
{ 
	pbarCS(0);
	DWT_Delay_us(ENCODER_CS_DELAY_US);
	HAL_SPI_Receive(Encoders_SPIConfig, (uint8_t*)rgENC_Values, 14, 100);
	pbarCS(1);
}

void Encoders_readSwitches(void)
{ 
	pbarCS(0);
	DWT_Delay_us(ENCODER_CS_DELAY_US);
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

void Encoders_reset(void){
	pbarRST(0);
	HAL_Delay(20);
	pbarRST(1);	
}

