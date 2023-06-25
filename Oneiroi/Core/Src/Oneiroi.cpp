#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "Owl.h"
#include "errorhandlers.h"
#include "message.h"
#include "ProgramManager.h"
#include "PatchRegistry.h"
#include "OpenWareMidiControl.h"
#include "ApplicationSettings.h"
#include "Pin.h"

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef abs
#define abs(x) ((x) > 0 ? (x) : -(x))
#endif

#define PATCH_RESET_COUNTER (2000/MAIN_LOOP_SLEEP_MS)

#define INLEVELGREEN PARAMETER_AF
#define MOD PARAMETER_AG

// GPIO
#define RECORD_BUTTON BUTTON_1
#define RECORD_GATE BUTTON_2
#define RANDOM_BUTTON BUTTON_3
#define RANDOM_GATE BUTTON_4
#define SYNC_GATE BUTTON_5
#define INLEVELRED BUTTON_6
#define PREPOST_SWITCH BUTTON_7
#define SSWT_SWITCH BUTTON_8

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
  MOD_LED
};

static bool randomButtonState = false;
static bool sswtSwitchState = false;
static uint16_t randomAmountState = 0;
static uint16_t filterModeState = 0;
static uint16_t mux_values[NOF_MUX_VALUES] DMA_RAM = {};

Pin randomGate(RANDOM_GATE_GPIO_Port, RANDOM_GATE_Pin);
Pin randomButton(RANDOM_BUTTON_GPIO_Port, RANDOM_BUTTON_Pin);
Pin sswtSwitch(SSWT_SWITCH_GPIO_Port, SSWT_SWITCH_Pin);
Pin randomAmountSwitch1(RANDOM_AMOUNT_SWITCH1_GPIO_Port, RANDOM_AMOUNT_SWITCH1_Pin);
Pin randomAmountSwitch2(RANDOM_AMOUNT_SWITCH2_GPIO_Port, RANDOM_AMOUNT_SWITCH2_Pin);
Pin filterModeSwitch1(FILTER_MODE_SWITCH1_GPIO_Port, FILTER_MODE_SWITCH1_Pin);
Pin filterModeSwitch2(FILTER_MODE_SWITCH2_GPIO_Port, FILTER_MODE_SWITCH2_Pin);

// MUX binary counter digital output pins
Pin muxA(MUX_A_GPIO_Port, MUX_A_Pin);
Pin muxB(MUX_B_GPIO_Port, MUX_B_Pin);
Pin muxC(MUX_C_GPIO_Port, MUX_C_Pin);

void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len){
  // IIR exponential filter with lambda 0.75
  // inverting ADCs
  parameter_values[0] = (parameter_values[0]*3 + 4095-adc_values[0])>>2;
  parameter_values[1] = (parameter_values[1]*3 + 4095-adc_values[1])>>2;
  parameter_values[2] = (parameter_values[2]*3 + 4095-adc_values[2])>>2;
  parameter_values[3] = (parameter_values[3]*3 + 4095-adc_values[3])>>2;
  parameter_values[4] = (parameter_values[4]*3 + 4095-adc_values[4])>>2;
  parameter_values[5] = (parameter_values[5]*3 + 4095-adc_values[5])>>2;
  parameter_values[6] = (parameter_values[6]*3 + 4095-adc_values[6])>>2;
}

void onChangePin(uint16_t pin)
{
  switch (pin)
  {
    case SYNC_GATE_Pin:
    {
      bool state = HAL_GPIO_ReadPin(SYNC_GATE_GPIO_Port, SYNC_GATE_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(SYNC_GATE, state);
      setLed(SYNC_LED, state);
      break;
    }
    case RECORD_BUTTON_Pin:
    {
      bool state = HAL_GPIO_ReadPin(RECORD_BUTTON_GPIO_Port, RECORD_BUTTON_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(RECORD_BUTTON, state);
      setLed(RECORD_LED, state);
      break;
    }
    case RECORD_GATE_Pin:
    {
      bool state = HAL_GPIO_ReadPin(RECORD_GATE_GPIO_Port, RECORD_GATE_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(RECORD_GATE, state);
      setLed(RECORD_LED, state);
      break;
    }
    case RANDOM_GATE_Pin:
    {
      bool state = HAL_GPIO_ReadPin(RANDOM_GATE_GPIO_Port, RANDOM_GATE_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(RANDOM_GATE, state);
      setLed(RANDOM_LED, state);
      break;
    }
    case PREPOST_SWITCH_Pin:
    {
      bool state = HAL_GPIO_ReadPin(PREPOST_SWITCH_GPIO_Port, PREPOST_SWITCH_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(PREPOST_SWITCH, state);
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
  }
}

void setLed(uint8_t led, uint32_t rgb)
{
  switch (led)
  {
  case RECORD_LED:
    HAL_GPIO_WritePin(RECORD_BUTTON_LED_GPIO_Port, RECORD_BUTTON_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_RESET : GPIO_PIN_SET);
    break;
  case RANDOM_LED:
    HAL_GPIO_WritePin(RANDOM_BUTTON_LED_GPIO_Port, RANDOM_BUTTON_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_RESET : GPIO_PIN_SET);
    break;
  case SYNC_LED:
    HAL_GPIO_WritePin(SYNC_LED_GPIO_Port, SYNC_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_SET : GPIO_PIN_RESET);
    break;
  case INLEVELRED_LED:
    HAL_GPIO_WritePin(INLEVELRED_LED_GPIO_Port, INLEVELRED_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_RESET : GPIO_PIN_SET);
    break;
  }
}

void ledsOn()
{
  setLed(RECORD_LED, 1);
  setLed(RANDOM_LED, 1);
  setLed(SYNC_LED, 1);
  setLed(INLEVELRED_LED, 1);
  setAnalogValue(MOD_LED, 4095);
}

void ledsOff()
{
  setLed(RECORD_LED, 0);
  setLed(RANDOM_LED, 0);
  setLed(SYNC_LED, 0);
  setLed(INLEVELRED_LED, 0);
  setAnalogValue(MOD_LED, 0);
  setAnalogValue(INLEVELGREEN_LED, 0);
}

void onSetup()
{
  // start MUX ADC
  extern ADC_HandleTypeDef MUX_PERIPH;
  if (HAL_ADC_Start_DMA(&MUX_PERIPH, (uint32_t *)mux_values, NOF_MUX_VALUES) != HAL_OK)
  {
    error(CONFIG_ERROR, "ADC1 Start failed");
  }

  setParameterValue(MOD, 0);
  setParameterValue(INLEVELGREEN, 0);

  ledsOn();
}

void setMux(uint8_t index)
{
  muxA.set(index & 0b001);
  muxB.set(index & 0b010);
  muxC.set(index & 0b100);
}

void readMux(uint8_t index, uint16_t *mux_values)
{
  setParameterValue(REVERB_TONESIZE_CV, mux_values[MUX_A]);
  setParameterValue(OSC_VOCT_CV, mux_values[MUX_B]);
  setParameterValue(PARAMETER_BA + index, mux_values[MUX_C]); // Faders are inverted
  setParameterValue(PARAMETER_CA + index, 4095 - mux_values[MUX_D]);
  setParameterValue(PARAMETER_DA + index, 4095 - mux_values[MUX_E]);
}

extern "C"
{
  void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
  {
    extern ADC_HandleTypeDef MUX_PERIPH;
    if (hadc == &MUX_PERIPH)
    {
      static uint8_t mux_index = 0;
      setMux(mux_index + 1);
      readMux(mux_index, mux_values);
      if (mux_index++ == 8)
      {
        mux_index = 0;
      }
    }
  }
}

void _loop()
{
  if (randomButtonState != !randomButton.get()) // Inverted: pressed = false
  {
    randomButtonState = !randomButton.get();
    setButtonValue(RANDOM_BUTTON, randomButtonState);
    setLed(RANDOM_LED, randomButtonState);
  }
  if (sswtSwitchState != !sswtSwitch.get()) // Inverted: pressed = false
  {
    sswtSwitchState = !sswtSwitch.get();
    setButtonValue(SSWT_SWITCH, sswtSwitchState); // Ok
  }
  uint8_t value = (randomAmountSwitch2.get() << 1) | randomAmountSwitch1.get();
  if (value != randomAmountState)
  {
    // Value: Mid = 1, Low = 2, High = 3
    randomAmountState = value;
    setParameterValue(RANDOM_AMOUNT, 2048 * (value - 1)); // Mid = 0, Low = 2048, High = 4096
  }
  value = (filterModeSwitch2.get() << 1) | filterModeSwitch1.get();
  if (value != filterModeState)
  {
    // Value: Mid = 1, Low = 2, High = 3
    filterModeState = value;
    setParameterValue(FILTER_MODE_SWITCH, 2048 * (value - 1)); // BP = 0, LP = 2048, HP = 4096
  }

  setAnalogValue(MOD_LED, getParameterValue(MOD));
  setAnalogValue(INLEVELGREEN_LED, getParameterValue(INLEVELGREEN));

#ifdef DEBUG

  int16_t delayCv = 4095 - getAnalogValue(DELAY_TIME_CV); // Ok (0 - 10v?)
  int16_t osc2Cv = 4095 - getAnalogValue(OSC_DETUNE_CV); // Ok (0 - 10v?)
  int16_t filterCv = 4095 - getAnalogValue(FILTER_CUTOFF_CV); // Ok (0 - 10v?)
  int16_t startCv = 4095 - getAnalogValue(LOOPER_START_CV); // Ok (0 - 10v?)
  int16_t lengthCv = 4095 - getAnalogValue(LOOPER_LENGTH_CV); // Ok (0 - 10v?)
  int16_t resonatorCv = 4095 - getAnalogValue(RESONATOR_HARMONY_CV); // Ok (0 - 10v?)
  int16_t speedCv = 4095 - getAnalogValue(LOOPER_SPEED_CV); // Ok (0 - 10v?)

  int16_t reverbCv = getParameterValue(REVERB_TONESIZE_CV); // Ok (0 - 10v?)
  int16_t vOctCv = getParameterValue(OSC_VOCT_CV); // ? 5v = 0

  int16_t looperVol = getParameterValue(LOOPER_VOL); // Not working
  int16_t reverbVol = getParameterValue(REVERB_VOL); // Ok, also LOOPER_VOL


  int16_t delayVol = getParameterValue(DELAY_VOL); // Ok
  int16_t resoVol = getParameterValue(RESONATOR_VOL); // Ok
  int16_t filterVol = getParameterValue(FILTER_VOL); // Ok
  int16_t inVol = getParameterValue(IN_VOL); // Ok
  int16_t sswtVol = getParameterValue(SSWT_VOL); // Ok
  int16_t sineVol = getParameterValue(SINE_VOL); // Ok

  int16_t speed = getParameterValue(LOOPER_SPEED); // Ok
  int16_t resoD = getParameterValue(FILTER_RESODRIVE); // Ok
  int16_t detune = getParameterValue(OSC_DETUNE); // Ok
  int16_t length = getParameterValue(LOOPER_LENGTH); // Ok
  int16_t pitch = getParameterValue(OSC_PITCH); // Ok
  int16_t start = getParameterValue(LOOPER_START); // Ok
  int16_t resoHarmony = getParameterValue(RESONATOR_HARMONY); // Ok
  int16_t resoDecay = getParameterValue(RESONATOR_DECAY); // Ok
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

bool first = true;

void onLoop(void)
{
  if (first)
  {
    ledsOff();
    first = false;
  }

  _loop();
  return;


  static uint32_t counter = PATCH_RESET_COUNTER;

  bool saveButtonsPressed = getButtonValue(RANDOM_BUTTON) && getButtonValue(RECORD_BUTTON);

  switch (owl.getOperationMode())
  {
  case STARTUP_MODE:
    ledsOff();
    break;
  case LOAD_MODE:
    //
    break;
  case RUN_MODE:
    if (getErrorStatus() != NO_ERROR)
    {
      owl.setOperationMode(ERROR_MODE);
    }
    else
    {
      _loop();

      if (saveButtonsPressed)
      {
        // press and hold to store settings
        if (--counter == 0)
        {
          counter = PATCH_RESET_COUNTER;
          settings.saveToFlash(false);
        }
      }
      else
      {
        counter = PATCH_RESET_COUNTER;
      }
    }
    break;
  case CONFIGURE_MODE:
    owl.setOperationMode(RUN_MODE);
    break;
  case STREAM_MODE:
    //
    break;
  case ERROR_MODE:
    ledsOn();
    if (--counter == 0)
      counter = PATCH_RESET_COUNTER;
    if (saveButtonsPressed)
    {
      ledsOff();
      setErrorStatus(NO_ERROR);
      owl.setOperationMode(RUN_MODE); // allows new patch selection if patch doesn't load
      program.resetProgram(false);
    }
    break;
  }
}