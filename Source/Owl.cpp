#include <string.h>
#include <stdlib.h>
#include "device.h"
#ifdef USE_USB_HOST
#include "usbh_core.h"
#include "usbh_midi.h"
#endif /* USE_USB_HOST */
#include "Owl.h"
#include "Codec.h"
#include "MidiReader.h"
#include "MidiController.h"
#include "ProgramVector.h"
#include "ProgramManager.h"
#include "ApplicationSettings.h"
#include "cmsis_os.h"
#include "BitState.hpp"
#include "errorhandlers.h"

#ifdef OWL_TESSERACT
#include "rainbow.h"
#endif /* OWL_TESSERACT */

#ifdef USE_SCREEN
#include "Graphics.h"
Graphics graphics;
#endif /* USE_SCREEN */

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
MidiReader mididevice;
MidiReader midihost;
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

#ifdef OWL_MICROLAB
void setLed(uint8_t ch, uint16_t brightness){
  // brightness should be a 10 bit value
  brightness = brightness&0x3ff;
  switch(ch){
  case LED1:
    // left
    TIM2->CCR1 = brightness;
    break;
  case LED2:
    // top
    TIM4->CCR3 = brightness;
    break;
  case LED3:
    // right
    TIM3->CCR4 = brightness;
    break;
  case LED4:
    // bottom
    TIM5->CCR2 = brightness;
    break;
  }
}

void initLed(){
  // Initialise RGB LED PWM timers
  extern TIM_HandleTypeDef htim2;
  extern TIM_HandleTypeDef htim3;
  extern TIM_HandleTypeDef htim4;
  extern TIM_HandleTypeDef htim5;
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  HAL_TIM_Base_Start(&htim4);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_Base_Start(&htim5);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
}

#endif /* OWL_MICROLAB */

#ifdef OWL_TESSERACT
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
#endif /* OWL_TESSERACT */

void HAL_GPIO_EXTI_Callback(uint16_t pin){
  switch(pin){
#ifdef PUSHBUTTON_Pin
  case PUSHBUTTON_Pin:
    setButtonValue(PUSHBUTTON, !(PUSHBUTTON_GPIO_Port->IDR & PUSHBUTTON_Pin));
    midi.sendCc(PUSHBUTTON, getButtonValue(PUSHBUTTON) ? 127 : 0);
    break;
#endif
#ifdef OWL_TESSERACT
  case TOGGLE_A1_Pin:
    break;
  case TOGGLE_A2_Pin:
    break;
  case TOGGLE_B1_Pin:
    break;
  case TOGGLE_B2_Pin:
    break;
#endif
#ifdef OWL_MICROLAB
    // todo remove
  case TRIG1_Pin:
  case TRIG2_Pin:    
    setButtonValue(PUSHBUTTON, !(TRIG1_GPIO_Port->IDR & TRIG1_Pin));
    setParameterValue(PARAMETER_E, (TRIG2_GPIO_Port->IDR & TRIG2_Pin) == 0 ? 4095 : 0);
    setLed(LED1, 0);
    setLed(LED2, 0);
    setLed(LED3, 0);
    setLed(LED4, 0);
    if(!(TRIG1_GPIO_Port->IDR & TRIG1_Pin)){
      setLed(LED1, 1023);
      setLed(LED3, 1023);
    }
    if(!(TRIG2_GPIO_Port->IDR & TRIG2_Pin)){
      setLed(LED2, 1023);
      setLed(LED4, 1023);
    }
    break;
#endif
#ifdef OWL_MICROLAB_LED
  case TRIG1_Pin:
    setButtonValue(PUSHBUTTON, !(TRIG1_GPIO_Port->IDR & TRIG1_Pin));
    break;
  case TRIG2_Pin:
    setParameterValue(PARAMETER_E, (TRIG2_GPIO_Port->IDR & TRIG2_Pin) == 0 ? 4095 : 0);
    break;
#endif
#ifdef OWL_PLAYERF7
  // sw1() pg14
  // sw2() pb4
  // tr1() pc11
  // tr2() pc10
  case ENC1_SW_Pin: // GPIO_PIN_14:
    setButtonValue(PUSHBUTTON, (ENC1_SW_GPIO_Port->IDR & ENC1_SW_Pin));
    break;
  case ENC2_SW_Pin: // GPIO_PIN_4:
    setButtonValue(PUSHBUTTON, (ENC2_SW_GPIO_Port->IDR & ENC2_SW_Pin));
    break;
  case TR_IN_A_Pin: // GPIO_PIN_11:
    setButtonValue(BYPASS_BUTTON, !(TR_IN_A_GPIO_Port->IDR & TR_IN_A_Pin));
    break;
  case TR_IN_B_Pin: // GPIO_PIN_10:
    setButtonValue(BYPASS_BUTTON, !(TR_IN_B_GPIO_Port->IDR & TR_IN_B_Pin));
    break;
#endif
  }
}

void setup(){
  settings.init();
#ifdef USE_CODEC
  extern SPI_HandleTypeDef CODEC_SPI;
  codec.begin(&CODEC_SPI);
  codec.set(0);
  codec.bypass(false);
#endif /* USE_CODEC */

  program.startManager();

#ifdef OWL_TESSERACT
  initLed();
  setLed(1000, 1000, 1000);
#endif /* OWL_TESSERACT */

#ifdef OWL_MICROLAB
  initLed();
  setLed(LED1, 0);
  setLed(LED2, 0);
  setLed(LED3, 0);
  setLed(LED4, 0);
#endif /* OWL_MICROLAB */

#ifdef USE_ENCODERS
  extern TIM_HandleTypeDef ENCODER_TIM1;
  extern TIM_HandleTypeDef ENCODER_TIM2;
  __HAL_TIM_SET_COUNTER(&ENCODER_TIM1, INT16_MAX/2);
  __HAL_TIM_SET_COUNTER(&ENCODER_TIM2, INT16_MAX/2);
  HAL_TIM_Encoder_Start_IT(&ENCODER_TIM1, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start_IT(&ENCODER_TIM2, TIM_CHANNEL_ALL);
#endif /* OWL_PLAYERF7 */

#ifdef USE_SCREEN
  extern SPI_HandleTypeDef OLED_SPI;
  graphics.begin(&OLED_SPI);
#endif /* USE_SCREEN */

#ifdef USE_ADC
  extern ADC_HandleTypeDef ADC_PERIPH;
  HAL_StatusTypeDef ret = HAL_ADC_Start_DMA(&ADC_PERIPH, (uint32_t*)adc_values, NOF_ADC_VALUES);
  if(ret != HAL_OK)
    error(CONFIG_ERROR, "ADC Start failed");
#endif /* USE_ADC */

  program.loadProgram(1);
  program.startProgram(false);

  midi.init(0);
}
 
void loop(void){
  taskYIELD();
  midi.push();
#ifdef OWL_TESSERACT
  setLed(rainbow[((adc_values[0]>>3)+(adc_values[1]>>3)+(adc_values[2]>>3)+(adc_values[3]>>3))&0x3ff]);
  // setLed(4095-adc_values[0], 4095-adc_values[1], 4095-adc_values[2]);
#endif /* OWL_TESSERACT */
#ifdef OWL_MICROLAB_LED
  setLed(LED1, adc_values[ADC_A]>>2);
  setLed(LED2, adc_values[ADC_B]>>2);
  setLed(LED3, adc_values[ADC_C]>>2);
  setLed(LED4, adc_values[ADC_D]>>2);
#endif /* OWL_MICROLAB */
}

extern "C"{
  // more from USB device interface
  void midi_device_rx(uint8_t *buffer, uint32_t length){
    for(uint16_t i=0; i<length; i+=4)
      mididevice.readMidiFrame(buffer+i);
  }
  // void midi_tx_usb_buffer(uint8_t* buffer, uint32_t length);

#ifdef USE_USB_HOST
  void midi_host_rx(uint8_t *buffer, uint32_t length){
    for(uint16_t i=0; i<length; i+=4)
      midihost.readMidiFrame(buffer+i);
  }
#endif /* USE_USB_HOST */

#ifdef USE_ENCODERS
  void encoderReset(uint8_t encoder, int32_t value){
  extern TIM_HandleTypeDef ENCODER_TIM1;
  extern TIM_HandleTypeDef ENCODER_TIM2;
    if(encoder == 0)
      __HAL_TIM_SetCounter(&ENCODER_TIM1, value);
    else if(encoder == 1)
      __HAL_TIM_SetCounter(&ENCODER_TIM2, value);
  }

  void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
  extern TIM_HandleTypeDef ENCODER_TIM1;
  extern TIM_HandleTypeDef ENCODER_TIM2;
    if(htim == &ENCODER_TIM1)
      encoderChanged(0, __HAL_TIM_GET_COUNTER(&ENCODER_TIM1));
    else if(htim == &ENCODER_TIM2)
      encoderChanged(1, __HAL_TIM_GET_COUNTER(&ENCODER_TIM2));
  }
#endif /* OWL_PLAYERF7 */

}
