#ifndef __HAL_TLC5971_H
#define __HAL_TLC5971_H

#ifdef __cplusplus
	extern "C" {
#endif

#include "stm32f4xx_hal.h"

// LED Mapping
#define LED_A				1
#define LED_B				0
#define LED_C				3
#define LED_D				2
		
#define LED_Red			0
#define LED_Green		1
#define LED_Blue		2
 
#define REG_BLANK		1<<0
#define REG_DSRPT		1<<1
#define REG_TMGRST	1<<2
#define REG_EXTGCK	1<<3
#define REG_OUTTMG	1<<4
		
// Prototypes
void TLC5971_init(SPI_HandleTypeDef *config);
void TLC5971_SetOutput_GS(unsigned char LED_ID, unsigned char LED_Colour,  unsigned short value);
void TLC5971_SetOutput_DC(unsigned char LED_ID, unsigned char LED_Colour,  unsigned char value);
void TLC5971_Update(void);

#ifdef __cplusplus
}
#endif
#endif
