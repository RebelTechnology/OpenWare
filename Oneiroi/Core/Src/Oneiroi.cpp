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

#define RECORDBUTTON BUTTON_1
#define RECORDGATE BUTTON_2
#define RANDOMBUTTON BUTTON_3
#define RANDOMGATE BUTTON_4
#define SYNCIN BUTTON_5
#define INLEVELRED BUTTON_6
#define PREPOSTSWITCH BUTTON_7
#define WTSWITCH BUTTON_8

#define MUX_PERIPH hadc1
#define NOF_MUX_VALUES 5

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
  return; // TODO: Do we need this?

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

void readMux(uint8_t index, uint16_t *adc_values)
{
  setParameterValue(PARAMETER_A, adc_values[0]);          // REVERBCV
  setParameterValue(PARAMETER_B, adc_values[1]);          // VOCTCV
  setParameterValue(PARAMETER_BA + index, adc_values[2]); // MUX 1
  setParameterValue(PARAMETER_CA + index, adc_values[3]); // MUX 2
  setParameterValue(PARAMETER_DA + index, adc_values[4]); // MUX 3
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
    randomAmountState = value;
    setParameterValue(PARAMETER_AC, value * 2047);
  }
  value = (filterModeSwitch2.get() << 1) | filterModeSwitch1.get();
  if (value != filterModeState)
  {
    filterModeState = value;
    setParameterValue(PARAMETER_AD, value * 2047);
  }
}