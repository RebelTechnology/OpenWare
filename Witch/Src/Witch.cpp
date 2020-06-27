#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "Owl.h"
#include "errorhandlers.h"
#include "message.h"
#include "ProgramManager.h"
#include "PatchRegistry.h"
#include "OpenWareMidiControl.h"

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif

void setGateValue(uint8_t ch, int16_t value){
  switch(ch){
  case PUSHBUTTON:
  case BUTTON_A:
    HAL_GPIO_WritePin(GATE_OUT_GPIO_Port, GATE_OUT_Pin, value ? GPIO_PIN_RESET :  GPIO_PIN_SET);
    break;
  case BUTTON_B:
    break;
  }    
}

bool isModeButtonPressed(){
  return HAL_GPIO_ReadPin(ENC_SW_GPIO_Port, ENC_SW_Pin) == GPIO_PIN_RESET;
}

void setup(){
  HAL_GPIO_WritePin(GATE_OUT_GPIO_Port, GATE_OUT_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_SW1_GPIO_Port, LED_SW1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_SW2_GPIO_Port, LED_SW2_Pin, GPIO_PIN_SET);
  owl_setup();
  setEncoderValue(program.getProgramIndex());
}

void loop(void){
  MX_USB_HOST_Process(); // todo: enable PWR management
  owl_loop();
}
