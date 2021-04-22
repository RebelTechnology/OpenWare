#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "Owl.h"
#include "errorhandlers.h"
#include "message.h"
#include "ProgramManager.h"
#include "PatchRegistry.h"
#include "OpenWareMidiControl.h"
#include "Codec.h"
#include "ApplicationSettings.h"
#include "qint.h"
#include "Pin.h"

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif

// LEDs
// 1, 2, 3, 4: CV level A, B, C, D
// 5, 6: DAC out F, G
// 7, 8, 9, 10: LED buttons

// Buttons
// SW1, SW2, SW3 : exti
// SW4, SW5/mode: poll

Pin ledpwm(LEDPWM_GPIO_Port, LEDPWM_Pin);
Pin led7(LEDSW1_GPIO_Port, LEDSW1_Pin);
Pin led8(LEDSW2_GPIO_Port, LEDSW2_Pin);
Pin led9(LEDSW3_GPIO_Port, LEDSW3_Pin);
Pin led10(LEDSW4_GPIO_Port, LEDSW4_Pin);

Pin sw1(SW1_GPIO_Port, SW1_Pin);
Pin sw2(SW2_GPIO_Port, SW2_Pin);
Pin sw3(SW3_GPIO_Port, SW3_Pin);
Pin sw4(SW4_GPIO_Port, SW4_Pin);
Pin sw5(SW5_GPIO_Port, SW5_Pin);

template<size_t SIZE, typename value_t>
class TakeoverControls {
private:
  value_t values[SIZE];
  bool takeover[SIZE];
public:
  TakeoverControls(){
    reset(true);
  }
  value_t get(uint8_t index){
    return values[index];
  }
  void set(uint8_t index, value_t value){
    values[index] = value;
  }
  void update(uint8_t index, value_t value, value_t threshold){
    if(takeover[index]){
      values[index] = value;
    }else if(abs(values[index] - value) < threshold){
      takeover[index] = true;
      values[index] = value;
    }
  }
  bool taken(uint8_t index){
    return takeover[index];
  }
  void reset(uint8_t index, bool state){
    takeover[index] = state;
  }
  void reset(bool state){
    for(size_t i=0; i<SIZE; ++i)
      takeover[i] = state;
  }
};

TakeoverControls<10, int16_t> takeover;
int16_t dac_values[2] = {0, 0};

void onChangePin(uint16_t pin){
  if(owl.getOperationMode() == RUN_MODE){
    switch(pin){
    case SW1_Pin:
      {
	bool state = HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET;
	setButtonValue(PUSHBUTTON, state);
	setButtonValue(BUTTON_A, state);
	setLed(7, state ? RED_COLOUR : NO_COLOUR);
	break;
      }
    case SW2_Pin:
      {
	bool state = HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_RESET;
	setButtonValue(BUTTON_B, state);
	setLed(8, state ? RED_COLOUR : NO_COLOUR);
	break;
      }
    case SW3_Pin:
      {
	bool state = HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin) == GPIO_PIN_RESET;
	setButtonValue(BUTTON_C, state);
	setLed(9, state ? RED_COLOUR : NO_COLOUR);
	break;
      }
    // case SW4_Pin:
    //   {
    // 	bool state = HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == GPIO_PIN_RESET;
    // 	setButtonValue(BUTTON_D, state);
    // 	break;
    //   }
    }
  }
}

void setAnalogValue(uint8_t ch, int16_t value){
  if(owl.getOperationMode() == RUN_MODE){
    extern DAC_HandleTypeDef hdac;
    value = __USAT(value, 12);
    switch(ch){
    case PARAMETER_F:
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, value);
      dac_values[0] = value;
      break;
    case PARAMETER_G:
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, value);
      dac_values[1] = value;
      break;
    }
  }
}

void setGateValue(uint8_t ch, int16_t value){
  if(owl.getOperationMode() == RUN_MODE){
    switch(ch){
    case BUTTON_A:
      setLed(7, value ? RED_COLOUR : NO_COLOUR);
      break;
    case BUTTON_B:
      setLed(8, value ? RED_COLOUR : NO_COLOUR);
      break;
    case BUTTON_C:
      setLed(9, value ? RED_COLOUR : NO_COLOUR);
      break;
    case BUTTON_D:
      setLed(10, value ? RED_COLOUR : NO_COLOUR);
      break;
    case PUSHBUTTON:
    case BUTTON_E:
      HAL_GPIO_WritePin(TR_OUT1_GPIO_Port, TR_OUT1_Pin, value ? GPIO_PIN_RESET :  GPIO_PIN_SET);
      break;
    case BUTTON_F:
      HAL_GPIO_WritePin(TR_OUT2_GPIO_Port, TR_OUT2_Pin, value ? GPIO_PIN_RESET :  GPIO_PIN_SET);
      break;
    }
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
  case 7:
    if(rgb == RED_COLOUR){
      ledpwm.high();
      led7.low();
    }else if(rgb == YELLOW_COLOUR){
      ledpwm.low();
      led7.high();
    }else{
      led7.set(ledpwm.get());
    }
    break;
  case 8:
    if(rgb == RED_COLOUR){
      ledpwm.high();
      led8.low();
    }else if(rgb == YELLOW_COLOUR){
      ledpwm.low();
      led8.high();
    }else{
      led8.set(ledpwm.get());
    }
    break;
  case 9:
    if(rgb == RED_COLOUR){
      ledpwm.high();
      led9.low();
    }else if(rgb == YELLOW_COLOUR){
      ledpwm.low();
      led9.high();
    }else{
      led9.set(ledpwm.get());
    }
    break;
  case 10:
    if(rgb == RED_COLOUR){
      ledpwm.high();
      led10.low();
    }else if(rgb == YELLOW_COLOUR){
      ledpwm.low();
      led10.high();
    }else{
      led10.set(ledpwm.get());
    }
    break;
  }
}

bool isModeButtonPressed(){
  return !sw5.get(); // HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin) == GPIO_PIN_RESET;
}

int16_t getAttenuatedCV(uint8_t index, uint16_t* adc_values){
  return Q15_MUL_Q15(adc_values[index*2], takeover.get(index+5)<<1);
}

void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len){
  // cv inputs are ADC_A, C, E, G
  // knobs are ADC_B, D, F, H, I
  if(isModeButtonPressed()){
    for(size_t i=0; i<4; ++i){
      takeover.update(i+5, adc_values[i*2+1], 31);
      if(takeover.taken(i+5))
        setLed(i+1, 0);
      else
        setLed(i+1, 4095);
    }
    takeover.update(9, adc_values[ADC_I], 31);
    if(takeover.taken(9)){
      setLed(5, 0);
      setLed(6, 0);
    }else{
      setLed(5, 4095);
      setLed(6, 4095);
    }      
  }else{
    for(size_t i=0; i<4; ++i){
      takeover.update(i, adc_values[i*2+1], 31);
      if(takeover.taken(i))
        setLed(i+1, getAttenuatedCV(i, adc_values));
      else
        setLed(i+1, 4095);
    }
    takeover.update(4, adc_values[ADC_I], 31);
    if(!takeover.taken(4)){
      setLed(5, 4095);
      setLed(6, 4095);
    }else{
      setLed(5, dac_values[0]);
      setLed(6, dac_values[1]);
    }
    // IIR exponential filter with lambda 0.75
    for(size_t i=0; i<4; ++i){
      int16_t xn = parameter_values[i]*3;
      int16_t x = takeover.get(i);
      int16_t cv = getAttenuatedCV(i, adc_values);
      parameter_values[i] = __USAT((xn+x+cv)>>2, 12);
    }
    parameter_values[4] = __USAT((parameter_values[4]*3+takeover.get(4))>>2, 12);
  }
}

#define PATCH_RESET_COUNTER (600/MAIN_LOOP_SLEEP_MS)

static uint32_t counter = 0;
static void update_preset(){
  switch(owl.getOperationMode()){
  case STARTUP_MODE:
  case STREAM_MODE:
  case LOAD_MODE: {
    uint32_t value = counter*0xfff/PATCH_RESET_COUNTER;
    setLed(1, counter > PATCH_RESET_COUNTER*0.1 ? value : 0);
    setLed(2, counter > PATCH_RESET_COUNTER*0.2 ? value : 0);
    setLed(5, counter > PATCH_RESET_COUNTER*0.3 ? value : 0);
    setLed(6, counter > PATCH_RESET_COUNTER*0.4 ? value : 0);
    setLed(3, counter > PATCH_RESET_COUNTER*0.5 ? value : 0);
    setLed(4, counter > PATCH_RESET_COUNTER*0.6 ? value : 0);
    if(getErrorStatus() != NO_ERROR || isModeButtonPressed())
      owl.setOperationMode(ERROR_MODE);
    break;
  }
  case RUN_MODE:
    if(isModeButtonPressed()){
      takeover.reset(false);
      owl.setOperationMode(CONFIGURE_MODE);
    }else if(getErrorStatus() != NO_ERROR){
      owl.setOperationMode(ERROR_MODE);
    }
    break;
  case CONFIGURE_MODE:
    if(isModeButtonPressed()){
      uint8_t patchselect = program.getProgramIndex();
      if(!sw1.get())
      	patchselect = 1;
      else if(!sw2.get())
      	patchselect = 2;
      else if(!sw3.get())
      	patchselect = 3;
      else if(!sw4.get())
      	patchselect = 4;
      if(patchselect >= registry.getNumberOfPatches())
	patchselect = program.getProgramIndex();
      for(int i=0; i<4; ++i){
	// if(getButtonValue(BUTTON_A+i))
	//   patchselect = i+1;
	if(patchselect == i+1)
	  setLed(7+i, YELLOW_COLOUR);
	else
	  setLed(7+i, NO_COLOUR);
      }
      if(program.getProgramIndex() != patchselect){
	program.loadProgram(patchselect); // enters load mode
	program.resetProgram(false);
	owl.setOperationMode(CONFIGURE_MODE);
	dac_values[0] = dac_values[1] = 0; // reset CV outputs to initial values
      }
      if(takeover.taken(9)){
	int16_t value = takeover.get(9)>>5;
	settings.audio_output_gain = value;
	codec.setOutputGain(value);
      }
    }else{
      takeover.reset(false);
      owl.setOperationMode(RUN_MODE);
      // reset CV outputs to previous values
      setAnalogValue(PARAMETER_F, dac_values[0]);
      setAnalogValue(PARAMETER_G, dac_values[1]);
    }
    break;
  case ERROR_MODE:
    setLed(1, counter > PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(2, counter > PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(5, counter < PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(6, counter < PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(3, counter > PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(4, counter > PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    if(isModeButtonPressed()){
      setErrorStatus(NO_ERROR);
      owl.setOperationMode(CONFIGURE_MODE);
    }
    break;
  }
  if(++counter >= PATCH_RESET_COUNTER)
    counter = 0;
}

void onChangeMode(OperationMode new_mode, OperationMode old_mode){
  ledpwm.high(); // switch button leds to red
  for(int i=1; i<=10; ++i)
    setLed(i, NO_COLOUR);
  setGateValue(BUTTON_E, 0); // this will only have an effect in RUN mode
  setGateValue(BUTTON_F, 0);
  counter = 0;
}

void setup(){
  initLed();
  HAL_GPIO_WritePin(LEDPWM_GPIO_Port, LEDPWM_Pin, GPIO_PIN_SET);
  owl.setup();
   for(size_t i=5; i<9; ++i)
    takeover.set(i, 2048); // set CV attenuation to 1
  takeover.set(9, settings.audio_output_gain<<5);
}

void loop(void){
  MX_USB_HOST_Process(); // todo: enable PWR management
  bool state = !sw4.get(); // HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == GPIO_PIN_RESET;
  if(state != getButtonValue(BUTTON_D)){
    setButtonValue(BUTTON_D, state);
    setLed(10, state ? RED_COLOUR : NO_COLOUR);
  }
  update_preset();
  owl.loop();
}
