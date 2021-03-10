#include "Owl.h"
#include "OpenWareMidiControl.h"
#include "device.h"
#include "errorhandlers.h"

extern uint32_t ledstatus;

void setGateValue(uint8_t ch, int16_t value){
  if(ch == BUTTON_F || ch == PUSHBUTTON)
    HAL_GPIO_WritePin(TRIG_OUT_GPIO_Port, TRIG_OUT_Pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void initLed(){
  extern TIM_HandleTypeDef htim2;
  extern TIM_HandleTypeDef htim5;
  extern TIM_HandleTypeDef htim4;
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_Base_Start(&htim5);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
  HAL_TIM_Base_Start(&htim4);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
}

void setLed(uint8_t led, uint32_t rgb){
  // rgb should be a 3x 10 bit value
  TIM5->CCR2 = 1023 - ((rgb>>20)&0x3ff);
  TIM4->CCR3 = 1023 - ((rgb>>10)&0x3ff);
  TIM2->CCR1 = 1023 - ((rgb>>00)&0x3ff);
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
// #ifdef OWL_WIZARD // done in ProgramManager::onProgramReady()
//   case SW4_Pin:
//     setButtonValue(BUTTON_D, !(SW4_GPIO_Port->IDR & SW4_Pin));
//     ledstatus ^= 0x3ff003ff;
//     break;
//   case SW5_Pin:
//     setButtonValue(BUTTON_E, !(SW5_GPIO_Port->IDR & SW5_Pin));
//     ledstatus = 0;
//     break;
// #endif
  }
}

void setup(){
  HAL_GPIO_WritePin(TRIG_OUT_GPIO_Port, TRIG_OUT_Pin, GPIO_PIN_RESET); // Trigger out off
  owl.setup();

#ifdef USE_USB_HOST
  // enable USB Host power
  HAL_GPIO_WritePin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin, GPIO_PIN_SET);
#endif
}


void loop(void){
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

  owl.loop();
}
