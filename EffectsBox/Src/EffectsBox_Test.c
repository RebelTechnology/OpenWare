#include <stdint.h>
#include "EffectsBox_Test.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "HAL_OLED.h"
#include "Flash_S25FL.h"

#define LED_RED	0
#define LED_YEL	1

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
uint8_t TSW1_State,TSW2_State;

void setLED(uint8_t led, LEDcolour col)
{
	GPIO_InitTypeDef 	GPIO_InitStruct;
	GPIO_TypeDef* 		GPIOx; 
	uint16_t 					GPIO_Pin;
	uint8_t 					LED_Colour;
	
	// Get switch pin and port number
	switch(led){
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
	
	// Set Output direction and LED colour
	switch (col){
		case YELLOW:	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;	LED_Colour = LED_YEL; break;
		case RED: 		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; LED_Colour = LED_RED; break;
  	        case NONE: 		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;	LED_Colour = 0;													break;
	}
	
	// Update Pin	
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOx,  GPIO_Pin,  (GPIO_PinState)LED_Colour);
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
		if (SW1_Read()) {setLED(1, RED);}
		else						{setLED(1, YELLOW);}
		
		if (SW2_Read()) {setLED(2, RED);}
		else						{setLED(2, YELLOW);}
		
		if (SW3_Read()) {setLED(3, RED);}
		else						{setLED(3, YELLOW);}
		
		if (SW4_Read()) {setLED(4, RED);}
		else						{setLED(4, YELLOW);}
		
		if (SW5_Read()) {setLED(5, RED);}
		else						{setLED(5, YELLOW);}
		
		if (SW6_Read()) {setLED(6, RED);}
		else						{setLED(6, YELLOW);}
		
		if (SW7_Read()) {setLED(7, RED);}
		else						{setLED(7, YELLOW);}
	
	}
	else
	{
		setLED(1, NONE);
		setLED(2, NONE);
		setLED(3, NONE);
		setLED(4, NONE);
		setLED(5, NONE);
		setLED(6, NONE);
		setLED(7, NONE);
	}

	TSW2_State = TSW2_Read();

	OLED_Refresh();
}
