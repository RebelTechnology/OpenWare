#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "Owl.h"
#include "usb_device.h"

#include "ads.h"
#ifdef USE_KX122
#include "kx122.h"
#endif
#include "ble_midi.h"

void onSetup(){
  ble_init();
  setLed(1, NO_COLOUR);
  MX_USB_DEVICE_Init();
}

void initLed(){
  // Initialise RGB LED PWM timers
#ifdef USE_LED_PWM
  extern TIM_HandleTypeDef htim1;
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
#else
  /*Configure GPIO pin : LED_GREEN_Pin, LED_RED_Pin */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = LED_GREEN_Pin | LED_RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GREEN_GPIO_Port, &GPIO_InitStruct);
#endif
}

void setLed(uint8_t led, uint32_t rgb){
  // rgb should be a 3x 10 bit value
  if(led == 0){
#ifdef USE_LED_PWM
    rgb &= COLOUR_LEVEL5; // turn down intensity
    TIM1->CCR1 = 1023 - ((rgb>>20)&0x3ff); // red
    TIM1->CCR3 = 1023 - ((rgb>>10)&0x3ff); // green
    TIM1->CCR2 = 1023 - ((rgb>>00)&0x3ff); // blue
#else
    switch(rgb){ // sinking current
    case RED_COLOUR:
      HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
      break;
    case GREEN_COLOUR:
      HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
      break;
    case YELLOW_COLOUR:
      HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
      break;
    case NO_COLOUR:
      HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
      break;
    }
#endif      
  }else if(led == 1){
    if(rgb == NO_COLOUR)
      HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
    else
      HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
  }
}

void onChangePin(uint16_t pin){
  switch(pin){
  case ADC_DRDY_Pin: {
    ads_drdy();
    break;
  }
#ifdef USE_KX122
  case ACC_INT1_Pin: {
    kx122_drdy();
    break;
  }
#endif
#ifdef PUSHBUTTON_Pin
  case PUSHBUTTON_Pin: {
    bool isSet = !(PUSHBUTTON_GPIO_Port->IDR & PUSHBUTTON_Pin);
    setButtonValue(PUSHBUTTON, isSet);
    midi_tx.sendCc(PUSHBUTTON, isSet ? 127 : 0);
    break;
  }
#endif
  }
}
