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
#include "TakeoverControls.h"
#include "qint.h"
#include "Pin.h"
#include "usb_device.h"
#include "usb_host.h"

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif

// 12x12 bit multiplication with unsigned operands and result
#define U12_MUL_U12(a,b) (__USAT(((uint32_t)(a)*(b))>>12, 12))

#define CV_ATTENUATION_DEFAULT 2186 // calibrated to provide 1V/oct over 5V

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

TakeoverControls<10, int16_t> takeover;
int16_t dac_values[2] = {0, 0};
bool button_led_values[4] = {false};
volatile uint8_t patchselect;

extern int16_t parameter_values[NOF_PARAMETERS];

int16_t getParameterValue(uint8_t pid){
  if(pid < 5)
    return takeover.get(pid);
  else if(pid < NOF_PARAMETERS)
    return parameter_values[pid];
  return 0;
}

// called from program, MIDI, or (potentially) digital bus
void setParameterValue(uint8_t pid, int16_t value){
  if(pid < 5){
    takeover.set(pid, value);
    takeover.reset(pid, false);
  }else if(pid < NOF_PARAMETERS){
    parameter_values[pid] = value;
  }
}

bool updatePin(size_t bid, Pin pin){
  // button id 'bid' goes from 1 to 4
  bool state = !pin.get();
  if(owl.getOperationMode() == RUN_MODE){
    setButtonValue(bid+3, state);
    setLed(bid+6, state ? RED_COLOUR : NO_COLOUR);
  }else if(owl.getOperationMode() == CONFIGURE_MODE && state){
    if(patchselect == bid && registry.hasPatch(bid+4)){
      patchselect = bid+4;
    }else if(registry.hasPatch(bid)){
      patchselect = bid;
    }else if(registry.hasPatch(bid+4)){
      patchselect = bid+4;
    }
  }
  return state;
}
  
void onChangePin(uint16_t pin){
  switch(pin){
  case SW1_Pin:
    setButtonValue(PUSHBUTTON, updatePin(1, sw1));
    break;
  case SW2_Pin:
    updatePin(2, sw2);
    break;
  case SW3_Pin:
    updatePin(3, sw3);
    break;
  }
}

static uint16_t scaleForDac(int16_t value){
  return U12_MUL_U12(value + 70, 3521);
}

void setAnalogValue(uint8_t ch, int16_t value){
  if(owl.getOperationMode() == RUN_MODE){
    extern DAC_HandleTypeDef hdac;
    switch(ch){
    case PARAMETER_F:
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, scaleForDac(value));
      dac_values[0] = value;
      break;
    case PARAMETER_G:
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, scaleForDac(value));
      dac_values[1] = value;
      break;
    case PARAMETER_BA:
      takeover.set(5, value);
      break;
    case PARAMETER_BB:
      takeover.set(6, value);
      break;
    case PARAMETER_BC:
      takeover.set(7, value);
      break;
    case PARAMETER_BD:
      takeover.set(8, value);
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

void setButtonLed(Pin pin, uint32_t rgb){
  if(rgb == RED_COLOUR){
    ledpwm.high();
    pin.low();
  }else if(rgb == YELLOW_COLOUR){
    ledpwm.low();
    pin.high();
  }else{
    pin.set(ledpwm.get());
  }
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
    setButtonLed(led7, rgb);
    break;
  case 8:
    setButtonLed(led8, rgb);
    break;
  case 9:
    setButtonLed(led9, rgb);
    break;
  case 10:
    setButtonLed(led10, rgb);
    break;
  }
}

bool isModeButtonPressed(){
  return !sw5.get(); // HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin) == GPIO_PIN_RESET;
}

int16_t getAttenuatedCV(uint8_t index, uint16_t* adc_values){
  // 12x12 bit multiplication with signed operands and no saturation
  return ((int32_t)adc_values[index*2] * takeover.get(index+5)) >> 12;
}

static uint16_t smooth_adc_values[NOF_ADC_VALUES];
extern "C"{
  void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
    // this runs at apprx 7.5kHz
    // with 144 cycles sample time and PCLK2 = 84MHz, div 8
    // giving a filter settling time of less than 3ms
    extern uint16_t adc_values[NOF_ADC_VALUES];
    for(size_t i=0; i<NOF_ADC_VALUES; ++i){
      // IIR exponential filter with lambda 0.75: y[n] = 0.75*y[n-1] + 0.25*x[n]
      smooth_adc_values[i] = (smooth_adc_values[i]*3 + adc_values[i]) >> 2;
    }
  }
}

void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len){
  // cv inputs are ADC_A, C, E, G
  // knobs are ADC_B, D, F, H, I
  if(isModeButtonPressed()){
    for(size_t i=0; i<4; ++i){
      int32_t value = smooth_adc_values[i*2+1] - 2048;
      // attenuvert from -2x to +2x
      if(value > 0){
	value = (value*value) >> 9;
      }else{
	value = - (value*value) >> 9;
      }
      takeover.update(i+5, value, 31);
      if(takeover.taken(i+5))
        setLed(i+1, 0);
      else
        setLed(i+1, 4095);
    }
    takeover.update(9, smooth_adc_values[ADC_I], 31);
    if(takeover.taken(9)){
      setLed(5, 0);
      setLed(6, 0);
    }else{
      setLed(5, 4095);
      setLed(6, 4095);
    }      
  }else{
    for(size_t i=0; i<4; ++i){
      takeover.update(i, smooth_adc_values[i*2+1], 31);
      if(takeover.taken(i))
        setLed(i+1, abs(getAttenuatedCV(i, smooth_adc_values)));
      else
        setLed(i+1, 4095);
    }
    takeover.update(4, smooth_adc_values[ADC_I], 31);
    if(takeover.taken(4)){
      setLed(5, dac_values[0]);
      setLed(6, dac_values[1]);
    }else{
      setLed(5, 4095);
      setLed(6, 4095);
    }
    for(size_t i=0; i<4; ++i){
      int16_t x = takeover.get(i);
      int16_t cv = getAttenuatedCV(i, smooth_adc_values);
      parameter_values[i] = __USAT(x+cv, 12);
    }
    parameter_values[4] = __USAT(takeover.get(4), 12);
  }
}

#define PATCH_RESET_COUNTER (600/MAIN_LOOP_SLEEP_MS)
uint16_t progress = 0;
void setProgress(uint16_t value, const char* msg){
  // debugMessage(msg, (int)(100*value/4095));
  progress = value == 4095 ? 0 : value*6;
}

static uint32_t counter = 0;
static void update_preset(){
  switch(owl.getOperationMode()){
  case STARTUP_MODE:
  case STREAM_MODE:
  case LOAD_MODE: {
    uint16_t value = progress;
    if(value == 0)
      value = counter*4095*6/PATCH_RESET_COUNTER;
    setLed(1, 4095 - __USAT(4095*0-value, 12));
    setLed(2, 4095 - __USAT(4095*1-value, 12));
    setLed(5, 4095 - __USAT(4095*2-value, 12));
    setLed(6, 4095 - __USAT(4095*3-value, 12));
    setLed(3, 4095 - __USAT(4095*4-value, 12));
    setLed(4, 4095 - __USAT(4095*5-value, 12));
    if(getErrorStatus() != NO_ERROR || isModeButtonPressed())
      owl.setOperationMode(ERROR_MODE);
    break;
  }
  case RUN_MODE:
    if(isModeButtonPressed()){
      owl.setOperationMode(CONFIGURE_MODE);
    }else if(getErrorStatus() != NO_ERROR){
      owl.setOperationMode(ERROR_MODE);
    }
    break;
  case CONFIGURE_MODE:
    if(isModeButtonPressed()){
      for(int i=1; i<=4; ++i){
	uint32_t colour = NO_COLOUR;
	if(patchselect == i)
	  colour = YELLOW_COLOUR;
	else if(patchselect == i+4)
	  colour = RED_COLOUR;
	setLed(6+i, colour);
      }
      if(takeover.taken(9)){
	uint8_t value = (takeover.get(9) >> 6) + 63;
	if(settings.audio_output_gain != value){
	  settings.audio_output_gain = value;
	  codec.setOutputGain(value);
	}
      }
    }else{
      if(program.getProgramIndex() != patchselect &&
	 registry.hasPatch(patchselect)){
	// change patch on mode button release
	program.loadProgram(patchselect); // enters load mode (calls onChangeMode)
	program.resetProgram(false);
      }else{
	owl.setOperationMode(RUN_MODE);
      }
      takeover.reset(false);
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

void onStartProgram(){
  // new patch selected or loaded
  takeover.set(0, getAnalogValue(ADC_B));
  takeover.set(1, getAnalogValue(ADC_D));
  takeover.set(2, getAnalogValue(ADC_F));
  takeover.set(3, getAnalogValue(ADC_H));
  takeover.set(4, getAnalogValue(ADC_I));
  takeover.reset(0, true);
  takeover.reset(1, true);
  takeover.reset(2, true);
  takeover.reset(3, true);
  takeover.reset(4, true);
  memset(dac_values, 0, sizeof(dac_values)); // reset CV outputs to initial values
  memset(button_led_values, 0, sizeof(button_led_values)); // reset leds
}

void onChangeMode(uint8_t new_mode, uint8_t old_mode){
  if(new_mode == CONFIGURE_MODE){
    // entering config mode
    HAL_GPIO_WritePin(TR_OUT1_GPIO_Port, TR_OUT1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(TR_OUT2_GPIO_Port, TR_OUT2_Pin, GPIO_PIN_SET);
    takeover.reset(false);
    patchselect = program.getProgramIndex();
    // store current LED settings
    button_led_values[0] = !led7.get();
    button_led_values[1] = !led8.get();
    button_led_values[2] = !led9.get();
    button_led_values[3] = !led10.get();
  }else if(new_mode == RUN_MODE){
    // we are either returning to the same patch or starting a new one
    ledpwm.high(); // switch button leds to red
    for(int i=1; i<7; ++i)
      setLed(i, NO_COLOUR);
    for(int i=7; i<=10; ++i)
      setLed(i, button_led_values[i-7] ? RED_COLOUR : NO_COLOUR);
    // reset CV outputs to previous values
    setAnalogValue(PARAMETER_F, dac_values[0]);
    setAnalogValue(PARAMETER_G, dac_values[1]);
    // todo: reset gates to previous values
  }
  counter = 0;
}

void onSetup(){
  initLed();
  HAL_GPIO_WritePin(LEDPWM_GPIO_Port, LEDPWM_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(TR_OUT1_GPIO_Port, TR_OUT1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(TR_OUT2_GPIO_Port, TR_OUT2_Pin, GPIO_PIN_SET);
  for(size_t i=5; i<9; ++i){
    takeover.set(i, CV_ATTENUATION_DEFAULT);
    takeover.reset(i, false);
  }
  takeover.set(9, settings.audio_output_gain<<5);
  takeover.reset(9, false);
  patchselect = program.getProgramIndex();

  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();

  /* init code for USB_HOST */
  MX_USB_HOST_Init();
}

void onLoop(){
  MX_USB_HOST_Process(); // todo: enable PWR management
  static bool sw4_state = false;
  if(sw4_state != !sw4.get())
    sw4_state = updatePin(4, sw4);
  update_preset();
}
