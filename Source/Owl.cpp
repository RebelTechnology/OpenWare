#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "Owl.h"
#ifdef USE_CODEC
#include "Codec.h"
#endif
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

#ifdef OWL_BIOSIGNALS
#include "ads.h"
#ifdef USE_KX122
#include "kx122.h"
#endif
#include "ble_midi.h"
#endif

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

#ifdef USE_CODEC
Codec codec;
#endif
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
  if(ch == BUTTON_F || ch == PUSHBUTTON)
    HAL_GPIO_WritePin(TRIG_OUT_GPIO_Port, TRIG_OUT_Pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
#elif defined OWL_PEDAL || defined OWL_MODULAR
  if(ch == PUSHBUTTON){
    HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
#ifdef OWL_MODULAR
    HAL_GPIO_WritePin(PUSH_GATE_OUT_GPIO_Port, PUSH_GATE_OUT_Pin, value ? GPIO_PIN_RESET :  GPIO_PIN_SET);
#endif
  }else if(ch == GREEN_BUTTON){
    setLed(0, GREEN_COLOUR);
  }else if(ch == RED_BUTTON){
    setLed(0, RED_COLOUR);
  }
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

void setLed(uint8_t led, uint32_t rgb){
  // rgb should be a 3x 10 bit value
#if defined OWL_TESSERACT
  TIM2->CCR1 = 1023 - ((rgb>>20)&0x3ff);
  TIM3->CCR4 = 1023 - ((rgb>>10)&0x3ff);
  TIM2->CCR2 = 1023 - ((rgb>>00)&0x3ff);
#elif defined OWL_WIZARD
  TIM5->CCR2 = 1023 - ((rgb>>20)&0x3ff);
  TIM4->CCR3 = 1023 - ((rgb>>10)&0x3ff);
  TIM2->CCR1 = 1023 - ((rgb>>00)&0x3ff);
#elif defined OWL_ALCHEMIST
  TIM2->CCR1 = 1023 - ((rgb>>20)&0x3ff);
  TIM2->CCR2 = 1023 - ((rgb>>10)&0x3ff);
  TIM3->CCR4 = 1023 - ((rgb>>00)&0x3ff);
#elif defined OWL_PEDAL || defined OWL_MODULAR
  switch(rgb){
  case RED_COLOUR:
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
    break;
  case GREEN_COLOUR:
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
    break;
  case YELLOW_COLOUR:
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
    break;
  case NO_COLOUR:
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
    break;
  }
#elif defined OWL_MAGUS
  TLC5946_setRGB(led+1, ((rgb>>20)&0x3ff)<<2, ((rgb>>10)&0x3ff)<<2, ((rgb>>00)&0x3ff)<<2);
#elif defined OWL_BIOSIGNALS
  if(led == 0){
#ifdef USE_LED_PWM
    rgb &= COLOUR_LEVEL5; // turn down intensity
    TIM1->CCR1 = 1023 - ((rgb>>20)&0x3ff); // red
    TIM1->CCR3 = 1023 - ((rgb>>10)&0x3ff); // green
    TIM1->CCR2 = 1023 - ((rgb>>00)&0x3ff); // blue
#else
    switch(rgb){ // sinking current
    case RED_COLOUR:
      HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
      break;
    case GREEN_COLOUR:
      HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
      break;
    case YELLOW_COLOUR:
      HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
      break;
    case NO_COLOUR:
      HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
      break;
    }
#endif      
  }else if(led == 1){
    if(rgb == NO_COLOUR)
      HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
    else
      HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
  }
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
#elif defined OWL_BIOSIGNALS
#ifdef USE_LED_PWM
  extern TIM_HandleTypeDef htim1;
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
#else
  /*Configure GPIO pin : LED_GREEN_Pin, LED_RED_Pin */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = LED_GREEN_Pin | LED_RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GREEN_GPIO_Port, &GPIO_InitStruct);
#endif
#endif
}

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
#ifdef OWL_BIOSIGNALS
  case ADC_DRDY_Pin: {
    ads_drdy();
  }
#ifdef USE_KX122
  case ACC_INT1_Pin: {
    kx122_drdy();
  }
#endif
#endif
#ifdef PUSHBUTTON_Pin
  case PUSHBUTTON_Pin: {
    bool isSet = !(PUSHBUTTON_GPIO_Port->IDR & PUSHBUTTON_Pin);
    setButtonValue(PUSHBUTTON, isSet);
    midi.sendCc(PUSHBUTTON, isSet ? 127 : 0);
#if defined OWL_PEDAL || defined OWL_MODULAR
    setLed(0, isSet ? RED_COLOUR : GREEN_COLOUR);
#endif
    break;
  }
#endif
#ifdef OWL_PEDAL
  case BYPASS_Pin: {
    bool isSet = !(BYPASS_GPIO_Port->IDR & BYPASS_Pin);
    setLed(0, isSet ? NO_COLOUR : GREEN_COLOUR);
    break;
  }
#endif
#ifdef OWL_MODULAR
  case PUSH_GATE_IN_Pin: {
    bool isSet = !(PUSH_GATE_IN_GPIO_Port->IDR & PUSH_GATE_IN_Pin);
    setButtonValue(PUSHBUTTON, isSet);
    setLed(0, isSet ? RED_COLOUR : GREEN_COLOUR);
    break;
  }
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
  // todo: select range automatically based on output value
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

#ifdef OWL_BIOSIGNALS
  ble_init();
#ifdef USE_LED
  initLed();
  setLed(0, YELLOW_COLOUR);
#endif
  setLed(1, NO_COLOUR);
#endif

#ifdef USE_BKPSRAM
  // __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();
  // __HAL_RCC_RTC_CLKPRESCALER(RCC_RTCCLKSOURCE_LSI);
  // __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSI);
  // __HAL_RCC_RTC_ENABLE();
#endif
  
#ifdef OWL_PEDAL
  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  // if (HAL_GetREVID() == 0x1001)
  // {
  //   /* Enable the Flash prefetch */
  //   __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  // }
  // enable expression pedal reference voltage
  HAL_GPIO_WritePin(EXPRESSION_PEDAL_TIP_GPIO_Port, EXPRESSION_PEDAL_TIP_Pin, GPIO_PIN_SET);
  // todo: on OWL Modular the ADC should read Exp pin PA2 instead of PA3
#endif

#ifdef OWL_WIZARD
  HAL_GPIO_WritePin(TRIG_OUT_GPIO_Port, TRIG_OUT_Pin, GPIO_PIN_RESET); // Trigger out off
#endif

#ifdef USE_SCREEN
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); // OLED off
#endif
#ifdef OWL_MAGUS
  HAL_GPIO_WritePin(TLC_BLANK_GPIO_Port, TLC_BLANK_Pin, GPIO_PIN_SET); // LEDs off
  HAL_GPIO_WritePin(ENC_NRST_GPIO_Port, ENC_NRST_Pin, GPIO_PIN_RESET); // Reset encoders 
#endif /* OWL_MAGUS */

#ifdef USE_DAC
  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
  HAL_DAC_Start(&hdac, DAC_CHANNEL_2);
  setAnalogValue(PARAMETER_A, 0);
  setAnalogValue(PARAMETER_B, 0);
#endif
  
  ledstatus = 0;
  storage.init();
  registry.init();
  settings.init(); // settings need the registry to be initialised first
#ifdef USE_CODEC
  codec.init();
  codec.set(0);
  codec.bypass(false);
  codec.setInputGain(settings.audio_input_gain);
  codec.setOutputGain(settings.audio_output_gain);
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

    HAL_GPIO_WritePin(TLC_BLANK_GPIO_Port, TLC_BLANK_Pin, GPIO_PIN_RESET);

    TLC5946_Refresh_DC();
    TLC5946_Refresh_GS();
    HAL_Delay(100);

    // Start BLANK PWM
    extern TIM_HandleTypeDef htim2;
    HAL_TIM_Base_Start(&htim2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

    // Start LED Driver PWM : GSCLK
    extern TIM_HandleTypeDef htim3;
    HAL_TIM_Base_Start(&htim3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  }
  {
    // Encoders
    extern SPI_HandleTypeDef ENCODERS_SPI;
    Encoders_init(&ENCODERS_SPI);
    Encoders_readAll();
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

#ifdef OWL_WAVETABLE
  extern ADC_HandleTypeDef hadc1;
  extern ADC_HandleTypeDef hadc3;
  HAL_StatusTypeDef ret = HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_values, 4);
  if(ret != HAL_OK)
    error(CONFIG_ERROR, "ADC1 Start failed");
  ret = HAL_ADC_Start_DMA(&hadc3, (uint32_t*)(adc_values+4), 4);
  if(ret != HAL_OK)
    error(CONFIG_ERROR, "ADC3 Start failed");
#else
#ifdef USE_ADC
  extern ADC_HandleTypeDef ADC_PERIPH;
  HAL_StatusTypeDef ret = HAL_ADC_Start_DMA(&ADC_PERIPH, (uint32_t*)adc_values, NOF_ADC_VALUES);
  if(ret != HAL_OK)
    error(CONFIG_ERROR, "ADC Start failed");
#endif /* USE_ADC */
#endif

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

#ifdef USE_DIGITALBUS
int busstatus;
#endif

#ifdef USE_MODE_BUTTON
bool isModeButtonPressed(){
  return HAL_GPIO_ReadPin(MODE_BUTTON_PORT, MODE_BUTTON_PIN) == GPIO_PIN_RESET;
}
int getGainSelectionValue(){
  return adc_values[MODE_BUTTON_GAIN]*128*4/4096;
}
int getPatchSelectionValue(){
  return adc_values[MODE_BUTTON_PATCH]*(registry.getNumberOfPatches()-1)*4/4095;
}
#endif

#ifdef USE_RGB_LED
void updateLed(){
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
  setLed(0, ledstatus | rainbow[colour]);
  // setLed(4095-adc_values[0], 4095-adc_values[1], 4095-adc_values[2]);
}
#endif /*USE_RGB_LED */

static volatile OperationMode operationMode = STARTUP_MODE;
void setOperationMode(OperationMode mode){
  setLed(0, YELLOW_COLOUR);
  operationMode = mode;
}

void loop(void){
#ifdef USE_MODE_BUTTON
  static int patchselect = 0;
  static int gainselect = 0;
  switch(operationMode){
  case STARTUP_MODE:
    operationMode = RUN_MODE;
    break;
  case LOAD_MODE:
    setLed(0, getParameterValue(PARAMETER_A)*BLUE_COLOUR/4095);
    break;
  case RUN_MODE:
    if(isModeButtonPressed()){
      patchselect = getPatchSelectionValue();
      gainselect = getGainSelectionValue();
      operationMode = CONFIGURE_MODE;
      setLed(0, NO_COLOUR);
    }else if(getErrorStatus() != NO_ERROR){
      operationMode = ERROR_MODE;
    }else{
      updateLed();
    }
    break;
  case CONFIGURE_MODE:
    if(isModeButtonPressed()){
      int value = getPatchSelectionValue();
      if(abs(patchselect - value) > 1){
	patchselect = value;
	value = max(1, min((int)registry.getNumberOfPatches()-1, value/4 + 1));
	if(program.getProgramIndex() != value){
	  program.loadProgram(value);
	  program.resetProgram(false);
	  setLed(0, value & 0x01 ? BLUE_COLOUR : GREEN_COLOUR);
	}
      }
      value = getGainSelectionValue();
      if(abs(gainselect - value) > 2){
	gainselect = value;
	value = max(0, min(127, value/4));
	codec.setOutputGain(value);    
	setLed(0, value & 0x01 ? YELLOW_COLOUR : CYAN_COLOUR);
      }
    }else{
      operationMode = RUN_MODE;
    }
    break;
  case ERROR_MODE:
    setLed(0, RED_COLOUR);
    if(isModeButtonPressed())
      program.resetProgram(false); // runAudioTask() changes to RUN_MODE
    break;
  }
#endif /* USE_MODE_BUTTON */

#ifdef FASCINATION_MACHINE
  static int output_gain = 0;
  int gain = adc_values[ADC_D]*255/4095;
  if(abs(gain - output_gain) > 1){
    output_gain = gain;
    codec.setOutputGain(output_gain/2);    
  }
  static int patch_index = 0;
  int patch = adc_values[ADC_E]*10/4095;
  if(abs(patch - patch_index) > 1){
    patch_index = patch;
    patch = patch/2 + 1;
    if(program.getProgramIndex() != patch){
      program.loadProgram(patch);
      program.resetProgram(false);
    }
  }
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

  IWDG->KR = 0xaaaa; // reset the watchdog timer (if enabled)
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

void jump_to_bootloader(void){
#ifdef USE_USB_HOST
  HAL_GPIO_WritePin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin, GPIO_PIN_RESET);
#endif
  *OWLBOOT_MAGIC_ADDRESS = OWLBOOT_MAGIC_NUMBER;
  /* Disable all interrupts */
  RCC->CIR = 0x00000000;
  NVIC_SystemReset();
  /* Shouldn't get here */
  while(1);
}

void device_reset(){
  *OWLBOOT_MAGIC_ADDRESS = 0;
#ifdef USE_BKPSRAM
  extern RTC_HandleTypeDef hrtc;
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0);
#endif
  NVIC_SystemReset();
  /* Shouldn't get here */
  while(1);
}

// called from patch program: Patch::sendMidi(MidiMessage)
void midi_send(uint8_t port, uint8_t status, uint8_t d1, uint8_t d2){
  uint8_t data[] = {port, status, d1, d2};
  midi.write(data, 4);
#ifdef USE_DIGITALBUS
  bus_tx_frame(data);
#endif /* USE_DIGITALBUS */
}

const char* getFirmwareVersion(){ 
  return (const char*)(HARDWARE_VERSION " " FIRMWARE_VERSION) ;
}
