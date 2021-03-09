#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "Owl.h"
#include "errorhandlers.h"
#include "ProgramManager.h"

#ifdef USE_SCREEN
#include "Graphics.h"
Graphics graphics;
#endif /* USE_SCREEN */

#ifdef USE_ENCODERS
extern TIM_HandleTypeDef ENCODER_TIM1;
extern TIM_HandleTypeDef ENCODER_TIM2;
#endif

#ifdef OWL_EFFECTSBOX
// static uint8_t buttonstate = 0;
// #define SW1_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW1_BTN_GPIO_Port,  SW1_BTN_Pin))
// #define SW2_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW2_BTN_GPIO_Port,  SW2_BTN_Pin))
// #define SW3_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW3_BTN_GPIO_Port,  SW3_BTN_Pin))
// #define SW4_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW4_BTN_GPIO_Port,  SW4_BTN_Pin))
// #define SW5_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW5_BTN_GPIO_Port,  SW5_BTN_Pin))
// #define SW6_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW6_BTN_GPIO_Port,  SW6_BTN_Pin))
// #define SW7_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW7_BTN_GPIO_Port,  SW7_BTN_Pin))
// #define TSW1_Read()		(1-HAL_GPIO_ReadPin(TSW1_A_GPIO_Port,  TSW1_A_Pin)) | (1-HAL_GPIO_ReadPin(TSW1_B_GPIO_Port,  TSW1_B_Pin))<<1
// #define TSW2_Read()		(1-HAL_GPIO_ReadPin(TSW2_A_GPIO_Port,  TSW2_A_Pin)) | (1-HAL_GPIO_ReadPin(TSW2_B_GPIO_Port,  TSW2_B_Pin))<<1
typedef enum {
	YELLOW, RED, NONE
} LEDcolour;
void setLED(uint8_t led, LEDcolour col){
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_TypeDef* GPIOx; 
  uint16_t GPIO_Pin;
  uint8_t LED_Colour;
	
  // Get switch pin and port number
  switch(led){
  case 0: GPIOx = SW1_LED_GPIO_Port;	GPIO_Pin = SW1_LED_Pin; break;
  case 1: GPIOx = SW2_LED_GPIO_Port;	GPIO_Pin = SW2_LED_Pin; break;
  case 2: GPIOx = SW3_LED_GPIO_Port;	GPIO_Pin = SW3_LED_Pin; break;
  case 3: GPIOx = SW4_LED_GPIO_Port;	GPIO_Pin = SW4_LED_Pin; break;
  case 4: GPIOx = SW5_LED_GPIO_Port;	GPIO_Pin = SW5_LED_Pin; break;
  case 5: GPIOx = SW6_LED_GPIO_Port;	GPIO_Pin = SW6_LED_Pin; break;
  case 6: GPIOx = SW7_LED_GPIO_Port;	GPIO_Pin = SW7_LED_Pin; break;
  }
	
  // Set pin number and direction
  GPIO_InitStruct.Pin = GPIO_Pin;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	
  // Set Output direction and LED colour
  switch (col){
  case YELLOW:	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; LED_Colour = YELLOW; break;
  case RED: 	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; LED_Colour = RED; break;
  case NONE: 	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;	LED_Colour = 0; break;
  }
  // Update Pin	
  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOx,  GPIO_Pin,  (GPIO_PinState)LED_Colour);
}

void updateProgramSelector(uint8_t button, uint8_t led, uint8_t patch, bool value){  
  setButtonValue(button, value);
  if(value){
    setLED(led, RED);
  }else{
    if(program.getProgramIndex() != patch){
      program.loadProgram(patch);
      for(int i=0; i<6; ++i)
	setLED(i, NONE);
    }
    setLED(led, YELLOW);
  }
}
#endif /* OWL_EFFECTSBOX */


void onChangePin(uint16_t pin){
  switch(pin){
#ifdef OWL_EFFECTSBOX
  case SW1_BTN_Pin:
    updateProgramSelector(BUTTON_A, 0, 1, !(SW1_BTN_GPIO_Port->IDR & SW1_BTN_Pin));
    break;
  case SW2_BTN_Pin:
    updateProgramSelector(BUTTON_B, 1, 2, !(SW2_BTN_GPIO_Port->IDR & SW2_BTN_Pin));    
    break;
  case SW3_BTN_Pin:
    updateProgramSelector(BUTTON_C, 2, 3, !(SW3_BTN_GPIO_Port->IDR & SW3_BTN_Pin));    
    setLED(2, (LEDcolour)getButtonValue(BUTTON_C));
    break;
  case SW4_BTN_Pin:
    updateProgramSelector(BUTTON_D, 3, 4, !(SW4_BTN_GPIO_Port->IDR & SW4_BTN_Pin));    
    break;
  case SW5_BTN_Pin:
    updateProgramSelector(BUTTON_E, 4, 5, !(SW5_BTN_GPIO_Port->IDR & SW5_BTN_Pin));    
    break;
  case SW6_BTN_Pin:
    updateProgramSelector(BUTTON_F, 5, 6, !(SW6_BTN_GPIO_Port->IDR & SW6_BTN_Pin));    
    break;
  case SW7_BTN_Pin:
    if((SW7_BTN_GPIO_Port->IDR & SW7_BTN_Pin)){
      setButtonValue(PUSHBUTTON, false);
      setLED(6, YELLOW);
    }else{
      setButtonValue(PUSHBUTTON, true);
      setLED(6, RED);
    }
    break;
#endif /* OWL_EFFECTSBOX */
  }
}

void setup(){
#ifdef OWL_EFFECTSBOX
  extern TIM_HandleTypeDef htim11;
  HAL_TIM_Base_Start(&htim11);
  HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1); // SW1-6 PWM
  extern TIM_HandleTypeDef htim1;
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); // SW7 PWM
  for(int i=0; i<7; ++i)
    setLED(i, NONE);
#endif /* OWL_EFFECTSBOX */
#ifdef USE_ENCODERS
  __HAL_TIM_SET_COUNTER(&ENCODER_TIM1, INT16_MAX/2);
  __HAL_TIM_SET_COUNTER(&ENCODER_TIM2, INT16_MAX/2);
  HAL_TIM_Encoder_Start_IT(&ENCODER_TIM1, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start_IT(&ENCODER_TIM2, TIM_CHANNEL_ALL);
#endif /* OWL_PLAYERF7 */
  owl.setup();
}

void loop(void){
  owl.loop();

#ifdef OWL_EFFECTSBOX
  // uint8_t state =
  //   (SW1_Read() << 0) |
  //   (SW2_Read() << 1) |
  //   (SW3_Read() << 2) |
  //   (SW4_Read() << 3) |
  //   (SW5_Read() << 4) |
  //   (SW6_Read() << 5) |
  //   (SW7_Read() << 6);
  // if(state != buttonstate){
  //   for(int i=0; i<7; ++i){
  //   }
  // }
#endif /* OWL_EFFECTSBOX */
  
#ifdef USE_USB_HOST
  if(HAL_GPIO_ReadPin(USB_HOST_PWR_FAULT_GPIO_Port, USB_HOST_PWR_FAULT_Pin) == GPIO_PIN_RESET){
    if(HAL_GPIO_ReadPin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin) == GPIO_PIN_SET){
      HAL_GPIO_WritePin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin, GPIO_PIN_RESET);
      error(USB_ERROR, "USBH PWR Fault");
    }
  }else{
    MX_USB_HOST_Process();
  }
#endif

#ifdef USE_SCREEN
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); // OLED off
  extern SPI_HandleTypeDef OLED_SPI;
  graphics.begin(&OLED_SPI);
#endif /* USE_SCREEN */

#ifdef OWL_EFFECTSBOX
  int16_t encoders[NOF_ENCODERS] = {(int16_t)__HAL_TIM_GET_COUNTER(&ENCODER_TIM1),
  				    (int16_t)__HAL_TIM_GET_COUNTER(&ENCODER_TIM2) };
  graphics.params.updateEncoders(encoders, 6);
  for(int i=0; i<NOF_ADC_VALUES; ++i)
    graphics.params.updateValue(i, getAnalogValue(i));
  // for(int i=NOF_ADC_VALUES; i<NOF_PARAMETERS; ++i)
  //   graphics.params.updateValue(i, 0);
#endif  
}
