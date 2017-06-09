#include <string.h>
#include "device.h"
#include "Owl.h"
#include "Codec.h"
#include "MidiReader.h"
#include "MidiController.h"
#include "ProgramVector.h"
#include "ProgramManager.h"
#include "ApplicationSettings.h"
#include "cmsis_os.h"
#include "BitState.hpp"
#include "rainbow.h"
#include "errorhandlers.h"

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif

Codec codec;
MidiReader midireader;
MidiController midi;
ApplicationSettings settings;

uint16_t adc_values[NOF_ADC_VALUES];
uint16_t dac_values[2];

uint16_t getAnalogValue(uint8_t ch){
  if(ch < NOF_ADC_VALUES)
    return adc_values[ch];
  else
    return 0;
}

void setAnalogValue(uint8_t ch, uint16_t value){
  if(ch < 2){
    value &= 0xfff;
    dac_values[ch] = value;
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t pin){
  switch(pin){
  case PUSHBUTTON_Pin:
    setButtonValue(PUSHBUTTON, !(PUSHBUTTON_GPIO_Port->IDR & PUSHBUTTON_Pin));
    break;
  }
}

void setLed(uint32_t rgb){
  // rgb should be a 3x 10 bit value
  TIM2->CCR1 = 1023 - ((rgb>>20)&0x3ff);
  TIM3->CCR4 = 1023 - ((rgb>>10)&0x3ff);
  TIM5->CCR2 = 1023 - ((rgb>>00)&0x3ff);
}

void setLed(int16_t red, int16_t green, int16_t blue){
  // parameters should be 0-4095
  red = 1023-(red>>2);
  green = 1023-(green>>2);
  blue = 1023-(blue>>2);
  TIM2->CCR1 = red;
  TIM3->CCR4 = green;
  TIM5->CCR2 = blue;
// LED_R PA0/LGP1 TIM2_CH1
// LED_G PA1/LGP2 TIM5_CH2
// LED_B PB1/LGP6 TIM3_CH4
}

void initLed(){
  // Initialise RGB LED PWM timers
  extern TIM_HandleTypeDef htim2;
  extern TIM_HandleTypeDef htim3;
  extern TIM_HandleTypeDef htim5;

  // Red
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  // only channels 1 and 8 have complementary output
  // HAL_TIMEx_PWMN_Start(&htim2, TIM_CHANNEL_1);
	
  // Green
  HAL_TIM_Base_Start(&htim5);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
  // HAL_TIMEx_PWMN_Start(&htim5,TIM_CHANNEL_2);

  // Blue
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  // HAL_TIMEx_PWMN_Start(&htim3,TIM_CHANNEL_4);
}

void setup(){
  settings.init();
  codec.reset();
  codec.bypass(0);

  program.startManager();

  initLed();
  setLed(1000, 1000, 1000);

  extern ADC_HandleTypeDef hadc3;
  // extern DMA_HandleTypeDef hdma_adc3;
  HAL_StatusTypeDef ret = HAL_ADC_Start_DMA(&hadc3, (uint32_t*)adc_values, NOF_ADC_VALUES);
  if(ret != HAL_OK)
    error(CONFIG_ERROR, "ADC Start failed");

  program.loadProgram(1);
  program.startProgram(false);

  midi.init(0);
}
 
void loop(void){
  taskYIELD();
  midi.push();
  setLed(rainbow[(4095-adc_values[3])>>2]);
  // setLed(4095-adc_values[0], 4095-adc_values[1], 4095-adc_values[2]);
}

extern "C"{
  // more from USB device interface
  void midi_rx_usb_buffer(uint8_t *buffer, uint32_t length){
    for(uint32_t i=0; i<length; i+=4)
      midireader.readMidiFrame(buffer+i);
  }
  // void midi_tx_usb_buffer(uint8_t* buffer, uint32_t length);
}
