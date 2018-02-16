#include <string.h>
#include <stdlib.h>
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
#include "errorhandlers.h"
#include "message.h"

#ifdef OWL_MAGUS
#include "purple-blue-cyan.h"
#include "orange-red-pink.h"
#include "HAL_TLC5946.h"
#include "HAL_MAX11300.h"
// #include "HAL_OLED.h"
#include "HAL_Encoders.h"
#endif

#ifdef USE_SCREEN
#include "Graphics.h"
Graphics graphics;
#endif /* USE_SCREEN */

#if defined USE_RGB_LED
#include "rainbow.h"
#endif /* OWL_TESSERACT */

#ifdef USE_USB_HOST
#include "usbh_core.h"
#include "usbh_midi.h"
#endif /* USE_USB_HOST */

#ifdef USE_DIGITALBUS
#include "bus.h"
#endif /* USE_DIGITALBUS */

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
    setButtonValue(BUTTON_A, !(ENC1_SW_GPIO_Port->IDR & ENC1_SW_Pin));
    setButtonValue(PUSHBUTTON, !(ENC1_SW_GPIO_Port->IDR & ENC1_SW_Pin));
    break;
  case ENC2_SW_Pin: // GPIO_PIN_4:
    setButtonValue(BUTTON_B, !(ENC2_SW_GPIO_Port->IDR & ENC2_SW_Pin));
    break;
  case TR_IN_A_Pin: // GPIO_PIN_11:
    setButtonValue(BUTTON_C, !(TR_IN_A_GPIO_Port->IDR & TR_IN_A_Pin));
    break;
  case TR_IN_B_Pin: // GPIO_PIN_10:
    setButtonValue(BUTTON_D, !(TR_IN_B_GPIO_Port->IDR & TR_IN_B_Pin));
    break;
#endif
#ifdef OWL_PRISM
  case ENC1_SW_Pin:
    setButtonValue(BUTTON_A, !(ENC1_SW_GPIO_Port->IDR & ENC1_SW_Pin));
    setButtonValue(PUSHBUTTON, !(ENC1_SW_GPIO_Port->IDR & ENC1_SW_Pin));
    break;
  case ENC2_SW_Pin:
    setButtonValue(BUTTON_B, !(ENC2_SW_GPIO_Port->IDR & ENC2_SW_Pin));
    break;
#endif    
  }
#ifdef USE_RGB_LED
  ledstatus = getButtonValue(PUSHBUTTON) ? 0x3ff : 0;
#endif
}

#ifdef OWL_MAGUS
bool updateMAX11300 = false;
// int16_t dynamicParameterValues[NOF_PARAMETERS];
uint8_t portMode[20];
void setPortMode(uint8_t index, uint8_t mode){
  if(index < 20){
    if(portMode[index] != mode){
      portMode[index] = mode;
      updateMAX11300 = true;
    }
  }
}
uint8_t getPortMode(uint8_t index){
  if(index < 20)
    return portMode[index];
  return 0;
}
#endif

static TickType_t xLastWakeTime;
static TickType_t xFrequency;

void setup(){
#ifdef USE_SCREEN
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); // OLED off
#endif
#ifdef OWL_MAGUS
  HAL_GPIO_WritePin(TLC_BLANK_GPIO_Port, TLC_BLANK_Pin, GPIO_PIN_RESET); // LEDs off
#endif /* OWL_MAGUS */
  
  ledstatus = 0;
  settings.init();
#ifdef USE_CODEC
  extern SPI_HandleTypeDef CODEC_SPI;
  codec.begin(&CODEC_SPI);
  // codec.set(0);
#if defined OWL_MICROLAB || defined OWL_MINILAB // || defined OWL_MAGUS
  codec.setOutputGain(127-9); // -9dB
#else
  codec.setOutputGain(127); // 0dB
#endif
  codec.bypass(false);
#endif /* USE_CODEC */

  program.startManager();

#ifdef OWL_MAGUS
  extern SPI_HandleTypeDef hspi5;
  // LEDs
  TLC5946_init(&hspi5);
  // TLC5946_setRGB_DC(63, 19, 60); // TODO: balance levels
  TLC5946_setRGB_DC(0xaa, 0xaa, 0xaa);
  TLC5946_setAll(0x1f, 0x1f, 0x1f);
  // Start LED Driver PWM
  extern TIM_HandleTypeDef htim3;
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  // HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);
  extern TIM_HandleTypeDef htim1;
  HAL_TIM_Base_Start_IT(&htim1);
  HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1);
  TLC5946_Refresh_DC();
  // Encoders
  Encoders_init(&hspi5);
  // Pixi
  MAX11300_init(&hspi5);
  MAX11300_setDeviceControl(DCR_RESET);
  HAL_Delay(1000);
  MAX11300_setDeviceControl(DCR_DACCTL_ImmUpdate|DCR_DACREF_Int|DCR_ADCCTL_ContSweep /* |DCR_ADCCONV_200ksps|DCR_BRST_Contextual*/);
  for(int i=0; i<20; ++i){
    setPortMode(i, PORT_UNI_INPUT);
    MAX11300_setDACValue(i, 0);
    updateMAX11300 = true;
  }
#endif /* OWL_MAGUS */

#ifdef OWL_EFFECTSBOX
  extern TIM_HandleTypeDef htim11;
  HAL_TIM_Base_Start(&htim11);
  HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1);
	
  extern TIM_HandleTypeDef htim1;
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
#endif /* OWL_EFFECTSBOX */

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
  xFrequency = 14 / portTICK_PERIOD_MS; // 20mS = 50Hz refresh rate
#ifdef OWL_PRISM
  xFrequency = 20 / portTICK_PERIOD_MS;
#endif

#ifdef USE_DIGITALBUS
  bus_setup();
#endif /* USE_DIGITALBUS */
}

#ifdef OWL_MAGUS
extern "C" {
  void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim){
    HAL_GPIO_TogglePin(TLC_BLANK_GPIO_Port, TLC_BLANK_Pin);
  }
}

void setLed(uint8_t led, uint32_t rgb){
  // rgb should be a 3x 10 bit value
  TLC5946_setRGB(led+1, ((rgb>>20)&0x3ff)<<2, ((rgb>>10)&0x3ff)<<2, ((rgb>>00)&0x3ff)<<2);
}
#endif

#define LED_RED   (0x3ff<<20)
#define LED_GREEN (0x3ff<<10)
#define LED_BLUE  (0x3ff<<00)

#ifdef USE_DIGITALBUS
int busstatus;
#endif

void loop(void){
#ifdef USE_DIGITALBUS
  busstatus = bus_status();
#endif
#ifdef USE_SCREEN
  graphics.draw();
  graphics.display();
#endif /* USE_SCREEN */
#ifdef OLED_DMA
  // When using OLED_DMA this must delay for a minimum amount to allow screen to update
  vTaskDelay(xFrequency);
#else
  vTaskDelayUntil(&xLastWakeTime, xFrequency);
#endif  
  midi.push();

#ifdef OWL_MAGUS
  if(updateMAX11300){
    for(int i=0; i<20; ++i){
      uint16_t mode;
      switch(portMode[i]){
      case PORT_UNI_OUTPUT:
	mode = PCR_Range_DAC_0_P10|PCR_Mode_DAC;
	break;
      case PORT_UNI_INPUT:
      default:
	mode = PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT;
	break;
      }
      MAX11300_setPortMode(i+1, mode);
    }
    updateMAX11300 = false;
  }
  TLC5946_Refresh_GS();
  Encoders_readAll();
  // graphics.params.updateEncoders((int16_t*)rgENC_Values, 7);
  graphics.params.updateEncoders(Encoders_get(), 7);
  MAX11300_bulkreadADC();
  for(int i=0; i<16; ++i){
    if(getPortMode(i) == PORT_UNI_INPUT){
      graphics.params.updateValue(i, MAX11300_getADCValue(i+1));
      uint16_t val = graphics.params.parameters[i]>>2;
      setLed(i, ledstatus ^ rainbowinputs[val&0x3ff]);
    }else{
      // DACs
    // TODO: store values set from patch somewhere and multiply with user[] value for outputs
    // graphics.params.updateOutput(i, getOutputValue(i));
      MAX11300_setDACValue(i+1, graphics.params.parameters[i]);
      graphics.params.updateValue(i, 0);
      uint16_t val = graphics.params.parameters[i]>>2;
      setLed(i, ledstatus ^ rainbowoutputs[val&0x3ff]);
    }
  }
  for(int i=16; i<NOF_PARAMETERS; ++i)
    if(getPortMode(i) == PORT_UNI_INPUT)
      graphics.params.updateValue(i, MAX11300_getADCValue(i+1));
    else
      graphics.params.updateValue(i, 0);
  MAX11300_bulkwriteDAC();
#endif /* OWL_MAGUS */

#ifdef OWL_PRISM
  int16_t encoders[2] = { getEncoderValue(0), getEncoderValue(1) };
  graphics.params.updateEncoders(encoders, 2);
  for(int i=0; i<2; ++i)
    graphics.params.updateValue(i, getAnalogValue(i));
  for(int i=2; i<NOF_PARAMETERS; ++i)
    graphics.params.updateValue(i, 0);
#endif /* OWL_PRISM */

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
  // incoming data from USB device interface
  void midi_device_rx(uint8_t *buffer, uint32_t length){
    for(uint16_t i=0; i<length; i+=4){
      if(!mididevice.readMidiFrame(buffer+i))
	mididevice.reset();
#ifdef USE_DIGITALBUS
      else
	bus_tx_frame(buffer+i);
#endif /* USE_DIGITALBUS */
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
  int16_t getEncoderValue(uint8_t encoder){
    extern TIM_HandleTypeDef ENCODER_TIM1;
    extern TIM_HandleTypeDef ENCODER_TIM2;
    if(encoder == 0)
      return __HAL_TIM_GET_COUNTER(&ENCODER_TIM1);
    else // if(encoder == 1)
      return __HAL_TIM_GET_COUNTER(&ENCODER_TIM2);
  }

  void encoderReset(uint8_t encoder, int16_t value){
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
#endif /* USE_ENCODERS */

}
