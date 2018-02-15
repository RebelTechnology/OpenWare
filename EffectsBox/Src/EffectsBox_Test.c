#include <stdint.h>
#include "EffectsBox_Test.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "HAL_OLED.h"
#include "Flash_S25FL.h"

#define LED_RED	0
#define LED_GRN	1

#define SW1_LED(colour)	HAL_GPIO_WritePin(SW1_LED_GPIO_Port,  SW1_LED_Pin,  (GPIO_PinState)colour)
#define SW2_LED(colour)	HAL_GPIO_WritePin(SW2_LED_GPIO_Port,  SW2_LED_Pin,  (GPIO_PinState)colour)
#define SW3_LED(colour)	HAL_GPIO_WritePin(SW3_LED_GPIO_Port,  SW3_LED_Pin,  (GPIO_PinState)colour)
#define SW4_LED(colour)	HAL_GPIO_WritePin(SW4_LED_GPIO_Port,  SW4_LED_Pin,  (GPIO_PinState)colour)
#define SW5_LED(colour)	HAL_GPIO_WritePin(SW5_LED_GPIO_Port,  SW5_LED_Pin,  (GPIO_PinState)colour)
#define SW6_LED(colour)	HAL_GPIO_WritePin(SW6_LED_GPIO_Port,  SW6_LED_Pin,  (GPIO_PinState)colour)
#define SW7_LED(colour)	HAL_GPIO_WritePin(SW7_LED_GPIO_Port,  SW7_LED_Pin,  (GPIO_PinState)colour)

#define SW1_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW1_BTN_GPIO_Port,  SW1_BTN_Pin))
#define SW2_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW2_BTN_GPIO_Port,  SW2_BTN_Pin))
#define SW3_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW3_BTN_GPIO_Port,  SW3_BTN_Pin))
#define SW4_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW4_BTN_GPIO_Port,  SW4_BTN_Pin))
#define SW5_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW5_BTN_GPIO_Port,  SW5_BTN_Pin))
#define SW6_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW6_BTN_GPIO_Port,  SW6_BTN_Pin))
#define SW7_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW7_BTN_GPIO_Port,  SW7_BTN_Pin))

#define TSW1_Read()		(1-HAL_GPIO_ReadPin(TSW1_A_GPIO_Port,  TSW1_A_Pin)) | (1-HAL_GPIO_ReadPin(TSW1_B_GPIO_Port,  TSW1_B_Pin))<<1
#define TSW2_Read()		(1-HAL_GPIO_ReadPin(TSW2_A_GPIO_Port,  TSW2_A_Pin)) | (1-HAL_GPIO_ReadPin(TSW2_B_GPIO_Port,  TSW2_B_Pin))<<1

extern SPI_HandleTypeDef hspi5;
	
uint8_t TSW1_State;
uint8_t TSW2_State;

enum {
	YELLOW, RED, NONE
} LedColour;

void setLed(uint8_t led, LedColour colour)
{

}

void SW_LED_EN(uint8_t sw, uint8_t state)
{
	GPIO_InitTypeDef 	GPIO_InitStruct;
	GPIO_TypeDef* 		GPIOx; 
	uint16_t 					GPIO_Pin;
	
	// Get switch pin and port number
	switch(sw)
	{
		case 1: GPIOx = SW1_LED_GPIO_Port;	GPIO_Pin = SW1_LED_Pin; break;
		case 2: GPIOx = SW2_LED_GPIO_Port;	GPIO_Pin = SW2_LED_Pin; break;
		case 3: GPIOx = SW3_LED_GPIO_Port;	GPIO_Pin = SW3_LED_Pin; break;
		case 4: GPIOx = SW4_LED_GPIO_Port;	GPIO_Pin = SW4_LED_Pin; break;
		case 5: GPIOx = SW5_LED_GPIO_Port;	GPIO_Pin = SW5_LED_Pin; break;
		case 6: GPIOx = SW6_LED_GPIO_Port;	GPIO_Pin = SW6_LED_Pin; break;
		case 7: GPIOx = SW7_LED_GPIO_Port;	GPIO_Pin = SW7_LED_Pin; break;
	}
	
	// Set pin number and direction
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	if (state){GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;}	// Enable
	else			{GPIO_InitStruct.Mode = GPIO_MODE_INPUT;}			// Disable

	// Update Pin
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void EffectsBox_Init(void)
{
	extern TIM_HandleTypeDef htim11;
  HAL_TIM_Base_Start(&htim11);
  HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1);
	
	extern TIM_HandleTypeDef htim1;
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	
	OLED_init(&hspi5);
	
	OLED_setPixel(10,10);
	OLED_setPixel(10,11);
	OLED_setPixel(10,12);
	OLED_setPixel(10,13);
	OLED_setPixel(10,14);
	OLED_setPixel(10,15);
	OLED_setPixel(10,16);
	OLED_setPixel(10,17);
}


void EffectsBox_Main(void)
{

	if(TSW1_Read())
	{
		SW_LED_EN(1, !SW1_Read());
		SW_LED_EN(2, !SW2_Read());
		SW_LED_EN(3, !SW3_Read());
		SW_LED_EN(4, !SW4_Read());
		SW_LED_EN(5, !SW5_Read());
		SW_LED_EN(6, !SW6_Read());
		SW_LED_EN(7, !SW7_Read());
	}
	
	SW1_LED(SW1_Read());
	SW2_LED(SW2_Read());
	SW3_LED(SW3_Read());
	SW4_LED(SW4_Read());
	SW5_LED(SW5_Read());
	SW6_LED(SW6_Read());
	SW7_LED(SW7_Read());

//	TSW2_State = TSW2_Read();

	OLED_Refresh();
}
