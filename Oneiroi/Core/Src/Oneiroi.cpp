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
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif

Pin randomGate(RANDOMGATEIN_GPIO_Port, RANDOMGATEIN_Pin);
Pin randomButton(RANDOMBUTTON_GPIO_Port, RANDOMBUTTON_Pin);
Pin wtSwitch(WTSWITCH_GPIO_Port, WTSWITCH_Pin);
Pin randomAmountSwitch1(RANDOMAMOUNTSWITCH1_GPIO_Port, RANDOMAMOUNTSWITCH1_Pin);
Pin randomAmountSwitch2(RANDOMAMOUNTSWITCH2_GPIO_Port, RANDOMAMOUNTSWITCH2_Pin);
Pin filterModeSwitch1(FILTERMODESWITCH1_GPIO_Port, FILTERMODESWITCH1_Pin);
Pin filterModeSwitch2(FILTERMODESWITCH2_GPIO_Port, FILTERMODESWITCH2_Pin);

bool randomGateState, wtSwitchState;

#define RECORDBUTTON BUTTON_1
#define RECORDGATE BUTTON_2
#define RANDOMBUTTON BUTTON_3
#define RANDOMGATE BUTTON_4
#define SYNCIN BUTTON_5
#define INLEVELRED BUTTON_6
#define PREPOSTSWITCH BUTTON_7
#define WTSWITCH BUTTON_8

enum leds {
  RECORDLED = 1,
  RANDOMLED,
  SYNCINLED,
  INLEVELREDLED
};

void onChangePin(uint16_t pin){
  switch(pin){
    case SYNCIN_Pin:
    {
      bool state = HAL_GPIO_ReadPin(SYNCIN_GPIO_Port, SYNCIN_Pin) == GPIO_PIN_RESET;
      setButtonValue(SYNCIN, state);
      break;
    }
    case RECORDBUTTON_Pin:
    {
      bool state = HAL_GPIO_ReadPin(RECORDBUTTON_GPIO_Port, RECORDBUTTON_Pin) == GPIO_PIN_RESET;
      setButtonValue(RECORDBUTTON, state);
      break;
    }
    case RECORDGATEIN_Pin:
    {
      bool state = HAL_GPIO_ReadPin(RECORDGATEIN_GPIO_Port, RECORDGATEIN_Pin) == GPIO_PIN_RESET;
      setButtonValue(RECORDGATE, state);
      break;
    }
    case RANDOMBUTTON_Pin:
    {
      bool state = HAL_GPIO_ReadPin(RANDOMBUTTON_GPIO_Port, RANDOMBUTTON_Pin) == GPIO_PIN_RESET;
      setButtonValue(RANDOMBUTTON, state);
      break;
    }
    case PREPOSTSWITCH_Pin:
    {
      bool state = HAL_GPIO_ReadPin(PREPOSTSWITCH_GPIO_Port, PREPOSTSWITCH_Pin) == GPIO_PIN_RESET;
      setButtonValue(PREPOSTSWITCH, state);
      break;
    }
  }
}

void setAnalogValue(uint8_t ch, int16_t value){
extern DAC_HandleTypeDef DAC_HANDLE;
  switch(ch){
  case PARAMETER_F:
    HAL_DAC_SetValue(&DAC_HANDLE, DAC_CHANNEL_1, DAC_ALIGN_12B_R, __USAT(value, 12));
    break;
  case PARAMETER_G:
    HAL_DAC_SetValue(&DAC_HANDLE, DAC_CHANNEL_2, DAC_ALIGN_12B_R, __USAT(value, 12));
    break;
  }
}

void setGateValue(uint8_t ch, int16_t value){
  switch(ch){
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

void setLed(uint8_t led, uint32_t rgb){
  switch(led){
  case 1:
    HAL_GPIO_WritePin(RECORDBUTTONLED_GPIO_Port, RECORDBUTTONLED_Pin, rgb == NO_COLOUR ? GPIO_PIN_SET : GPIO_PIN_RESET);
    break;
  case 2:
    HAL_GPIO_WritePin(RANDOMBUTTONLED_GPIO_Port, RANDOMBUTTONLED_Pin, rgb == NO_COLOUR ? GPIO_PIN_SET : GPIO_PIN_RESET);
    break;
  case 3:
    HAL_GPIO_WritePin(SYNCIN_GPIO_Port, SYNCIN_Pin, rgb == NO_COLOUR ? GPIO_PIN_SET : GPIO_PIN_RESET);
    break;
  case 4:
    HAL_GPIO_WritePin(INLEVELREDLED_GPIO_Port, INLEVELREDLED_Pin, rgb == NO_COLOUR ? GPIO_PIN_SET : GPIO_PIN_RESET);
    break;
  }
}

void onSetup(){
  setLed(RECORDLED, 0);
  setLed(RANDOMLED, 0);
  setLed(SYNCINLED, 0);
  setLed(INLEVELREDLED, 0);
  //setGateValue(PUSHBUTTON, 0);
}

void onLoop(void){
  if (randomGateState != randomGate.get()) {
    randomGateState = randomGate.get();
    setButtonValue(RANDOMGATE, randomGateState);
    setLed(RANDOMLED, randomGateState);
  }
  if (wtSwitchState != wtSwitch.get()) {
    wtSwitchState = wtSwitch.get();
    setButtonValue(WTSWITCH, wtSwitchState);
  }
  if (wtSwitchState != wtSwitch.get()) {
    wtSwitchState = wtSwitch.get();
    setButtonValue(WTSWITCH, wtSwitchState);
  }
}
