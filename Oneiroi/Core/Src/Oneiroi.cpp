#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "Owl.h"
#include "errorhandlers.h"
#include "message.h"
#include "ProgramManager.h"
#include "OpenWareMidiControl.h"
#include "Pin.h"
#include "ApplicationSettings.h"
#include "Storage.h"
#include "MidiMessage.h"
#include "cmsis_os.h"

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef abs
#define abs(x) ((x) > 0 ? (x) : -(x))
#endif

#define PATCH_RESET_COUNTER (500/MAIN_LOOP_SLEEP_MS)

#define INLEVELGREEN PARAMETER_AF
#define MOD PARAMETER_AG

#define FUNC1 PARAMETER_H
#define FUNC2 PARAMETER_AH

// GPIO
#define FUNC_BUTTON PUSHBUTTON
#define CU_DOWN GREEN_BUTTON
#define CU_UP RED_BUTTON
#define RECORD_BUTTON BUTTON_1
#define RECORD_GATE BUTTON_2
#define RANDOM_BUTTON BUTTON_3
#define RANDOM_GATE BUTTON_4
#define SYNC_GATE BUTTON_5
#define INLEVELRED BUTTON_6
#define PREPOST_SWITCH BUTTON_7
#define SSWT_SWITCH BUTTON_8
#define FUNC_1 BUTTON_9
#define FUNC_2 BUTTON_10

#define RANDOM_AMOUNT PARAMETER_AA
#define FILTER_MODE_SWITCH PARAMETER_AB

// ADC3
#define OSC_DETUNE_CV PARAMETER_A
#define FILTER_CUTOFF_CV PARAMETER_B
#define RESONATOR_HARMONY_CV PARAMETER_C
#define DELAY_TIME_CV PARAMETER_D
#define LOOPER_START_CV PARAMETER_E
#define LOOPER_LENGTH_CV PARAMETER_F
#define LOOPER_SPEED_CV PARAMETER_G

// Muxed
#define REVERB_TONESIZE_CV PARAMETER_AC
#define OSC_VOCT_CV PARAMETER_AD

#define LOOPER_VOL PARAMETER_BA
#define REVERB_VOL PARAMETER_BB
#define DELAY_VOL PARAMETER_BC
#define RESONATOR_VOL PARAMETER_BD
#define FILTER_VOL PARAMETER_BE
#define IN_VOL PARAMETER_BF
#define SSWT_VOL PARAMETER_BG
#define SINE_VOL PARAMETER_BH

#define LOOPER_SPEED PARAMETER_CA
#define FILTER_RESODRIVE PARAMETER_CB
#define OSC_DETUNE PARAMETER_CC
#define LOOPER_LENGTH PARAMETER_CD
#define OSC_PITCH PARAMETER_CE
#define LOOPER_START PARAMETER_CF
#define RESONATOR_HARMONY PARAMETER_CG
#define RESONATOR_DECAY PARAMETER_CH

#define REVERB_TONESIZE PARAMETER_DA
#define REVERB_DECAY PARAMETER_DB
#define MOD_AMOUNT PARAMETER_DC
#define MOD_FREQ PARAMETER_DD
#define FILTER_CUTOFF PARAMETER_DE
#define DELAY_FEEDBACK PARAMETER_DF
#define DELAY_TIME PARAMETER_DG
#define RANDOM_MODE PARAMETER_DH

enum leds
{
  RECORD_LED = 1,
  RANDOM_LED,
  SYNC_LED,
  INLEVELRED_LED,
  INLEVELGREEN_LED,
  MOD_LED,
  CU_DOWN_LED,
  CU_UP_LED,
  FUNC_1_LED,
  FUNC_2_LED,
};

static bool calibration = false;
float oscVOctSampleLow = 0.5f, oscVOctSampleHigh = 0.5f, oscVOctVoltsLow = -5.f, oscVOctVoltsHigh = 5.f;
float filterVOctSampleLow = 0.5f, filterVOctSampleHigh = 0.5f, filterVOctVoltsLow = -5.f, filterVOctVoltsHigh = 10.f;
static bool randomButtonState = false;
static bool sswtSwitchState = false;
static bool funcButtonState = false;
static uint16_t randomAmountState = 0;
static uint16_t filterModeState = 0;
static uint16_t mux_values[NOF_MUX_VALUES] DMA_RAM = {};
uint16_t mins[40];
uint16_t maxes[40];

Pin randomGate(RANDOM_GATE_GPIO_Port, RANDOM_GATE_Pin);
Pin randomButton(RANDOM_BUTTON_GPIO_Port, RANDOM_BUTTON_Pin);
Pin funcButton(FUNC_BUTTON_GPIO_Port, FUNC_BUTTON_Pin);
Pin sswtSwitch(SSWT_SWITCH_GPIO_Port, SSWT_SWITCH_Pin);
Pin randomAmountSwitch1(RANDOM_AMOUNT_SWITCH1_GPIO_Port, RANDOM_AMOUNT_SWITCH1_Pin);
Pin randomAmountSwitch2(RANDOM_AMOUNT_SWITCH2_GPIO_Port, RANDOM_AMOUNT_SWITCH2_Pin);
Pin filterModeSwitch1(FILTER_MODE_SWITCH1_GPIO_Port, FILTER_MODE_SWITCH1_Pin);
Pin filterModeSwitch2(FILTER_MODE_SWITCH2_GPIO_Port, FILTER_MODE_SWITCH2_Pin);

Pin funcLed1(FUNC_1_LED_GPIO_Port, FUNC_1_LED_Pin);
Pin funcLed2(FUNC_2_LED_GPIO_Port, FUNC_2_LED_Pin);

// MUX binary counter digital output pins
Pin muxA(MUX_A_GPIO_Port, MUX_A_Pin);
Pin muxB(MUX_B_GPIO_Port, MUX_B_Pin);
Pin muxC(MUX_C_GPIO_Port, MUX_C_Pin);

void setCalibratedParameterValue(uint8_t pid, int16_t value)
{
  int16_t previous = getParameterValue(pid);
  // IIR exponential filter with lambda 0.75: y[n] = 0.75*y[n-1] + 0.25*x[n]
  float v = (float)((previous * 3 + value) >> 2) ;

  if (value < mins[pid])
  {
    value = mins[pid];
  }
  else if (value > maxes[pid])
  {
    value = maxes[pid];
  }
  v = (4095.f / (maxes[pid] - mins[pid])) * (value - mins[pid]);

  setParameterValue(pid, v);
}

void setMux(uint8_t index)
{
  muxA.set(index & 0b001);
  muxB.set(index & 0b010);
  muxC.set(index & 0b100);
}

void readMux(uint8_t index, uint16_t *mux_values)
{
  uint16_t muxA = 4095 - mux_values[MUX_A];
  uint16_t muxB = 4095 - mux_values[MUX_B];
  uint16_t muxC = 4095 - mux_values[MUX_C];
  uint16_t muxD = 4095 - mux_values[MUX_D];
  uint16_t muxE = 4095 - mux_values[MUX_E];

  setCalibratedParameterValue(REVERB_TONESIZE_CV, muxA);
  setCalibratedParameterValue(OSC_VOCT_CV, muxB);
  setCalibratedParameterValue(PARAMETER_BA + index, muxC);
  setCalibratedParameterValue(PARAMETER_CA + index, muxD);
  setCalibratedParameterValue(PARAMETER_DA + index, muxE);

  if (calibration)
  {
    float v = muxB / 4096.f;
    oscVOctSampleLow = min(oscVOctSampleLow, v);
    oscVOctSampleHigh = max(oscVOctSampleHigh, v);
    float scalar = ((oscVOctVoltsLow - oscVOctVoltsHigh) / (oscVOctSampleLow - oscVOctSampleHigh) * UINT16_MAX);
    settings.output_scalar = scalar;
    settings.output_offset = ((oscVOctSampleLow - oscVOctVoltsLow * UINT16_MAX / scalar) * UINT16_MAX);
  }
}

extern "C"
{
  
  void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
  {
    extern ADC_HandleTypeDef ADC_PERIPH;
    extern ADC_HandleTypeDef MUX_PERIPH;
    extern uint16_t adc_values[NOF_ADC_VALUES];
    if (hadc == &MUX_PERIPH)
    {
      static uint8_t mux_index = 0;
      setMux(mux_index + 1);
      readMux(mux_index, mux_values);
      mux_index = (mux_index + 1) & 0b111;
    }else if (hadc == &ADC_PERIPH){
      for (size_t i = 0; i < NOF_ADC_VALUES; i++)
	{
	  uint16_t value = 4095 - adc_values[i];
	  setCalibratedParameterValue(i, value);
	}
    }
  }
}

void readGpio()
{
  if (randomButtonState != !randomButton.get()) // Inverted: pressed = false
  {
    randomButtonState = !randomButton.get();
    setButtonValue(RANDOM_BUTTON, randomButtonState);
  }
  if (sswtSwitchState != !sswtSwitch.get()) // Inverted: pressed = false
  {
    sswtSwitchState = !sswtSwitch.get();
    setButtonValue(SSWT_SWITCH, sswtSwitchState);
  }
  if (funcButtonState != !funcButton.get()) // Inverted: pressed = false
  {
    funcButtonState = !funcButton.get();
    setButtonValue(FUNC_BUTTON, funcButtonState);
  }

  setAnalogValue(MOD_LED, getParameterValue(MOD));
  setAnalogValue(INLEVELGREEN_LED, getParameterValue(INLEVELGREEN));

  setLed(FUNC_1_LED, getParameterValue(FUNC1));
  setLed(FUNC_2_LED, getParameterValue(FUNC2));

#ifdef DEBUG

  int16_t delayCv = getParameterValue(DELAY_TIME_CV); // Ok (0 - 10v?)
  int16_t osc2Cv = getParameterValue(OSC_DETUNE_CV); // Ok (0 - 10v?)
  int16_t filterCv = getParameterValue(FILTER_CUTOFF_CV); // Ok (0 - 10v?)
  int16_t startCv = getParameterValue(LOOPER_START_CV); // Ok (0 - 10v?)
  int16_t lengthCv = getParameterValue(LOOPER_LENGTH_CV); // Ok (0 - 10v?)
  int16_t resonatorCv = getParameterValue(RESONATOR_HARMONY_CV); // Ok (0 - 10v?)
  int16_t speedCv = getParameterValue(LOOPER_SPEED_CV); // Ok (0 - 10v?)
  int16_t reverbCv = getParameterValue(REVERB_TONESIZE_CV); // Ok (0 - 10v?)
  int16_t vOctCv = getParameterValue(OSC_VOCT_CV); // ? 5v = 0

  int16_t looperVol = getParameterValue(LOOPER_VOL); // 0x4d8 - 0xffc
  int16_t reverbVol = getParameterValue(REVERB_VOL); // 0x4d6 - 0xffc

  int16_t delayVol = getParameterValue(DELAY_VOL); // 0x4d4 - 0xffc
  int16_t resoVol = getParameterValue(RESONATOR_VOL); // 0x4da - 0xffc
  int16_t filterVol = getParameterValue(FILTER_VOL); // 0x4d7 - 0xffc
  int16_t inVol = getParameterValue(IN_VOL); // 0x4d3 - 0xffc
  int16_t sswtVol = getParameterValue(SSWT_VOL); // 0x4d1 - 0xffc
  int16_t sineVol = getParameterValue(SINE_VOL); // 0x4d5 - 0xffc

  int16_t speed = getParameterValue(LOOPER_SPEED); // 0x4d9 - 0xfc5
  int16_t resoD = getParameterValue(FILTER_RESODRIVE); // 0x4da - 0xfc8
  int16_t detune = getParameterValue(OSC_DETUNE); // 0x4db - 0xfc6
  int16_t length = getParameterValue(LOOPER_LENGTH); // 0x4d8 - 0xfc7
  int16_t pitch = getParameterValue(OSC_PITCH); // 0x4d8 - 0xfc9
  int16_t start = getParameterValue(LOOPER_START); // 0x4da - 0xfc8
  int16_t resoHarmony = getParameterValue(RESONATOR_HARMONY); // 0x4d8 - 0xfc7
  int16_t resoDecay = getParameterValue(RESONATOR_DECAY); // 0x4da - 0xfc6
  int16_t toneSize = getParameterValue(REVERB_TONESIZE); // Ok
  int16_t decay = getParameterValue(REVERB_DECAY); // Ok
  int16_t cutoff = getParameterValue(FILTER_CUTOFF); // Ok

  int16_t delayF = getParameterValue(DELAY_FEEDBACK); // Not working
  int16_t delayA = getParameterValue(DELAY_TIME); // Not working
  int16_t randomMode = getParameterValue(RANDOM_MODE); // Not working

  int16_t modAmount = getParameterValue(MOD_AMOUNT); // Ok
  int16_t modFreq = getParameterValue(MOD_FREQ); // Ok

  #endif
}

void onChangePin(uint16_t pin)
{
  switch (pin)
  {
    case SYNC_GATE_Pin:
    {
      bool state = HAL_GPIO_ReadPin(SYNC_GATE_GPIO_Port, SYNC_GATE_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(SYNC_GATE, state);
      break;
    }
    case RECORD_BUTTON_Pin:
    {
      bool state = HAL_GPIO_ReadPin(RECORD_BUTTON_GPIO_Port, RECORD_BUTTON_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(RECORD_BUTTON, state);
      break;
    }
    case RECORD_GATE_Pin:
    {
      bool state = HAL_GPIO_ReadPin(RECORD_GATE_GPIO_Port, RECORD_GATE_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(RECORD_GATE, state);
      break;
    }
    case RANDOM_GATE_Pin:
    {
      bool state = HAL_GPIO_ReadPin(RANDOM_GATE_GPIO_Port, RANDOM_GATE_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(RANDOM_GATE, state);
      break;
    }
    case PREPOST_SWITCH_Pin:
    {
      bool state = HAL_GPIO_ReadPin(PREPOST_SWITCH_GPIO_Port, PREPOST_SWITCH_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(PREPOST_SWITCH, state);
      break;
    }
    case FUNC_BUTTON_Pin:
    {
      bool state = HAL_GPIO_ReadPin(FUNC_BUTTON_GPIO_Port, FUNC_BUTTON_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(FUNC_BUTTON, state);
      break;
    }
  }
}

void setAnalogValue(uint8_t ch, int16_t value)
{
  extern DAC_HandleTypeDef DAC_HANDLE;
  switch (ch)
  {
  case INLEVELGREEN_LED:
    HAL_DAC_SetValue(&DAC_HANDLE, DAC_CHANNEL_1, DAC_ALIGN_12B_R, __USAT(value, 12));
    break;
  case MOD_LED:
    HAL_DAC_SetValue(&DAC_HANDLE, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 4095 - __USAT(value, 12)); // Inverted
    break;
  }
}

void setGateValue(uint8_t ch, int16_t value)
{
  switch (ch)
  {
  case RECORD_BUTTON:
    setLed(RECORD_LED, value);
    break;
  case RANDOM_BUTTON:
    setLed(RANDOM_LED, value);
    break;
  case SYNC_GATE:
    setLed(SYNC_LED, value);
    break;
  case INLEVELRED:
    setLed(INLEVELRED_LED, value);
    break;
  case CU_DOWN:
    setLed(CU_DOWN_LED, value);
    break;
  case CU_UP:
    setLed(CU_UP_LED, value);
    break;
  case FUNC_1:
    setLed(FUNC_1_LED, value);
    break;
  case FUNC_2:
    setLed(FUNC_2_LED, value);
    break;
  }
}

void setLed(uint8_t led, uint32_t rgb)
{
  uint32_t pwm = 1024 - (__USAT(rgb>>2, 10)); // expects 12-bit parameter value

  switch (led)
  {
  case RECORD_LED:
    HAL_GPIO_WritePin(RECORD_BUTTON_LED_GPIO_Port, RECORD_BUTTON_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_RESET : GPIO_PIN_SET);
    break;
  case RANDOM_LED:
    HAL_GPIO_WritePin(RANDOM_BUTTON_LED_GPIO_Port, RANDOM_BUTTON_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_RESET : GPIO_PIN_SET);
    break;
  case SYNC_LED:
    HAL_GPIO_WritePin(SYNC_LED_GPIO_Port, SYNC_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_SET : GPIO_PIN_RESET); // Inverted
    break;
  case INLEVELRED_LED:
    HAL_GPIO_WritePin(INLEVELRED_LED_GPIO_Port, INLEVELRED_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_RESET : GPIO_PIN_SET);
    break;
  case CU_DOWN_LED:
    HAL_GPIO_WritePin(CU_DOWN_LED_GPIO_Port, CU_DOWN_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_SET : GPIO_PIN_RESET); // Inverted
    break;
  case CU_UP_LED:
    HAL_GPIO_WritePin(CU_UP_LED_GPIO_Port, CU_UP_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_SET : GPIO_PIN_RESET); // Inverted
    break;
  case FUNC_1_LED:
    TIM4->CCR3 = pwm;
    break;
  case FUNC_2_LED:
    TIM4->CCR4 = pwm;
    break;
  }
}

void ledsOn()
{
  setLed(RECORD_LED, 1);
  setLed(RANDOM_LED, 1);
  setLed(SYNC_LED, 1);
  setLed(INLEVELRED_LED, 1);
  setLed(CU_DOWN_LED, 1);
  setLed(CU_UP_LED, 1);
  setLed(FUNC_2_LED, RED_COLOUR);
  setAnalogValue(MOD_LED, 4095);
}

void ledsOff()
{
  setLed(RECORD_LED, NO_COLOUR);
  setLed(RANDOM_LED, NO_COLOUR);
  setLed(SYNC_LED, NO_COLOUR);
  setLed(INLEVELRED_LED, NO_COLOUR);
  setLed(CU_DOWN_LED, NO_COLOUR);
  setLed(CU_UP_LED, NO_COLOUR);
  setLed(FUNC_2_LED, NO_COLOUR);
  setAnalogValue(MOD_LED, NO_COLOUR);
  setAnalogValue(INLEVELGREEN_LED, NO_COLOUR);
}

void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len)
{
  if (calibration)
    {
      uint16_t value = 4095 - adc_values[FILTER_CUTOFF_CV];
      float v = value / 4096.f;
      filterVOctSampleLow = min(filterVOctSampleLow, v);
      filterVOctSampleHigh = max(filterVOctSampleHigh, v);
      float scalar = ((filterVOctVoltsLow - filterVOctVoltsHigh) / (filterVOctSampleLow - filterVOctSampleHigh) * UINT16_MAX);
      settings.input_scalar = scalar;
      settings.input_offset = ((filterVOctSampleLow - filterVOctVoltsLow * UINT16_MAX / scalar) * UINT16_MAX);
    }

  uint8_t value = (randomAmountSwitch2.get() << 1) | randomAmountSwitch1.get();
  parameter_values[RANDOM_AMOUNT] = 2047 * (value - 1); // Mid = 0, Low = 2047, High = 4094

  value = (filterModeSwitch2.get() << 1) | filterModeSwitch1.get();
  parameter_values[FILTER_MODE_SWITCH] = 2047 * (value - 1); // BP = 0, LP = 2047, HP = 4094
}

void onSetup()
{
  extern TIM_HandleTypeDef htim4;
  HAL_TIM_Base_Start(&htim4);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3); // FUNC_1_LED
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4); // FUNC_2_LED

  //funcLed1.outputMode();
  //funcLed2.outputMode();

  extern ADC_HandleTypeDef MUX_PERIPH;
  if (HAL_ADC_Start_DMA(&MUX_PERIPH, (uint32_t *)mux_values, NOF_MUX_VALUES) != HAL_OK)
  {
    error(CONFIG_ERROR, "ADC1 Start failed");
  }

  onChangePin(SYNC_GATE_Pin);
  onChangePin(RECORD_BUTTON_Pin);
  onChangePin(RECORD_GATE_Pin);
  onChangePin(RANDOM_GATE_Pin);
  onChangePin(PREPOST_SWITCH_Pin);

  for (size_t i = 0; i < NOF_PARAMETERS; i++)
  {
    mins[i] = 0;
    maxes[i] = (i >= 24 || i <= 38) ? 4030 : 4095; // Pots have reduced range
  }
}

void onLoop(void)
{
  static bool configMode = false;
  static uint32_t counter = PATCH_RESET_COUNTER;

  bool funcButtonPressed = HAL_GPIO_ReadPin(FUNC_BUTTON_GPIO_Port, FUNC_BUTTON_Pin) == GPIO_PIN_RESET;

  switch (owl.getOperationMode())
  {
  case STARTUP_MODE:
    if (getErrorStatus() != NO_ERROR)
    {
      owl.setOperationMode(ERROR_MODE);
    }
    break;
  case LOAD_MODE:
    if (--counter == 0)
    {
      counter = PATCH_RESET_COUNTER;
      ledsOff();
      owl.setOperationMode(RUN_MODE);
    }
    else if (getErrorStatus() != NO_ERROR)
    {
      owl.setOperationMode(ERROR_MODE);
    }
    break;
  case RUN_MODE:
    readGpio();
    if (getErrorStatus() != NO_ERROR)
    {
      owl.setOperationMode(ERROR_MODE);
    }
    break;

  case CONFIGURE_MODE:
    if (funcButtonPressed)
    {
      if (calibration)
      {
        // Exit config mode
        calibration = false;
        setLed(RECORD_LED, 0);
        setLed(RANDOM_LED, 0);
        settings.saveToFlash(false);
        owl.setOperationMode(LOAD_MODE);
      }
      else if (configMode == false)
      {
        // Enter config mode.
        setLed(RECORD_LED, 1);
        setLed(RANDOM_LED, 1);
        configMode = true;
      }
    }
    else
    {
      if (configMode)
      {
        // Enter calibration.
        calibration = true;
        readGpio();
        extern uint16_t adc_values[NOF_ADC_VALUES];
        extern int16_t parameter_values[NOF_PARAMETERS];
        updateParameters(parameter_values, NOF_PARAMETERS, adc_values, NOF_ADC_VALUES);
      }
      else
      {
        // Config button has not been pressed during startup, go to load mode.
        ledsOn();
        owl.setOperationMode(LOAD_MODE);
      }
    }
    break;

  case ERROR_MODE:
    ledsOn();
    if (funcButtonPressed)
    {
      if (--counter == 0)
      {
        // Reset device after error.
        ledsOff();
        setErrorStatus(NO_ERROR);
        owl.setOperationMode(STARTUP_MODE);
        program.resetProgram(false);
      }
    }
    else
    {
      const char* message = getErrorMessage();
      counter = PATCH_RESET_COUNTER;
    }
    break;
  }
}

#define MAX_PATCH_SETTINGS 16
#define PATCH_SETTINGS_NAME "oneiroi"

bool onMidiSend(uint8_t port, uint8_t status, uint8_t d1, uint8_t d2){
  static MidiMessage data[MAX_PATCH_SETTINGS] = {0}; 
  MidiMessage msg(port, status, d1, d2);
  if(msg.isPitchBend()){
    int ch = msg.getStatus();
    if(ch < MAX_PATCH_SETTINGS)
      data[ch] = msg;
  }else if(msg.getStatus() == START){
    // clear settings
    memset(data, 0, sizeof(data));
    return false; // suppress this message
  }else if(msg.getStatus() == STOP){
    const char* filename = msg.getChannel() == 1 ? PATCH_SETTINGS_NAME ".cv" : PATCH_SETTINGS_NAME ".mod";
    debugMessage("Saving settings", (int)msg.getChannel());
    taskENTER_CRITICAL();
    storage.writeResource(filename, (uint8_t*)data, sizeof(data), FLASH_DEFAULT_FLAGS);
    taskEXIT_CRITICAL();
    debugMessage(filename, (int)sizeof(data));
    return false; // suppress this message
  }
  return true;
}

/*
  // called twice from the constructor: restore("oneiroi.cv") and restore("oneiroi.mod")
  void restore(const char* name){
    Resource* resource = Resource::load(name);
    if(resource){
      MidiMessage[] cfg = (MidiMessage[])resource->getData();
      uint8_t count = resource->getSize()/sizeof(MidiMessage); // number of messages in file
      for(int i=0; i<count; ++i)
        if(cfg[i].packed != 0) // Roberto: add this to avoid loading uninitialized, zero values
    	  processMidi(cfg[i]);
    }
  }
  void processMidi(MidiMessage msg){
    if(msg.isPitchBend()){
      float value = msg.getPitchBend()/8192.0f; // convert signed 14-bit pitch bend to float
      switch(msg.getChannel()){
      case 0:
        // update setting 0 with 'value'
	break;
      }
    }
  }
  void save(){
    // send start
    int8_t ch = 1; // 1 saves "oneiroi.cv", 0 saves "oneiroi.mod"
    sendMidi(MidiMessage(USB_COMMAND_SINGLE_BYTE, START|ch, 0, 0)); // send MIDI START
    // for each setting:
    uint8_t channel; // setting id between 0 and 12
    float value; // settings value between -1 and 0.9999
    int16_t bend = (int16_t)(value * 8192); // convert to 14-bit signed int
    sendMidi(MidiMessage::pb(channel, bend));
    // then save:
    sendMidi(MidiMessage(USB_COMMAND_SINGLE_BYTE, STOP|ch, 0, 0)); // send MIDI STOP
  }
*/
