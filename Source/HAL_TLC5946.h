#ifndef __HAL_TLC5946_H
#define __HAL_TLC5946_H

#ifdef __cplusplus
	extern "C" {
#endif

#include "stm32f4xx_hal.h"

// Modes
#define Mode_GS 	0
#define Mode_DC		1

// Pin Control
#define pXLAT(state)		HAL_GPIO_WritePin(TLC_XLAT_GPIO_Port,  TLC_XLAT_Pin,  (GPIO_PinState)state)
#define pMODE(state)		HAL_GPIO_WritePin(TLC_MODE_GPIO_Port,  TLC_MODE_Pin,  (GPIO_PinState)state)
#define pBLANK(state)		HAL_GPIO_WritePin(TLC_BLANK_GPIO_Port, TLC_BLANK_Pin, (GPIO_PinState)state)
 
// Prototypes
void TLC5946_init (SPI_HandleTypeDef *config);
void TLC5946_SetOutput_GS(unsigned char IC, unsigned char LED_ID, unsigned short value);
void TLC5946_SetOutput_DC(unsigned char IC, unsigned char LED_ID, unsigned char value);
void TLC5946_Refresh_GS(void);
void TLC5946_Refresh_DC(void);
void TLC5946_TxINTCallback(void);

void Magus_setRGB(unsigned char LED_ID, unsigned short val_R, unsigned short val_G, unsigned short val_B);
void Magus_setRGB_DC(unsigned short val_R, unsigned short val_G, unsigned short val_B);

#ifdef __cplusplus
}
#endif
#endif
