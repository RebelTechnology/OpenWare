#include "Owl.h"
#include "OpenWareMidiControl.h"
#include "device.h"
#include "errorhandlers.h"

extern uint32_t ledstatus;
void owl_mode_button(void);

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
  bool value;
  switch(pin){
  case SW1_Pin:
    value = !(SW1_GPIO_Port->IDR & SW1_Pin);
    setButtonValue(BUTTON_A, value);
    setButtonValue(PUSHBUTTON, value);
    ledstatus = value ? ledstatus |= 0x000003ff : ledstatus &= ~0x000003ff;
    break;
  case SW2_Pin:
    value = !(SW2_GPIO_Port->IDR & SW2_Pin);
    setButtonValue(BUTTON_B, value);
    ledstatus = value ? ledstatus |= 0x000ffc00 : ledstatus &= ~0x000ffc00;
    break;
  case SW3_Pin:
    value = !(SW3_GPIO_Port->IDR & SW3_Pin);
    setButtonValue(BUTTON_C, value);
    ledstatus = value ? ledstatus |= 0x3ff00000 : ledstatus &= ~0x3ff00000;
    break;
  }
}

void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len){
#ifdef OWL_EUROWIZARD
  parameter_values[0] = (parameter_values[0]*3 + 4095-adc_values[ADC_A])>>2;
  parameter_values[1] = (parameter_values[1]*3 + 4095-adc_values[ADC_B])>>2;
#else
  parameter_values[0] = (parameter_values[0]*3 + adc_values[ADC_A])>>2;
  parameter_values[1] = (parameter_values[1]*3 + adc_values[ADC_B])>>2;
#endif
  parameter_values[2] = (parameter_values[2]*3 + adc_values[ADC_C])>>2;
  parameter_values[3] = (parameter_values[3]*3 + adc_values[ADC_D])>>2;
  parameter_values[4] = (parameter_values[4]*3 + adc_values[ADC_E])>>2;
  // poll buttons SW4 and SW5
  bool value = !(SW4_GPIO_Port->IDR & SW4_Pin);
  if(getButtonValue(BUTTON_D) != value){
    setButtonValue(BUTTON_D, value);
    ledstatus = value ? ledstatus |= 0x3ff003ff : ledstatus &= ~0x3ff003ff;
    // ledstatus ^= 0x3ff003ff;
  }
  // mode button is read in owl_mode_button()
  // if(getButtonValue(BUTTON_E) != !(SW5_GPIO_Port->IDR & SW5_Pin)){
  //   setButtonValue(BUTTON_E, !(SW5_GPIO_Port->IDR & SW5_Pin));
  //   extern uint32_t ledstatus;
  //   ledstatus = 0;
  // }
}


void onSetup(){
  HAL_GPIO_WritePin(TRIG_OUT_GPIO_Port, TRIG_OUT_Pin, GPIO_PIN_RESET); // Trigger out off
  initLed();
  setLed(0, NO_COLOUR);

#ifdef USE_USB_HOST
  // enable USB Host power
  HAL_GPIO_WritePin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin, GPIO_PIN_SET);
#endif
}

void onLoop(){
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
  owl_mode_button();
}
