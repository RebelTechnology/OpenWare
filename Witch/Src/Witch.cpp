#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "Owl.h"
#include "errorhandlers.h"
#include "message.h"
#include "ProgramManager.h"
#include "PatchRegistry.h"
#include "OpenWareMidiControl.h"

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif

void pinChanged(uint16_t pin){
  switch(pin){
  case SW2_Pin:
    {
      bool state = HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_RESET;
      setButtonValue(BUTTON_B, state);
      break;
    }
  case SW3_Pin:
    {
      bool state = HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin) == GPIO_PIN_RESET;
      setButtonValue(BUTTON_C, state);
      break;
    }
  case SW4_Pin:
    {
      bool state = HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == GPIO_PIN_RESET;
      setButtonValue(BUTTON_D, state);
      break;
    }
  }
}

void setAnalogValue(uint8_t ch, int16_t value){
  extern DAC_HandleTypeDef hdac;
  // todo set LEDs
  switch(ch){
  case PARAMETER_F:
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, __USAT(value, 12));
    break;
  case PARAMETER_G:
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, __USAT(value, 12));
    break;
  }
}

void setGateValue(uint8_t ch, int16_t value){
  switch(ch){
  case PUSHBUTTON:
  case BUTTON_E:
    HAL_GPIO_WritePin(TR_OUT1_GPIO_Port, TR_OUT1_Pin, value ? GPIO_PIN_RESET :  GPIO_PIN_SET);
    break;
  case BUTTON_F:
    HAL_GPIO_WritePin(TR_OUT2_GPIO_Port, TR_OUT2_Pin, value ? GPIO_PIN_RESET :  GPIO_PIN_SET);
    break;
  }    
}

void initLed(){
  extern TIM_HandleTypeDef htim1;
  extern TIM_HandleTypeDef htim3;
  extern TIM_HandleTypeDef htim4;
  extern TIM_HandleTypeDef htim13;
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_Base_Start(&htim4);
  HAL_TIM_Base_Start(&htim13);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim13, TIM_CHANNEL_1);
}

void setLed(uint8_t led, uint32_t rgb){
  // uint32_t value = 1023 - ((rgb>>20)&0x3ff); // red
  uint32_t value = 1023 - (__USAT(rgb>>2, 10)); // expects 12-bit parameter value
  switch(led){
  case 0:
    break;
  case 1:
    TIM3->CCR1 = value;
    break;
  case 2:
    TIM3->CCR3 = value;
    break;
  case 3:
    TIM4->CCR3 = value;
    break;
  case 4:
    TIM13->CCR1 = value;
    break;
  case 5:
    TIM3->CCR4 = value;
    break;
  case 6:
    TIM1->CCR1 = value;
    break;
  }
}

void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len){
  // IIR exponential filter with lambda 0.75
  parameter_values[0] = (parameter_values[0]*3 + adc_values[ADC_A] + adc_values[ADC_B])>>2;
  parameter_values[1] = (parameter_values[1]*3 + adc_values[ADC_C] + adc_values[ADC_D])>>2;
  parameter_values[2] = (parameter_values[2]*3 + adc_values[ADC_E] + adc_values[ADC_F])>>2;
  parameter_values[3] = (parameter_values[3]*3 + adc_values[ADC_G] + adc_values[ADC_H])>>2;
  parameter_values[4] = (parameter_values[4]*3 + adc_values[ADC_I])>>2;  
}

bool isModeButtonPressed(){
  return HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin) == GPIO_PIN_RESET;
}


#define PATCH_RESET_COUNTER 80
static uint32_t counter = 0;
static void update_preset(){
  switch(getOperationMode()){
  case STARTUP_MODE:
    setOperationMode(RUN_MODE);
    break;
  case STREAM_MODE:
  case LOAD_MODE:
    setLed(1, counter > PATCH_RESET_COUNTER*0.1 ? 4095 : 0);
    setLed(2, counter > PATCH_RESET_COUNTER*0.2 ? 4095 : 0);
    setLed(5, counter > PATCH_RESET_COUNTER*0.3 ? 4095 : 0);
    setLed(6, counter > PATCH_RESET_COUNTER*0.4 ? 4095 : 0);
    setLed(3, counter > PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(4, counter > PATCH_RESET_COUNTER*0.6 ? 4095 : 0);
    break;
  case RUN_MODE:
    if(isModeButtonPressed()){
      setOperationMode(CONFIGURE_MODE);
    }else if(getErrorStatus() != NO_ERROR){
      setOperationMode(ERROR_MODE);
    }else{
      setLed(1, getParameterValue(PARAMETER_A));
      setLed(2, getParameterValue(PARAMETER_B));
      setLed(3, getParameterValue(PARAMETER_C));
      setLed(4, getParameterValue(PARAMETER_D));
      setLed(5, getParameterValue(PARAMETER_F));
      setLed(6, getParameterValue(PARAMETER_G));
    }
  case CONFIGURE_MODE:
    if(isModeButtonPressed()){
      uint8_t patchselect = program.getProgramIndex();
      if(getButtonValue(BUTTON_A)){
	patchselect = 1;
      }else if(getButtonValue(BUTTON_B)){
	patchselect = 2;
      }else if(getButtonValue(BUTTON_C)){
	patchselect = 3;
      }else if(getButtonValue(BUTTON_D)){
	patchselect = 4;
      }
      if(patchselect >= registry.getNumberOfPatches())
	patchselect = program.getProgramIndex();
      for(size_t i=1; i<5; ++i)
	setLed(i, i == patchselect ? 4095 : 0);
      if(program.getProgramIndex() != patchselect){
      	program.loadProgram(patchselect);
	program.resetProgram(false);
      }
    }else{
      setOperationMode(RUN_MODE);
    }
    break;
  case ERROR_MODE:
    setLed(1, counter > PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(2, counter > PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(5, counter < PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(6, counter < PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(3, counter > PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(4, counter > PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    if(isModeButtonPressed())
      program.resetProgram(false); // runAudioTask() changes to RUN_MODE
    break;
  }
  if(++counter >= PATCH_RESET_COUNTER)
    counter = 0;
}

void setup(){
  HAL_GPIO_WritePin(TR_OUT1_GPIO_Port, TR_OUT1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(TR_OUT2_GPIO_Port, TR_OUT2_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LEDPWM1_GPIO_Port, LEDPWM1_Pin, GPIO_PIN_SET);
  owl_setup();
}

void loop(void){
  MX_USB_HOST_Process(); // todo: enable PWR management
  bool state = HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET;
  if(state != getButtonValue(BUTTON_A)){
      setButtonValue(PUSHBUTTON, state);
      setButtonValue(BUTTON_A, state);
  }
  // state = HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin) == GPIO_PIN_RESET;
  // if(state != getButtonValue(BUTTON_E))
  //   setButtonValue(BUTTON_E, state); // todo: mode button
  update_preset();
  owl_loop();
}
