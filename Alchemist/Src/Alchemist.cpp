#include "Owl.h"
#include "OpenWareMidiControl.h"
#include "device.h"
#include "errorhandlers.h"

extern uint32_t ledstatus;

void initLed(){
  // Initialise RGB LED PWM timers
  extern TIM_HandleTypeDef htim2;
  extern TIM_HandleTypeDef htim3;
  // Red
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  // Green
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  // Blue
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
}

void setLed(uint8_t led, uint32_t rgb){
  // rgb should be a 3x 10 bit value
  TIM2->CCR1 = 1023 - ((rgb>>20)&0x3ff);
  TIM2->CCR2 = 1023 - ((rgb>>10)&0x3ff);
  TIM3->CCR4 = 1023 - ((rgb>>00)&0x3ff);
}

void onChangePin(uint16_t pin){
  switch(pin){
  case SW1_Pin:
    setButtonValue(BUTTON_A, !(SW1_GPIO_Port->IDR & SW1_Pin));
    setButtonValue(PUSHBUTTON, !(SW1_GPIO_Port->IDR & SW1_Pin));
    ledstatus ^= 0x000003ff;
    break;
  case SW2_Pin:
    setButtonValue(BUTTON_B, !(SW2_GPIO_Port->IDR & SW2_Pin));
    // setParameterValue(PARAMETER_E, (SW2_GPIO_Port->IDR & SW2_Pin) == 0 ? 4095 : 0);
    ledstatus ^= 0x000ffc00; // getButtonValue(BUTTON_B) ? 0x000ffc00 : 0;
    break;
  case SW3_Pin:
    setButtonValue(BUTTON_C, !(SW3_GPIO_Port->IDR & SW3_Pin));
    ledstatus ^= 0x3ff00000; // getButtonValue(BUTTON_C) ? 0x3ff00000 : 0;
    break;
  }
}
