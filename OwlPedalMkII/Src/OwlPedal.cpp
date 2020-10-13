#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "Owl.h"
#include "errorhandlers.h"
#include "message.h"
#include "ProgramManager.h"
#include "PatchRegistry.h"
#include "OpenWareMidiControl.h"


void setLed(uint8_t led, uint32_t rgb){
  switch(rgb){
  case RED_COLOUR:
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
    break;
  case GREEN_COLOUR:
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
    break;
  // case YELLOW_COLOUR:
  //   // not working
  //   HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
  //   HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
  //   break;
  case NO_COLOUR:
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
    break;
  }
}

void setGateValue(uint8_t ch, int16_t value){
  if(ch == PUSHBUTTON){
    HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
  }else if(ch == GREEN_BUTTON){
    setLed(0, GREEN_COLOUR);
  }else if(ch == RED_BUTTON){
    setLed(0, RED_COLOUR);
  }
}

void setup(){
  setLed(0, RED_COLOUR);
  HAL_GPIO_WritePin(EXP_TIP_GPIO_Port, EXP_TIP_Pin, GPIO_PIN_SET);
  owl_setup();
  setLed(0, GREEN_COLOUR);
}

void loop(void){
  owl_loop();
}
