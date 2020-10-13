#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "Owl.h"
#include "errorhandlers.h"
#include "message.h"
#include "ProgramManager.h"
#include "PatchRegistry.h"
#include "OpenWareMidiControl.h"


void setGateValue(uint8_t ch, int16_t value){
  // if(ch == PUSHBUTTON)
  //   HAL_GPIO_WritePin(GATE_OUT_GPIO_Port, GATE_OUT_Pin, value ? GPIO_PIN_RESET :  GPIO_PIN_SET);
}

void setup(){
  HAL_GPIO_WritePin(EXP_TIP_GPIO_Port, EXP_TIP_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
  owl_setup();
}

void loop(void){
  owl_loop();
}
