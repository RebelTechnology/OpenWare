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
#include "FlashStorage.h"
#include "PatchRegistry.h"

#ifdef OWL_MAGUS
#include "purple-blue-cyan.h"
#include "orange-red-pink.h"
#include "HAL_TLC5946.h"
#include "HAL_MAX11300.h"
// #include "HAL_OLED.h"
#include "HAL_Encoders.h"
#define TLC5940_RED_DC 0x55
#define TLC5940_GREEN_DC 0x55
#define TLC5940_BLUE_DC 0x55
#endif

#ifdef USE_SCREEN
#include "Graphics.h"
Graphics graphics;
#endif /* USE_SCREEN */

#if defined USE_RGB_LED
#include "rainbow.h"
#endif /* USE_RGB_LED */

#ifdef USE_USB_HOST
#include "usbh_core.h"
#include "usbh_midi.h"
extern "C"{
void MX_USB_HOST_Process(void);
}
#endif /* USE_USB_HOST */

#ifdef USE_DIGITALBUS
#include "bus.h"
#endif /* USE_DIGITALBUS */

#ifdef USE_ENCODERS
extern TIM_HandleTypeDef ENCODER_TIM1;
extern TIM_HandleTypeDef ENCODER_TIM2;
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

#ifdef USE_ADC
uint16_t adc_values[NOF_ADC_VALUES];
#endif
#ifdef USE_DAC
extern DAC_HandleTypeDef hdac;
#endif
uint32_t ledstatus;

int16_t getAnalogValue(uint8_t ch){
#ifdef USE_ADC
  if(ch < NOF_ADC_VALUES)
    return adc_values[ch];
  else
#endif
    return 0;
}

void setAnalogValue(uint8_t ch, int16_t value){
#ifdef USE_DAC
  switch(ch){
  case PARAMETER_F:
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, __USAT(value, 12));
    break;
  case PARAMETER_G:
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, __USAT(value, 12));
    break;
  }
#endif
}

void setGateValue(uint8_t ch, int16_t value){
#ifdef OWL_WIZARD
  if(ch == BUTTON_F)
    HAL_GPIO_WritePin(TRIG_OUT_GPIO_Port, TRIG_OUT_Pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
#endif
}

void midiSetInputChannel(int8_t channel){
  settings.midi_input_channel = channel;
  mididevice.setInputChannel(channel);
#ifdef USE_USB_HOST
  midihost.setInputChannel(channel);
#endif
#ifdef USE_DIGITALBUS
  bus_set_input_channel(channel);
#endif
}

void midiSetOutputChannel(int8_t channel){
  settings.midi_output_channel = channel;
  midi.setOutputChannel(channel);
}

#ifdef USE_RGB_LED
void setLed(uint8_t led, uint32_t rgb){
  // rgb should be a 3x 10 bit value
#if defined OWL_TESSERACT || defined OWL_ALCHEMIST
  TIM2->CCR1 = 1023 - ((rgb>>20)&0x3ff);
  TIM3->CCR4 = 1023 - ((rgb>>10)&0x3ff);
  TIM2->CCR2 = 1023 - ((rgb>>00)&0x3ff);
#elif defined OWL_WIZARD
  TIM2->CCR1 = 1023 - ((rgb>>20)&0x3ff);
  TIM5->CCR2 = 1023 - ((rgb>>10)&0x3ff);
  TIM4->CCR3 = 1023 - ((rgb>>00)&0x3ff);
#endif
}

void initLed(){
  // Initialise RGB LED PWM timers
#if defined OWL_TESSERACT || defined OWL_ALCHEMIST
  extern TIM_HandleTypeDef htim2;
  extern TIM_HandleTypeDef htim3;
  // Red
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  // Green
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  // Blue
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
#elif defined OWL_WIZARD
  extern TIM_HandleTypeDef htim2;
  extern TIM_HandleTypeDef htim5;
  extern TIM_HandleTypeDef htim4;
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_Base_Start(&htim5);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
  HAL_TIM_Base_Start(&htim4);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
#endif
}
#endif /* USE_RGB_LED */

#ifdef OWL_EFFECTSBOX
// static uint8_t buttonstate = 0;
// #define SW1_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW1_BTN_GPIO_Port,  SW1_BTN_Pin))
// #define SW2_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW2_BTN_GPIO_Port,  SW2_BTN_Pin))
// #define SW3_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW3_BTN_GPIO_Port,  SW3_BTN_Pin))
// #define SW4_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW4_BTN_GPIO_Port,  SW4_BTN_Pin))
// #define SW5_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW5_BTN_GPIO_Port,  SW5_BTN_Pin))
// #define SW6_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW6_BTN_GPIO_Port,  SW6_BTN_Pin))
// #define SW7_Read()		(1-(uint8_t)HAL_GPIO_ReadPin(SW7_BTN_GPIO_Port,  SW7_BTN_Pin))
// #define TSW1_Read()		(1-HAL_GPIO_ReadPin(TSW1_A_GPIO_Port,  TSW1_A_Pin)) | (1-HAL_GPIO_ReadPin(TSW1_B_GPIO_Port,  TSW1_B_Pin))<<1
// #define TSW2_Read()		(1-HAL_GPIO_ReadPin(TSW2_A_GPIO_Port,  TSW2_A_Pin)) | (1-HAL_GPIO_ReadPin(TSW2_B_GPIO_Port,  TSW2_B_Pin))<<1
typedef enum {
	YELLOW, RED, NONE
} LEDcolour;
void setLED(uint8_t led, LEDcolour col){
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_TypeDef* GPIOx; 
  uint16_t GPIO_Pin;
  uint8_t LED_Colour;
	
  // Get switch pin and port number
  switch(led){
  case 0: GPIOx = SW1_LED_GPIO_Port;	GPIO_Pin = SW1_LED_Pin; break;
  case 1: GPIOx = SW2_LED_GPIO_Port;	GPIO_Pin = SW2_LED_Pin; break;
  case 2: GPIOx = SW3_LED_GPIO_Port;	GPIO_Pin = SW3_LED_Pin; break;
  case 3: GPIOx = SW4_LED_GPIO_Port;	GPIO_Pin = SW4_LED_Pin; break;
  case 4: GPIOx = SW5_LED_GPIO_Port;	GPIO_Pin = SW5_LED_Pin; break;
  case 5: GPIOx = SW6_LED_GPIO_Port;	GPIO_Pin = SW6_LED_Pin; break;
  case 6: GPIOx = SW7_LED_GPIO_Port;	GPIO_Pin = SW7_LED_Pin; break;
  }
	
  // Set pin number and direction
  GPIO_InitStruct.Pin = GPIO_Pin;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	
  // Set Output direction and LED colour
  switch (col){
  case YELLOW:	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; LED_Colour = YELLOW; break;
  case RED: 	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; LED_Colour = RED; break;
  case NONE: 	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;	LED_Colour = 0; break;
  }
  // Update Pin	
  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOx,  GPIO_Pin,  (GPIO_PinState)LED_Colour);
}

void updateProgramSelector(uint8_t button, uint8_t led, uint8_t patch, bool value){  
  setButtonValue(button, value);
  if(value){
    setLED(led, RED);
  }else{
    if(program.getProgramIndex() != patch){
      program.loadProgram(patch);
      for(int i=0; i<6; ++i)
	setLED(i, NONE);
    }
    setLED(led, YELLOW);
  }
}
#endif /* OWL_EFFECTSBOX */

extern "C" {
  
void HAL_GPIO_EXTI_Callback(uint16_t pin){
  switch(pin){
#ifdef PUSHBUTTON_Pin
  case PUSHBUTTON_Pin:
    setButtonValue(PUSHBUTTON, !(PUSHBUTTON_GPIO_Port->IDR & PUSHBUTTON_Pin));
    midi.sendCc(PUSHBUTTON, getButtonValue(PUSHBUTTON) ? 127 : 0);
    break;
#endif
#ifdef OWL_EFFECTSBOX
  case SW1_BTN_Pin:
    updateProgramSelector(BUTTON_A, 0, 1, !(SW1_BTN_GPIO_Port->IDR & SW1_BTN_Pin));
    break;
  case SW2_BTN_Pin:
    updateProgramSelector(BUTTON_B, 1, 2, !(SW2_BTN_GPIO_Port->IDR & SW2_BTN_Pin));    
    break;
  case SW3_BTN_Pin:
    updateProgramSelector(BUTTON_C, 2, 3, !(SW3_BTN_GPIO_Port->IDR & SW3_BTN_Pin));    
    setLED(2, (LEDcolour)getButtonValue(BUTTON_C));
    break;
  case SW4_BTN_Pin:
    updateProgramSelector(BUTTON_D, 3, 4, !(SW4_BTN_GPIO_Port->IDR & SW4_BTN_Pin));    
    break;
  case SW5_BTN_Pin:
    updateProgramSelector(BUTTON_E, 4, 5, !(SW5_BTN_GPIO_Port->IDR & SW5_BTN_Pin));    
    break;
  case SW6_BTN_Pin:
    updateProgramSelector(BUTTON_F, 5, 6, !(SW6_BTN_GPIO_Port->IDR & SW6_BTN_Pin));    
    break;
  case SW7_BTN_Pin:
    if((SW7_BTN_GPIO_Port->IDR & SW7_BTN_Pin)){
      setButtonValue(PUSHBUTTON, false);
      setLED(6, YELLOW);
    }else{
      setButtonValue(PUSHBUTTON, true);
      setLED(6, RED);
    }
    break;
#endif /* OWL_EFFECTSBOX */
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
#if defined OWL_WIZARD || defined OWL_ALCHEMIST
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
#endif
// #ifdef OWL_WIZARD
//   case SW4_Pin:
//     setButtonValue(BUTTON_D, !(SW4_GPIO_Port->IDR & SW4_Pin));
//     ledstatus ^= 0x3ff003ff;
//     break;
//   case SW5_Pin:
//     setButtonValue(BUTTON_E, !(SW5_GPIO_Port->IDR & SW5_Pin));
//     ledstatus = 0;
//     break;
// #endif
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
}
}

#ifdef OWL_MAGUS
static bool updateMAX11300 = false;
// int16_t dynamicParameterValues[NOF_PARAMETERS];
static uint8_t portMode[20];
void setPortMode(uint8_t index, uint8_t mode){
  if(index < 20){
    if(portMode[index] != mode){
      portMode[index] = mode;
      updateMAX11300 = true;
      // MAX11300_setDACValue(index+1, 0);
    }
  }
}
uint8_t getPortMode(uint8_t index){
  if(index < 20)
    return portMode[index];
  return PORT_UNI_INPUT;
}
#endif

static TickType_t xLastWakeTime;
static TickType_t xFrequency;

void setup(){

#ifdef OWL_WIZARD
  HAL_GPIO_WritePin(TRIG_OUT_GPIO_Port, TRIG_OUT_Pin, GPIO_PIN_SET);
#endif

#ifdef USE_DAC
  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
  HAL_DAC_Start(&hdac, DAC_CHANNEL_2);
  setAnalogValue(PARAMETER_A, 0);
  setAnalogValue(PARAMETER_B, 0);
#endif
  
#ifdef OWL_PEDAL
  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  // if (HAL_GetREVID() == 0x1001)
  // {
  //   /* Enable the Flash prefetch */
  //   __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  // }
#endif

#ifdef USE_SCREEN
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); // OLED off
#endif
#ifdef OWL_MAGUS
  HAL_GPIO_WritePin(TLC_BLANK_GPIO_Port, TLC_BLANK_Pin, GPIO_PIN_RESET); // LEDs off
#endif /* OWL_MAGUS */
  
  ledstatus = 0;
  storage.init();
  registry.init();
  settings.init(); // settings need the registry to be initialised first
#ifdef USE_CODEC
  codec.begin();
  codec.set(0);
  codec.bypass(false);
#endif /* USE_CODEC */

  program.startManager();

#ifdef OWL_MAGUS
  {
    extern SPI_HandleTypeDef TLC5946_SPI;
    // LEDs
    TLC5946_init(&TLC5946_SPI);
    // TLC5946_setRGB_DC(63, 19, 60); // TODO: balance levels
    TLC5946_setRGB_DC(TLC5940_RED_DC, TLC5940_GREEN_DC, TLC5940_BLUE_DC);
    TLC5946_setAll(0x10, 0x10, 0x10);
    // Start LED Driver PWM
    extern TIM_HandleTypeDef htim3;
    HAL_TIM_Base_Start(&htim3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

    extern TIM_HandleTypeDef htim2;
    HAL_TIM_Base_Start(&htim2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

    TLC5946_Refresh_DC();
    TLC5946_Refresh_GS();
  }
  {
    // Encoders
    extern SPI_HandleTypeDef ENCODERS_SPI;
    Encoders_init(&ENCODERS_SPI);
  }
  {
    // Pixi
    extern SPI_HandleTypeDef MAX11300_SPI;
    MAX11300_init(&MAX11300_SPI);
    MAX11300_setDeviceControl(DCR_RESET);
  }
#endif /* OWL_MAGUS */

#ifdef OWL_EFFECTSBOX
  extern TIM_HandleTypeDef htim11;
  HAL_TIM_Base_Start(&htim11);
  HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1); // SW1-6 PWM
  extern TIM_HandleTypeDef htim1;
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); // SW7 PWM
  for(int i=0; i<7; ++i)
    setLED(i, NONE);
#endif /* OWL_EFFECTSBOX */

#ifdef USE_RGB_LED
  initLed();
  setLed(0, NO_COLOUR);
#endif /* USE_RGB_LED */

#ifdef USE_ENCODERS
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

  midiSetInputChannel(settings.midi_input_channel);
  midiSetOutputChannel(settings.midi_output_channel);

  xLastWakeTime = xTaskGetTickCount();
  xFrequency = 20 / portTICK_PERIOD_MS; // 20mS = 50Hz refresh rate

#ifdef USE_DIGITALBUS
  bus_setup();
  bus_set_input_channel(settings.midi_input_channel);
#endif /* USE_DIGITALBUS */

#ifdef USE_USB_HOST
  // enable USB Host power
  HAL_GPIO_WritePin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin, GPIO_PIN_SET);
#endif
}

#ifdef OWL_MAGUS
// extern "C" {
//   void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim){
//     HAL_GPIO_TogglePin(TLC_BLANK_GPIO_Port, TLC_BLANK_Pin);
//   }
// }

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
#ifdef FASCINATION_MACHINE
  static int output_gain = 0;
  int gain = getParameterValue(PARAMETER_D)*255/4095;
  if(abs(gain - output_gain) > 1){
    output_gain = gain;
    codec.setOutputGain(output_gain/2);    
  }
  // float gain = getParameterValue(PARAMETER_D)*127.0/4095;
  // if((gain - (int)gain) < 0.2 && (int)gain != output_gain){
  //   output_gain = gain;
  //   codec.setOutputGain(output_gain);
  // }
  static int patch_index = 0;
  int patch = getParameterValue(PARAMETER_E)*10/4095;
  if(abs(patch - patch_index) > 1){
    patch_index = patch;
    patch = patch/2 + 1;
    // if(program.getProgramIndex() != patch){
      program.loadProgram(patch);
      program.resetProgram(false);
    // }
  }
  // float patch = getParameterValue(PARAMETER_E)*5.0/4095+1;
  // if((patch - (int)patch) < 0.2 && (int)patch != patch_index){
  //   patch_index = patch;
  //   if(program.getProgramIndex() != patch_index){
  //     program.loadProgram(patch_index);
  //     program.resetProgram(false);
  //   }
  // }
#endif

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

#ifdef OWL_EFFECTSBOX
  // uint8_t state =
  //   (SW1_Read() << 0) |
  //   (SW2_Read() << 1) |
  //   (SW3_Read() << 2) |
  //   (SW4_Read() << 3) |
  //   (SW5_Read() << 4) |
  //   (SW6_Read() << 5) |
  //   (SW7_Read() << 6);
  // if(state != buttonstate){
  //   for(int i=0; i<7; ++i){
  //   }
  // }
#endif /* OWL_EFFECTSBOX */

#ifdef OWL_MAGUS
  if(updateMAX11300){
    MAX11300_setDeviceControl(DCR_DACCTL_ImmUpdate|DCR_DACREF_Int|DCR_ADCCTL_ContSweep /* |DCR_ADCCONV_200ksps|DCR_BRST_Contextual*/);
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
      setLed(i, rainbowinputs[val&0x3ff]);
    }else{
      // DACs
    // TODO: store values set from patch somewhere and multiply with user[] value for outputs
    // graphics.params.updateOutput(i, getOutputValue(i));
      // MAX11300_setDACValue(i+1, graphics.params.parameters[i]);
      graphics.params.updateValue(i, 0);
      uint16_t val = graphics.params.parameters[i]>>2;
      setLed(i, rainbowoutputs[val&0x3ff]);
      MAX11300_setDAC(i+1, graphics.params.parameters[i]);
    }
  }
  for(int i=16; i<20; ++i){
    if(getPortMode(i) == PORT_UNI_INPUT){
      graphics.params.updateValue(i, MAX11300_getADCValue(i+1));
    }else{
      graphics.params.updateValue(i, 0);
      MAX11300_setDAC(i+1, graphics.params.parameters[i]);
    }
  }
  // MAX11300_bulkwriteDAC();
#endif /* OWL_MAGUS */

#ifdef OWL_PRISM
  int16_t encoders[NOF_ENCODERS] = {(int16_t)__HAL_TIM_GET_COUNTER(&ENCODER_TIM1),
				    (int16_t)__HAL_TIM_GET_COUNTER(&ENCODER_TIM2) };
  graphics.params.updateEncoders(encoders, 2);
#ifndef OWL_RACK
  for(int i=0; i<NOF_ENCODERS; ++i)
    graphics.params.updateValue(i, getAnalogValue(i)-2048); // update two bipolar cv inputs
  for(int i=2; i<NOF_PARAMETERS; ++i)
    graphics.params.updateValue(i, 0);
#endif
#endif /* OWL_PRISM */

#ifdef OWL_EFFECTSBOX
  int16_t encoders[NOF_ENCODERS] = {(int16_t)__HAL_TIM_GET_COUNTER(&ENCODER_TIM1),
  				    (int16_t)__HAL_TIM_GET_COUNTER(&ENCODER_TIM2) };
  graphics.params.updateEncoders(encoders, 6);
  for(int i=0; i<NOF_ADC_VALUES; ++i)
    graphics.params.updateValue(i, getAnalogValue(i));
  // for(int i=NOF_ADC_VALUES; i<NOF_PARAMETERS; ++i)
  //   graphics.params.updateValue(i, 0);
#endif  

#ifdef USE_RGB_LED
  uint32_t colour =
    (adc_values[ADC_A]>>3)+
    (adc_values[ADC_B]>>3)+
    (adc_values[ADC_C]>>3)+
    (adc_values[ADC_D]>>3);
#ifdef ADC_E
  colour += (adc_values[ADC_E]>>3);
#endif
#ifdef FASCINATION_MACHINE
  extern float audio_envelope;
  colour = colour*(1+audio_envelope);
#endif
  colour &= 0x3ff;
  setLed(0, ledstatus ^ rainbow[colour]);
  // setLed(4095-adc_values[0], 4095-adc_values[1], 4095-adc_values[2]);
#endif /* USE_RGB_LED */
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
    ledstatus ^= 0x3ff003ff;
  }
  void midi_host_rx(uint8_t *buffer, uint32_t length){
    for(uint16_t i=0; i<length; i+=4){
      if(!midihost.readMidiFrame(buffer+i)){
	midihost.reset();
      }else{
	ledstatus ^= 0x000ffc00;
      }
    }
  }
#endif /* USE_USB_HOST */

#if 0 // ifdef USE_ENCODERS
  int16_t getEncoderValue(uint8_t encoder){
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

// __attribute__((naked))
// void reboot(void){
//   const uint32_t bootloaderMagicNumber = 0xDADAB007;
//   /* This address is within the first 64k of memory.
//    * The magic number must match what is in the bootloader */
//   *((unsigned long *)0x2000FFF0) = bootloaderMagicNumber;
//   NVIC_SystemReset();
//   /* Shouldn't get here */
//   while(1);
// }


/* Jump to the bootloader. We set a magic number in memory that our bootloader 
 * startup code looks for. RAM is preserved across system reset, so when it 
 * finds this magic number, it will go to the bootloader code 
 * rather than the application code.
 */
// __attribute__((naked))
void jump_to_bootloader(void){
  /* Disable USB in advance: this will give the computer time to
   * recognise it's been disconnected, so when the system bootloader
   * comes online it will get re-enumerated.
   */
  // usb_deinit();
  /* Blink LEDs */
  // setLed(RED);
  // for(uint8_t i = 0; i < 3; i++) {
  //   volatile uint32_t delayCounter;
  //   for(delayCounter = 0; delayCounter < 2000000; delayCounter++);
  //   setLed(NONE);
  //   for(delayCounter = 0; delayCounter < 2000000; delayCounter++);
  //   setLed(RED);
  // }

  /* Disable all interrupts */
  RCC->CIR = 0x00000000;
  const uint32_t bootloaderMagicNumber = 0xDADAB007;
  /* This address is within the first 64k of memory.
   * The magic number must match what is in the bootloader */
  *((unsigned long *)0x2000FFF0) = bootloaderMagicNumber;
  /* todo:
   * disable USB (host and device)
   * then drive USB pins low for 1 second
   * to allow disconnect
   */
  NVIC_SystemReset();
  /* Shouldn't get here */
  while(1);
}


void midi_send(uint8_t port, uint8_t status, uint8_t d1, uint8_t d2){
  uint8_t data[] = {port, status, d1, d2};
  midi.write(data, 4);
#ifdef USE_DIGITALBUS
  bus_tx_frame(data);
#endif /* USE_DIGITALBUS */
}
