#include <stdint.h>
#include "EffectsBox_Test.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "HAL_OLED.h"
#include "Flash_S25FL.h"

#define LED_RED	0
#define LED_GRN	1

#define SW1_LED(state)	HAL_GPIO_WritePin(SW1_LED_GPIO_Port,  SW1_LED_Pin,  (GPIO_PinState)state)
#define SW2_LED(state)	HAL_GPIO_WritePin(SW2_LED_GPIO_Port,  SW2_LED_Pin,  (GPIO_PinState)state)
#define SW3_LED(state)	HAL_GPIO_WritePin(SW3_LED_GPIO_Port,  SW3_LED_Pin,  (GPIO_PinState)state)
#define SW4_LED(state)	HAL_GPIO_WritePin(SW4_LED_GPIO_Port,  SW4_LED_Pin,  (GPIO_PinState)state)
#define SW5_LED(state)	HAL_GPIO_WritePin(SW5_LED_GPIO_Port,  SW5_LED_Pin,  (GPIO_PinState)state)
#define SW6_LED(state)	HAL_GPIO_WritePin(SW6_LED_GPIO_Port,  SW6_LED_Pin,  (GPIO_PinState)state)
#define SW7_LED(state)	HAL_GPIO_WritePin(SW7_LED_GPIO_Port,  SW7_LED_Pin,  (GPIO_PinState)state)

#define SW1_Read()		1-HAL_GPIO_ReadPin(SW1_BTN_GPIO_Port,  SW1_BTN_Pin)
#define SW2_Read()		1-HAL_GPIO_ReadPin(SW2_BTN_GPIO_Port,  SW2_BTN_Pin)
#define SW3_Read()		1-HAL_GPIO_ReadPin(SW3_BTN_GPIO_Port,  SW3_BTN_Pin)
#define SW4_Read()		1-HAL_GPIO_ReadPin(SW4_BTN_GPIO_Port,  SW4_BTN_Pin)
#define SW5_Read()		1-HAL_GPIO_ReadPin(SW5_BTN_GPIO_Port,  SW5_BTN_Pin)
#define SW6_Read()		1-HAL_GPIO_ReadPin(SW6_BTN_GPIO_Port,  SW6_BTN_Pin)
#define SW7_Read()		1-HAL_GPIO_ReadPin(SW7_BTN_GPIO_Port,  SW7_BTN_Pin)

#define TSW1_Read()		(1-HAL_GPIO_ReadPin(TSW1_A_GPIO_Port,  TSW1_A_Pin)) | (1-HAL_GPIO_ReadPin(TSW1_B_GPIO_Port,  TSW1_B_Pin))<<1
#define TSW2_Read()		(1-HAL_GPIO_ReadPin(TSW2_A_GPIO_Port,  TSW2_A_Pin)) | (1-HAL_GPIO_ReadPin(TSW2_B_GPIO_Port,  TSW2_B_Pin))<<1

uint8_t TSW1_State;
uint8_t TSW2_State;

void EffectsBox_Init(void)
{
	extern TIM_HandleTypeDef htim11;
  HAL_TIM_Base_Start(&htim11);
  HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1);
	
	extern TIM_HandleTypeDef htim1;
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	
	
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
	SW1_LED(TSW1_Read());
	SW2_LED(TSW1_Read());
	SW3_LED(TSW1_Read());
	SW4_LED(TSW2_Read());
	SW5_LED(TSW2_Read());
	SW6_LED(TSW2_Read());
	
	SW7_LED(SW7_Read());

	
	TSW1_State = TSW1_Read();
	TSW2_State = TSW2_Read();
	
	
	OLED_Refresh();
	
	
}
