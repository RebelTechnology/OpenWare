#include "stm32f4xx_hal.h"
#include <string.h>
#include "TLC5971.h"

uint8_t rgTxBuffer[28];

SPI_HandleTypeDef* TLC5971_SPIConfig;
	
//_____ Functions _____________________________________________________________________________________________________
void TLC5971_SetOutput_GS (unsigned char LED_ID, unsigned char LED_Colour,  unsigned short value)
{
	unsigned char ucBuffLoc = LED_ID*6;
	
	switch (LED_Colour)
	{
		case LED_Red:
			rgTxBuffer[ucBuffLoc+0] = (value&0x00FF)>>0;
			rgTxBuffer[ucBuffLoc+1] = (value&0xFF00)>>8;
			break;
		
		case LED_Green:
			rgTxBuffer[ucBuffLoc+2] = (value&0x00FF)>>0;
			rgTxBuffer[ucBuffLoc+3] = (value&0xFF00)>>8;
			break;
		
		case LED_Blue:
			rgTxBuffer[ucBuffLoc+4] = (value&0x00FF)>>0;
			rgTxBuffer[ucBuffLoc+5] = (value&0xFF00)>>8;
			break;
	}
}	

void TLC5971_SetOutput_DC (unsigned char LED_ID, unsigned char LED_Colour,  unsigned char value)
{
	unsigned char ucBuffTemp;
		
	if (value < 128)
	{
		switch (LED_Colour)
		{
			case LED_Red:
				ucBuffTemp 		 	= rgTxBuffer[24];		
				rgTxBuffer[24]  = (value&0x7F);
				rgTxBuffer[24] |= (ucBuffTemp&0x80);
				break;
			
			case LED_Green:
				ucBuffTemp 		 	= rgTxBuffer[24];	
				rgTxBuffer[24]  = (value<<7);
				rgTxBuffer[24] |= (ucBuffTemp&0x7F);
			
				ucBuffTemp 		 	= rgTxBuffer[25];
				rgTxBuffer[25]  = (value>>1);
				rgTxBuffer[25] |= (ucBuffTemp&0xC0);
				break;
			
			case LED_Blue:
				ucBuffTemp 		 	= rgTxBuffer[25];
				rgTxBuffer[25]  = (value<<6);
				rgTxBuffer[25] |= (ucBuffTemp&0x3F);
			
				ucBuffTemp 		 	= rgTxBuffer[26];
				rgTxBuffer[26]  = (value>>2);
				rgTxBuffer[26] |= (ucBuffTemp&0xE0);
				break;
		}
	}		
}

void TLC5971_Settings (unsigned char value)
{
	unsigned char ucBuffTemp = rgTxBuffer[26];
	
	rgTxBuffer[26]  = (value<<5);
	rgTxBuffer[26] |= (ucBuffTemp&0x1F);
	rgTxBuffer[27]	= (value>>3);
}
void TLC5971_Update(void)
{
	HAL_SPI_Transmit(TLC5971_SPIConfig, rgTxBuffer, sizeof rgTxBuffer, 100);
}

//_____ Initialisaion _________________________________________________________________________________________________
void TLC5971_init(SPI_HandleTypeDef *spiconfig)
{
	TLC5971_SPIConfig = spiconfig;
	
	memset(rgTxBuffer, 0, sizeof rgTxBuffer);
}

