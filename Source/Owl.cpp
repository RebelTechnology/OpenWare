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

#if defined USE_RGB_LED || defined OWL_MAGUS
#include "rainbow.h"
#endif /* OWL_TESSERACT */

#ifdef USE_SCREEN
#include "Graphics.h"
Graphics graphics;
#endif /* USE_SCREEN */

#ifdef OWL_MAGUS
#include "HAL_TLC5946.h"
#include "HAL_MAX11300.h"
// #include "HAL_OLED.h"
#include "HAL_Encoders.h"
#endif

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
MidiController midi;
#ifdef USE_USB_HOST
MidiReader midihost;
#endif /* USE_USB_HOST */
ApplicationSettings settings;

uint16_t adc_values[NOF_ADC_VALUES];
uint16_t dac_values[2];
uint32_t ledstatus;

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

#ifdef OWL_MICROLAB_LED
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
#endif /* OWL_MICROLAB_LED */

#ifdef USE_RGB_LED
void setLed(uint32_t rgb){
  // rgb should be a 3x 10 bit value
#if defined OWL_TESSERACT
  TIM2->CCR1 = 1023 - ((rgb>>20)&0x3ff);
  TIM3->CCR4 = 1023 - ((rgb>>10)&0x3ff);
  TIM5->CCR2 = 1023 - ((rgb>>00)&0x3ff);
#elif defined OWL_MINILAB
  TIM2->CCR1 = 1023 - ((rgb>>20)&0x3ff);
  TIM5->CCR2 = 1023 - ((rgb>>10)&0x3ff);
  TIM4->CCR3 = 1023 - ((rgb>>00)&0x3ff);
#elif defined OWL_MICROLAB
  TIM2->CCR1 = 1023 - ((rgb>>20)&0x3ff);
  TIM3->CCR4 = 1023 - ((rgb>>10)&0x3ff);
  TIM5->CCR2 = 1023 - ((rgb>>00)&0x3ff);
#endif
}

void setLed(int16_t red, int16_t green, int16_t blue){
  // parameters should be 0-4095
  red = 1023-(red>>2);
  green = 1023-(green>>2);
  blue = 1023-(blue>>2);
#if defined OWL_TESSERACT
  TIM2->CCR1 = red;
  TIM3->CCR4 = green;
  TIM5->CCR2 = blue;
#elif defined OWL_MINILAB
  TIM2->CCR1 = red;
  TIM5->CCR2 = green;
  TIM4->CCR3 = blue;
#elif defined OWL_MICROLAB
  TIM2->CCR1 = red;
  TIM3->CCR4 = green;
  TIM5->CCR2 = blue;
#endif
}

void initLed(){
  // MiniLab
  // PWM1: TIM2_CH1
  // PWM2: TIM5_CH2
  // PWM3: TIM4_CH3
  // Tesseract and MicroLab
  // LED_R PA0/LGP1 TIM2_CH1
  // LED_G PA1/LGP2 TIM5_CH2
  // LED_B PB1/LGP6 TIM3_CH4

  // Initialise RGB LED PWM timers
#if defined OWL_TESSERACT
  extern TIM_HandleTypeDef htim2;
  extern TIM_HandleTypeDef htim3;
  extern TIM_HandleTypeDef htim5;
  // Red
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  // Green
  HAL_TIM_Base_Start(&htim5);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
  // Blue
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
#elif defined OWL_MINILAB
  extern TIM_HandleTypeDef htim2;
  extern TIM_HandleTypeDef htim5;
  extern TIM_HandleTypeDef htim4;
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_Base_Start(&htim5);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
  HAL_TIM_Base_Start(&htim4);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
#elif defined OWL_MICROLAB
  extern TIM_HandleTypeDef htim2;
  extern TIM_HandleTypeDef htim3;
  extern TIM_HandleTypeDef htim5;
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_Base_Start(&htim5);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
#endif
}
#endif /* USE_RGB_LED */

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
#ifdef OWL_MINILAB
  case TRIG_SW1_Pin:
    setButtonValue(BUTTON_A, !(TRIG_SW1_GPIO_Port->IDR & TRIG_SW1_Pin));
    setButtonValue(PUSHBUTTON, !(TRIG_SW1_GPIO_Port->IDR & TRIG_SW1_Pin));
    break;
  case TRIG_SW2_Pin:
    setButtonValue(BUTTON_B, !(TRIG_SW2_GPIO_Port->IDR & TRIG_SW2_Pin));
    break;
  case TRIG_SW3_Pin:
    setButtonValue(BUTTON_C, !(TRIG_SW3_GPIO_Port->IDR & TRIG_SW3_Pin));
    break;
  case TRIG_SW4_Pin:
    setButtonValue(BUTTON_D, !(TRIG_SW4_GPIO_Port->IDR & TRIG_SW4_Pin));
    break;
#endif
#ifdef OWL_MICROLAB
  case SW1_Pin:
    setButtonValue(BUTTON_A, !(SW1_GPIO_Port->IDR & SW1_Pin));
    setButtonValue(PUSHBUTTON, !(SW1_GPIO_Port->IDR & SW1_Pin));
    break;
  case SW2_Pin:
    setButtonValue(BUTTON_B, !(SW2_GPIO_Port->IDR & SW2_Pin));
    setParameterValue(PARAMETER_E, (SW2_GPIO_Port->IDR & SW2_Pin) == 0 ? 4095 : 0);
    ledstatus = getButtonValue(BUTTON_B) ? 0xffc00 : 0;
    break;
  case SW3_Pin:
    setButtonValue(BUTTON_C, !(SW3_GPIO_Port->IDR & SW3_Pin));
    ledstatus = getButtonValue(BUTTON_C) ? 0x3ff00000 : 0;
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
#ifdef USE_RGB_LED
  ledstatus = getButtonValue(PUSHBUTTON) ? 0x3ff : 0;
#endif
}


static TickType_t xLastWakeTime;
static TickType_t xFrequency;

void setup(){
  ledstatus = 0;
  settings.init();
#ifdef USE_CODEC
  extern SPI_HandleTypeDef CODEC_SPI;
  codec.begin(&CODEC_SPI);
  // codec.set(0);
#if defined OWL_MICROLAB || defined OWL_MINILAB // || defined OWL_MAGUS
  codec.setOutputGain(127-9); // -9dB
#else
  codec.setOutputGain(0); // 0dB
#endif
  codec.bypass(false);
#endif /* USE_CODEC */

  program.startManager();

#ifdef OWL_MAGUS
  ledstatus = 0x3fful << 20;
  extern SPI_HandleTypeDef hspi5;
  /* OLED_init(&hspi5); */
  Encoders_init(&hspi5);

  // LEDs
  TLC5946_init(&hspi5);
  Magus_setRGB_DC(20, 20, 20); // todo balance levels
  for(int i=0; i<16; i+=3)
    setLed(i, (1023<<00));
  for(int i=1; i<16; i+=3)
    setLed(i, (1023<<10));
  for(int i=2; i<16; i+=3)
    setLed(i, (1023<<20));
    // setLed(i, (512<<20) + (512<<10) + 512);
  // Start LED Driver PWM
  extern TIM_HandleTypeDef htim3;
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  // HAL_TIMEx_PWMN_Start(&htim3,TIM_CHANNEL_4);

  // Pixi
  MAX11300_init(&hspi5);
  MAX11300_setDeviceControl(DCR_RESET);
  HAL_Delay(1000);
  MAX11300_setDeviceControl(DCR_DACCTL_ImmUpdate|DCR_DACREF_Int|DCR_ADCCTL_ContSweep /* |DCR_ADCCONV_200ksps|DCR_BRST_Contextual*/);

  for(int i=0; i<8; ++i)
    MAX11300_setPortMode(i+1, PCR_Range_ADC_M5_P5|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
  for(int i=8; i<16; ++i)
    MAX11300_setPortMode(i+1, PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  for(int i=16; i<20; ++i)
    MAX11300_setPortMode(i+1, PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
  for(int i=0; i<20; ++i)
    MAX11300_setDACValue(i, 0);
#endif
  
#ifdef USE_RGB_LED
  initLed();
  setLed(1000, 1000, 1000);
#endif /* USE_RGB_LED */

#ifdef OWL_MICROLAB_LED
  initLed();
  setLed(LED1, 0);
  setLed(LED2, 0);
  setLed(LED3, 0);
  setLed(LED4, 0);
#endif /* OWL_MICROLAB_LED */

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

  xLastWakeTime = xTaskGetTickCount();
  xFrequency = 20 / portTICK_PERIOD_MS; // 20mS, 50Hz refresh rate
}

#ifdef OWL_MAGUS
void setLed(uint8_t led, uint32_t rgb){
  // rgb should be a 3x 10 bit value
  Magus_setRGB(led+1, ((rgb>>20)&0x3ff)<<2, ((rgb>>10)&0x3ff)<<2, ((rgb>>00)&0x3ff)<<2);
  // Magus_setRGB(led+1, (rgb>>20)&0xfff, (rgb>>10)&0xfff, (rgb>>00)&0xfff);
}
#endif

void loop(void){
  vTaskDelayUntil(&xLastWakeTime, xFrequency);
  // taskYIELD();
  midi.push();

#ifdef OWL_MAGUS
    // also update LEDs and MAX11300
    TLC5946_Refresh_GS();
    // for(int i=0; i<16; i+=2)
    //   setLed(i, MAX11300_getADCValue(i) + baseled);    
    // for(int i=0; i<16; i+=1){
    //   uint16_t value = MAX11300_readADC(i+1);
    //   setLed(i, baseled | ((value << 10) & 0x3ff) | (value & 0x3ff));
    //   graphics.params.parameters[i] = value;
    // }
    Encoders_readAll();
    extern uint16_t rgENC_Values[7];
    graphics.params.updateEncoders((int16_t*)rgENC_Values, 7);
    // extern uint8_t rgADCData_Rx[41];
    // graphics.params.updateValues((uint16_t*)(rgADCData_Rx+1), 16);

    MAX11300_bulkreadADC();
    for(int i=0; i<8; ++i){
      graphics.params.updateValue(i, MAX11300_getADCValue(i+1));
      setLed(i, ledstatus ^ (graphics.params.parameters[i] >> 2)); // MAX11300_getADCValue(i+1)>>2));
    }
    for(int i=8; i<16; ++i){
      graphics.params.updateValue(i, 0);
      MAX11300_setDACValue(i+1, graphics.params.parameters[i]);
      setLed(i, ledstatus ^ (graphics.params.parameters[i] >> 2)); // MAX11300_getADCValue(i+1)>>2));
    }
    MAX11300_bulkwriteDAC();
      
#else
#endif /* OWL_MAGUS */

#ifdef USE_SCREEN
  graphics.draw();
  graphics.display();
#endif /* USE_SCREEN */

#ifdef USE_RGB_LED
  uint32_t colour =
    (adc_values[ADC_A]>>3)+
    (adc_values[ADC_B]>>3)+
    (adc_values[ADC_C]>>3)+
    (adc_values[ADC_D]>>3);
#ifdef ADC_E
  colour += (adc_values[ADC_E]>>3);
#endif
  colour &= 0x3ff;
  setLed(ledstatus ^ rainbow[colour]);
  // setLed(4095-adc_values[0], 4095-adc_values[1], 4095-adc_values[2]);
#endif /* USE_RGB_LED */
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
    for(uint16_t i=0; i<length; i+=4){
      if(!mididevice.readMidiFrame(buffer+i)){
	mididevice.reset();
      }
    }
  }
  // void midi_tx_usb_buffer(uint8_t* buffer, uint32_t length);

#ifdef USE_USB_HOST
  void midi_host_reset(void){
    midihost.reset();
    ledstatus ^= 0x3ff00000;
  }
  void midi_host_rx(uint8_t *buffer, uint32_t length){
    for(uint16_t i=0; i<length; i+=4){
      if(!midihost.readMidiFrame(buffer+i)){
	midihost.reset();
      }else{
	ledstatus ^= 0x80000; // 0x20000000
      }
    }
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
