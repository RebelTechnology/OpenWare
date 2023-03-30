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

void onChangePin(uint16_t pin){
  switch(pin){
    case SYNCIN_Pin:
      break;
    case RECORDBUTTON_Pin:
      break;
    case RECORDGATEIN_Pin:
      break;
    case RANDOMBUTTON_Pin:
      break;
    case PREPOSTSWITCH_Pin:
      break;
    case FILTERMODESWITCH_Pin:
      break;
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
  case BUTTON_A:
    setLed(1, value);
    break;
  case BUTTON_B:
    setLed(2, value);
    break;
  case BUTTON_C:
    setLed(3, value);
    break;
  case BUTTON_D:
    setLed(4, value);
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
  setLed(1, 0);
  setLed(2, 0);
  setLed(3, 0);
  setLed(4, 0);
  //setGateValue(PUSHBUTTON, 0);
}

void onLoop(void){

}
