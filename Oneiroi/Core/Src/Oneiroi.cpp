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

// GPIO
#define RECORDBUTTON BUTTON_1
#define RECORDGATE BUTTON_2
#define RANDOMBUTTON BUTTON_3
#define RANDOMGATE BUTTON_4
#define SYNCIN BUTTON_5
#define INLEVELRED BUTTON_6
#define PREPOSTSWITCH BUTTON_7
#define WTSWITCH BUTTON_8

#define RANDOMAMOUNT PARAMETER_AA
#define FILTERMODE PARAMETER_AB

// ADC3
#define OSC2CV PARAMETER_A
#define FILTERCV PARAMETER_B
#define RESONATORCV PARAMETER_C
#define DELAYCV PARAMETER_D
#define STARTCV PARAMETER_E
#define LENGTHCV PARAMETER_F
#define SPEEDCV PARAMETER_G

// Muxed
#define REVERBCV PARAMETER_AC
#define VOCTCV PARAMETER_AD

#define LOOPER_VOL PARAMETER_BA
#define REVERB_VOL PARAMETER_BB
#define DELAY_VOL PARAMETER_BC
#define RESO_VOL PARAMETER_BD
#define FILTER_VOL PARAMETER_BE
#define IN_VOL PARAMETER_BF
#define SSWT_VOL PARAMETER_BG
#define SINE_VOL PARAMETER_BH

#define SPEED PARAMETER_CA
#define RESOD PARAMETER_CB
#define DETUNE PARAMETER_CC
#define LENGTH PARAMETER_CD
#define PITCH PARAMETER_CE
#define START PARAMETER_CF
#define RESOHARMONY PARAMETER_CG
#define RESODECAY PARAMETER_CH

#define TONESIZE PARAMETER_DA
#define DECAY PARAMETER_DB
#define MODAMOUNT PARAMETER_DC
#define MODFREQ PARAMETER_DD
#define CUTOFF PARAMETER_DE
#define DELAYF PARAMETER_DF
#define DELAYA PARAMETER_DG
#define RANDOM_MODE PARAMETER_DH

enum leds
{
  RECORDLED = 1,
  RANDOMLED,
  SYNCINLED,
  INLEVELREDLED,
  INLEVELLEDGREEN, // PARAMETER_F
  MODLED           // PARAMETER_G
};

static bool randomButtonState = false;
static bool wtSwitchState = false;
static uint16_t randomAmountState = 0;
static uint16_t filterModeState = 0;
static uint16_t mux_values[NOF_MUX_VALUES] DMA_RAM = {};

Pin randomGate(RANDOMGATEIN_GPIO_Port, RANDOMGATEIN_Pin);
Pin randomButton(RANDOMBUTTON_GPIO_Port, RANDOMBUTTON_Pin);
Pin wtSwitch(WTSWITCH_GPIO_Port, WTSWITCH_Pin);
Pin randomAmountSwitch1(RANDOMAMOUNTSWITCH1_GPIO_Port, RANDOMAMOUNTSWITCH1_Pin);
Pin randomAmountSwitch2(RANDOMAMOUNTSWITCH2_GPIO_Port, RANDOMAMOUNTSWITCH2_Pin);
Pin filterModeSwitch1(FILTERMODESWITCH1_GPIO_Port, FILTERMODESWITCH1_Pin);
Pin filterModeSwitch2(FILTERMODESWITCH2_GPIO_Port, FILTERMODESWITCH2_Pin);

// MUX binary counter digital output pins
Pin muxA(MUX_A_GPIO_Port, MUX_A_Pin);
Pin muxB(MUX_B_GPIO_Port, MUX_B_Pin);
Pin muxC(MUX_C_GPIO_Port, MUX_C_Pin);

void onChangePin(uint16_t pin)
{
  switch (pin)
  {
  case SYNCIN_Pin:
  {
    bool state = HAL_GPIO_ReadPin(SYNCIN_GPIO_Port, SYNCIN_Pin) == GPIO_PIN_RESET; // Inverted
    setButtonValue(SYNCIN, state);
    setLed(SYNCINLED, state);
    break;
  }
  case RECORDBUTTON_Pin:
  {
    bool state = HAL_GPIO_ReadPin(RECORDBUTTON_GPIO_Port, RECORDBUTTON_Pin) == GPIO_PIN_RESET; // Inverted
    setButtonValue(RECORDBUTTON, state);
    setLed(RECORDLED, state);
    break;
  }
  case RECORDGATEIN_Pin:
  {
    bool state = HAL_GPIO_ReadPin(RECORDGATEIN_GPIO_Port, RECORDGATEIN_Pin) == GPIO_PIN_RESET; // Inverted
    setButtonValue(RECORDGATE, state);
    setLed(RECORDLED, state);
    break;
  }
  case RANDOMGATEIN_Pin:
  {
    bool state = HAL_GPIO_ReadPin(RANDOMGATEIN_GPIO_Port, RANDOMGATEIN_Pin) == GPIO_PIN_RESET; // Inverted
    setButtonValue(RANDOMGATE, state);
    setLed(RANDOMLED, state);
    break;
  }
  case PREPOSTSWITCH_Pin:
  {
    bool state = HAL_GPIO_ReadPin(PREPOSTSWITCH_GPIO_Port, PREPOSTSWITCH_Pin) == GPIO_PIN_RESET; // Inverted
    setButtonValue(PREPOSTSWITCH, state);
    break;
  }
  }
}

void setAnalogValue(uint8_t ch, int16_t value)
{
  extern DAC_HandleTypeDef DAC_HANDLE;
  switch (ch)
  {
  case INLEVELLEDGREEN:
    HAL_DAC_SetValue(&DAC_HANDLE, DAC_CHANNEL_1, DAC_ALIGN_12B_R, __USAT(value, 12));
    break;
  case MODLED:
    HAL_DAC_SetValue(&DAC_HANDLE, DAC_CHANNEL_2, DAC_ALIGN_12B_R, __USAT(value, 12));
    break;
  }
}

void setGateValue(uint8_t ch, int16_t value)
{
  switch (ch)
  {
  case RECORDBUTTON:
    setLed(RECORDLED, value);
    break;
  case RANDOMBUTTON:
    setLed(RANDOMLED, value);
    break;
  case SYNCIN:
    setLed(SYNCINLED, value);
    break;
  case INLEVELRED:
    setLed(INLEVELREDLED, value);
    break;
  }
}

void setLed(uint8_t led, uint32_t rgb)
{
  switch (led)
  {
  case RECORDLED:
    HAL_GPIO_WritePin(RECORDBUTTONLED_GPIO_Port, RECORDBUTTONLED_Pin, rgb == NO_COLOUR ? GPIO_PIN_RESET : GPIO_PIN_SET);
    break;
  case RANDOMLED:
    HAL_GPIO_WritePin(RANDOMBUTTONLED_GPIO_Port, RANDOMBUTTONLED_Pin, rgb == NO_COLOUR ? GPIO_PIN_RESET : GPIO_PIN_SET);
    break;
  case SYNCINLED:
    HAL_GPIO_WritePin(SYNCIN_GPIO_Port, SYNCIN_Pin, rgb == NO_COLOUR ? GPIO_PIN_SET : GPIO_PIN_RESET); // Inverted
    break;
  case INLEVELREDLED:
    HAL_GPIO_WritePin(INLEVELREDLED_GPIO_Port, INLEVELREDLED_Pin, rgb == NO_COLOUR ? GPIO_PIN_RESET : GPIO_PIN_SET);
    break;
  }
}

void onSetup()
{
  setLed(RECORDLED, 0);
  setLed(RANDOMLED, 0);
  setLed(SYNCINLED, 0);
  setLed(INLEVELREDLED, 0);
  // setGateValue(PUSHBUTTON, 0);

  // start MUX ADC
  extern ADC_HandleTypeDef MUX_PERIPH;
  if (HAL_ADC_Start_DMA(&MUX_PERIPH, (uint32_t *)mux_values, NOF_MUX_VALUES) != HAL_OK)
    error(CONFIG_ERROR, "ADC1 Start failed");
}

void setMux(uint8_t index)
{
  muxA.set(index & 0b001);
  muxB.set(index & 0b010);
  muxC.set(index & 0b100);
}

void readMux(uint8_t index, uint16_t *mux_values)
{
  setParameterValue(REVERBCV, mux_values[MUX_A]);
  setParameterValue(VOCTCV, mux_values[MUX_B]);
  setParameterValue(PARAMETER_BA + index, mux_values[MUX_C]); // MUX 1
  setParameterValue(PARAMETER_CA + index, mux_values[MUX_D]); // MUX 2
  setParameterValue(PARAMETER_DA + index, mux_values[MUX_E]); // MUX 3
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
        mux_index = 0;
    }
  }
}

void onLoop(void)
{
  if (randomButtonState != randomButton.get())
  {
    randomButtonState = !randomButton.get(); // Inverted
    setButtonValue(RANDOMBUTTON, randomButtonState);
    setLed(RANDOMLED, randomButtonState);
  }
  if (wtSwitchState != wtSwitch.get())
  {
    wtSwitchState = !wtSwitch.get(); // Inverted
    setButtonValue(WTSWITCH, wtSwitchState);
  }
  uint8_t value = (randomAmountSwitch2.get() << 1) | randomAmountSwitch1.get();
  if (value != randomAmountState)
  {
    randomAmountState = value; // LOW = 2,
    setParameterValue(RANDOMAMOUNT, value * 2047);
  }
  value = (filterModeSwitch2.get() << 1) | filterModeSwitch1.get();
  if (value != filterModeState)
  {
    filterModeState = value; // BP = 1, LP = 2, HP = 3
    setParameterValue(FILTERMODE, value * 2047);
  }

  /*
  int16_t delayCv = 4095 - getAnalogValue(DELAYCV); // Ok (0 - 10v?)
  int16_t osc2Cv = 4095 - getAnalogValue(OSC2CV); // Ok (0 - 10v?)
  int16_t filterCv = 4095 - getAnalogValue(FILTERCV); // Ok (0 - 10v?)
  int16_t startCv = 4095 - getAnalogValue(STARTCV); // Ok (0 - 10v?)
  int16_t lengthCv = 4095 - getAnalogValue(LENGTHCV); // Ok (0 - 10v?)
  int16_t resonatorCv = 4095 - getAnalogValue(RESONATORCV); // Ok (0 - 10v?)
  int16_t speedCv = 4095 - getAnalogValue(SPEEDCV); // Ok (0 - 10v?)

  int16_t reverbCv = 4095 - getParameterValue(REVERBCV); // Ok (0 - 10v?)
  int16_t vOctCv = getParameterValue(VOCTCV); // ? 5v = 0
  */

  //int16_t randomAmount = getParameterValue(RANDOMAMOUNT); // Not working: HIGH = 6k, MID = 2k, LOW = 6k
  //int16_t FILTERMODE = getParameterValue(FILTERMODE); // HP = 6k, BP = 2k, LP = 4k

  int16_t looperVol = getParameterValue(LOOPER_VOL); // Not working
  int16_t reverbVol = getParameterValue(REVERB_VOL); // Ok, also LOOPER_VOL
  /*
  int16_t delayVol = 4095 - getParameterValue(DELAY_VOL); // Ok
  int16_t resoVol = 4095 - getParameterValue(RESO_VOL); // Ok
  int16_t filterVol = 4095 - getParameterValue(FILTER_VOL); // Ok
  int16_t inVol = 4095 - getParameterValue(IN_VOL); // Ok
  int16_t sswtVol = 4095 - getParameterValue(SSWT_VOL); // Ok
  int16_t sineVol = 4095 - getParameterValue(SINE_VOL); // Ok
  */

  /*
  int16_t speed = 4095 - getParameterValue(SPEED); // Ok
  int16_t resoD = 4095 - getParameterValue(RESOD); // Ok
  int16_t detune = 4095 - getParameterValue(DETUNE); // Ok
  int16_t length = 4095 - getParameterValue(LENGTH); // Ok
  int16_t pitch = 4095 - getParameterValue(PITCH); // Ok
  int16_t start = 4095 - getParameterValue(START); // Ok
  int16_t resoHarmony = 4095 - getParameterValue(RESOHARMONY); // Ok
  int16_t resoDecay = 4095 - getParameterValue(RESODECAY); // Ok
  */
  int16_t toneSize = 4095 - getParameterValue(TONESIZE); // Ok
  int16_t decay = 4095 - getParameterValue(DECAY); // Ok
  //int16_t cutoff = 4095 - getParameterValue(CUTOFF); // Ok
  int16_t delayF = getParameterValue(DELAYF); // Not working
  int16_t delayA = getParameterValue(DELAYA); // Not working
  int16_t randomMode = getParameterValue(RANDOM_MODE); // Not working

  int16_t modAmount = 4095 - getParameterValue(MODAMOUNT); // Ok
  int16_t modFreq = 4095 - getParameterValue(MODFREQ); // Ok

  setAnalogValue(INLEVELLEDGREEN, modFreq); // Ok
  setAnalogValue(MODLED, modAmount); // Ok
}