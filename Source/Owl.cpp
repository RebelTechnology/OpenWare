#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "Owl.h"
#ifdef USE_CODEC
#include "Codec.h"
#endif
#include "MidiReceiver.h"
#include "MidiStreamReader.h"
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
#ifdef USE_SCREEN
#include "Graphics.h"
#endif

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

Owl owl;
uint32_t ledstatus;
MidiController midi_tx;
MidiReceiver midi_rx;
ApplicationSettings settings;
#ifdef USE_CODEC
Codec codec;
#endif
#ifdef USE_ADC
uint16_t adc_values[NOF_ADC_VALUES];
#endif
#ifdef USE_DAC
extern DAC_HandleTypeDef hdac;
#endif

int16_t getAnalogValue(uint8_t ch){
#ifdef USE_ADC
  if(ch < NOF_ADC_VALUES)
    return adc_values[ch];
#endif
  return 0;
}

__weak void setAnalogValue(uint8_t ch, int16_t value){
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

__weak void setGateValue(uint8_t ch, int16_t value){
#if defined OWL_PEDAL || defined OWL_MODULAR
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
  midi_rx.setInputChannel(channel);
}

void midiSetOutputChannel(int8_t channel){
  settings.midi_output_channel = channel;
  midi_tx.setOutputChannel(channel);
}

__weak void setLed(uint8_t led, uint32_t rgb){
  // rgb should be a 3x 10 bit value
#if defined OWL_TESSERACT
  TIM2->CCR1 = 1023 - ((rgb>>20)&0x3ff);
  TIM3->CCR4 = 1023 - ((rgb>>10)&0x3ff);
  TIM2->CCR2 = 1023 - ((rgb>>00)&0x3ff);
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

__weak void initLed(){
  // Initialise RGB LED PWM timers
#if defined OWL_TESSERACT
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


extern "C" {

void HAL_GPIO_EXTI_Callback(uint16_t pin){
  pinChanged(pin);
}
}

__weak void pinChanged(uint16_t pin){
  switch(pin){
#ifdef OWL_BIOSIGNALS
  case ADC_DRDY_Pin: {
    ads_drdy();
    break;
  }
#ifdef USE_KX122
  case ACC_INT1_Pin: {
    kx122_drdy();
    break;
  }
#endif
#endif
#ifdef PUSHBUTTON_Pin
  case PUSHBUTTON_Pin: {
    bool isSet = !(PUSHBUTTON_GPIO_Port->IDR & PUSHBUTTON_Pin);
    setButtonValue(PUSHBUTTON, isSet);
    midi_tx.sendCc(PUSHBUTTON, isSet ? 127 : 0);
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

static TickType_t xLastWakeTime;
static TickType_t xFrequency;

void Owl::setup(void){
#ifdef USE_IWDG
#ifdef STM32H743xx
  IWDG1->KR = 0xCCCC; // Enable IWDG and turn on LSI
  IWDG1->KR = 0x5555; // ensure watchdog register write is allowed
  IWDG1->PR = 0x05;   // prescaler 128
  IWDG1->RLR = 0x753; // reload 8 seconds
#else
  IWDG->KR = 0xCCCC; // Enable IWDG and turn on LSI
  IWDG->KR = 0x5555; // ensure watchdog register write is allowed
  IWDG->PR = 0x05;   // prescaler 128
  IWDG->RLR = 0x753; // reload 8 seconds
#endif
#endif
  initLed();
  setLed(0, NO_COLOUR);
#ifdef USE_BKPSRAM
  HAL_PWR_EnableBkUpAccess();
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

#ifdef USE_DAC
  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
  HAL_DAC_Start(&hdac, DAC_CHANNEL_2);
  setAnalogValue(PARAMETER_F, 0);
  setAnalogValue(PARAMETER_G, 0);
#endif

#if defined USE_ADC && !defined OWL_WAVETABLE
  extern ADC_HandleTypeDef ADC_PERIPH;
  HAL_StatusTypeDef ret = HAL_ADC_Start_DMA(&ADC_PERIPH, (uint32_t*)adc_values, NOF_ADC_VALUES);
  if(ret != HAL_OK)
    error(CONFIG_ERROR, "ADC Start failed");
#endif /* USE_ADC */

  midi_rx.init();
  midiSetInputChannel(settings.midi_input_channel);
  midiSetOutputChannel(settings.midi_output_channel);

  xLastWakeTime = xTaskGetTickCount();
  xFrequency = MAIN_LOOP_SLEEP_MS / portTICK_PERIOD_MS; // 20mS = 50Hz refresh rate

#ifdef USE_DIGITALBUS
  bus_setup();
  bus_set_input_channel(settings.midi_input_channel);
#endif /* USE_DIGITALBUS */
}

__weak void setup(){
#ifdef OWL_BIOSIGNALS
  ble_init();
  setLed(1, NO_COLOUR);
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

#ifdef USE_ENCODERS
  __HAL_TIM_SET_COUNTER(&ENCODER_TIM1, INT16_MAX/2);
  __HAL_TIM_SET_COUNTER(&ENCODER_TIM2, INT16_MAX/2);
  HAL_TIM_Encoder_Start_IT(&ENCODER_TIM1, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start_IT(&ENCODER_TIM2, TIM_CHANNEL_ALL);
#endif /* OWL_PLAYERF7 */

#ifdef OWL_WAVETABLE
  extern ADC_HandleTypeDef hadc1;
  extern ADC_HandleTypeDef hadc3;
  HAL_StatusTypeDef ret = HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_values, 4);
  if(ret != HAL_OK)
    error(CONFIG_ERROR, "ADC1 Start failed");
  ret = HAL_ADC_Start_DMA(&hadc3, (uint32_t*)(adc_values+4), 4);
  if(ret != HAL_OK)
    error(CONFIG_ERROR, "ADC3 Start failed");
#endif
  owl.setup();
}

#ifdef USE_DIGITALBUS
int busstatus;
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

OperationMode Owl::getOperationMode(){
  return operationMode;
}

void Owl::setOperationMode(OperationMode mode){
  setLed(0, YELLOW_COLOUR);
  operationMode = mode;
}

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

void owl_mode_button(void){
  static int patchselect = 0;
  static int gainselect = 0;
  switch(owl.getOperationMode()){
  case STARTUP_MODE:
    owl.setOperationMode(RUN_MODE);
    break;
  case LOAD_MODE:
    setLed(0, getParameterValue(PARAMETER_A)*BLUE_COLOUR/4095);
    break;
  case RUN_MODE:
    if(isModeButtonPressed()){
      patchselect = getPatchSelectionValue();
      gainselect = getGainSelectionValue();
      owl.setOperationMode(CONFIGURE_MODE);
      setLed(0, NO_COLOUR);
    }else if(getErrorStatus() != NO_ERROR){
      owl.setOperationMode(ERROR_MODE);
    }else{
#ifdef USE_RGB_LED
      updateLed();
#endif
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
      owl.setOperationMode(RUN_MODE);
    }
    break;
  case ERROR_MODE:
    setLed(0, RED_COLOUR);
    if(isModeButtonPressed())
      program.resetProgram(false); // runAudioTask() changes to RUN_MODE
    break;
  }
}
#endif /* USE_MODE_BUTTON */

__weak void loop(void){
#ifdef USE_MODE_BUTTON
  owl_mode_button();
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
#if defined USB_HOST_PWR_FAULT_Pin && defined USB_HOST_PWR_EN_Pin
  if(HAL_GPIO_ReadPin(USB_HOST_PWR_FAULT_GPIO_Port, USB_HOST_PWR_FAULT_Pin) == GPIO_PIN_RESET){
    if(HAL_GPIO_ReadPin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin) == GPIO_PIN_SET){
      HAL_GPIO_WritePin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin, GPIO_PIN_RESET);
      error(USB_ERROR, "USBH PWR Fault");
    }
  }else{
    MX_USB_HOST_Process();
  }
#else
  MX_USB_HOST_Process();
#endif
#endif

  owl.loop();

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
}

void Owl::loop(){
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
  midi_tx.transmit();
#ifdef USE_IWDG
#ifdef STM32H743xx
  IWDG1->KR = 0xaaaa; // reset the watchdog timer (if enabled)
#else
  IWDG->KR = 0xaaaa; // reset the watchdog timer (if enabled)
#endif
#endif
  if(backgroundTask != NULL)
    backgroundTask->loop();
}

void Owl::setBackgroundTask(BackgroundTask* bt){
  if(backgroundTask != NULL)
    backgroundTask->end();
  backgroundTask = bt;
  if(backgroundTask != NULL)
    backgroundTask->begin();
}

extern "C"{

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
#if defined USB_HOST_PWR_EN_GPIO_Port && defined USB_HOST_PWR_EN_Pin
  HAL_GPIO_WritePin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin, GPIO_PIN_RESET);
#endif
#endif
// #ifdef USE_USBD_MIDI
//   extern USBD_HandleTypeDef USBD_HANDLE;
//   USBD_DeInit(&USBD_HANDLE);
// #endif
  *OWLBOOT_MAGIC_ADDRESS = OWLBOOT_MAGIC_NUMBER;
#ifdef USE_BKPSRAM
  extern RTC_HandleTypeDef hrtc;
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0);
#endif
  /* Disable all interrupts */
#ifdef STM32H743xx
  RCC->CIER = 0x00000000;
#else
  RCC->CIR = 0x00000000;
#endif
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
  /* Disable all interrupts */
#ifdef STM32H743xx
  RCC->CIER = 0x00000000;
#else
  RCC->CIR = 0x00000000;
#endif
  NVIC_SystemReset();
  /* Shouldn't get here */
  while(1);
}

// called from patch program: Patch::sendMidi(MidiMessage)
void midi_send(uint8_t port, uint8_t status, uint8_t d1, uint8_t d2){
  midi_tx.send(MidiMessage(port, status, d1, d2));
}

const char* getFirmwareVersion(){ 
  return (const char*)(HARDWARE_VERSION " " FIRMWARE_VERSION) ;
}
