#include "stm32f7xx_hal.h"

// _____ Defines _______________________________________________________________________
#define pRST_Set()	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET)
#define pDC_Set()		HAL_GPIO_WritePin(OLED_DC_GPIO_Port, 	OLED_DC_Pin, 	GPIO_PIN_SET)
#define pCS_Set()		HAL_GPIO_WritePin(OLED_CS_GPIO_Port, 	OLED_CS_Pin, 	GPIO_PIN_SET)

#define pRST_Clr()	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET)
#define pDC_Clr()		HAL_GPIO_WritePin(OLED_DC_GPIO_Port, 	OLED_DC_Pin, 	GPIO_PIN_RESET)
#define pCS_Clr()		HAL_GPIO_WritePin(OLED_CS_GPIO_Port, 	OLED_CS_Pin, 	GPIO_PIN_RESET)

#define OLED_DAT	1
#define OLED_CMD	0

// _____ Prototypes ____________________________________________________________________
void OLED_writeCMD(const uint8_t* data, uint16_t length);
void OLED_writeDAT(const uint8_t* data, uint16_t length);
uint8_t OLED_getPixel(uint8_t x, uint8_t y);
void OLED_setPixel(uint8_t x, uint8_t y);
void OLED_clearPixel(uint8_t x, uint8_t y);
void OLED_togglePixel(uint8_t x, uint8_t y);
void OLED_ClearScreen(void);
void OLED_Config(SPI_HandleTypeDef* spi, uint8_t* pixel);
void NopDelay(uint32_t nops);
