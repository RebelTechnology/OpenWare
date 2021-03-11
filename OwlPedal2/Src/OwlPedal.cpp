#include "Owl.h"
#include "device.h"
#include "errorhandlers.h"
#include "MidiController.h"
#include "OpenWareMidiControl.h"

#define PATCH_RESET_COUNTER (1000/MAIN_LOOP_SLEEP_MS)

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
  case YELLOW_COLOUR:
    // not working
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
    break;
  case NO_COLOUR:
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
    break;
  }
}

void onChangePin(uint16_t pin){
  switch(pin){
  case SW1_Pin: {
    bool isSet = !(SW1_GPIO_Port->IDR & SW1_Pin);
    setLed(0, isSet ? NO_COLOUR : GREEN_COLOUR);
    break;
  }
  case SW2_Pin: {
    bool isSet = !(SW2_GPIO_Port->IDR & SW2_Pin);
    setButtonValue(PUSHBUTTON, isSet);
    midi_tx.sendCc(PUSHBUTTON, isSet ? 127 : 0);
    setLed(0, isSet ? RED_COLOUR : GREEN_COLOUR);
    break;
  }
  case SW3_Pin: {
    bool isSet = !(SW3_GPIO_Port->IDR & SW3_Pin);
    setButtonValue(BUTTON_B, isSet);
    break;
  }
  case SW4_Pin: {
    bool isSet = !(SW4_GPIO_Port->IDR & SW4_Pin);
    setButtonValue(BUTTON_C, isSet);
    break;
  }
  }
}

void setGateValue(uint8_t ch, int16_t value){
  if(ch == PUSHBUTTON){
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, value ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
#ifdef OWL_MODULAR
    HAL_GPIO_WritePin(PUSH_GATE_OUT_GPIO_Port, PUSH_GATE_OUT_Pin, value ? GPIO_PIN_RESET :  GPIO_PIN_SET);
#endif
  }else if(ch == GREEN_BUTTON){
    setLed(0, GREEN_COLOUR);
  }else if(ch == RED_BUTTON){
    setLed(0, RED_COLOUR);
  }
}

void setup(){
  setLed(0, RED_COLOUR);
  HAL_GPIO_WritePin(EXP_TIP_GPIO_Port, EXP_TIP_Pin, GPIO_PIN_SET);
  owl.setup();
  setLed(0, GREEN_COLOUR);
}

void loop(){
  static uint32_t counter = PATCH_RESET_COUNTER;
  switch(owl.getOperationMode()){
  case STARTUP_MODE:
  case LOAD_MODE:
    setLed(0, counter > PATCH_RESET_COUNTER/2 ? GREEN_COLOUR : NO_COLOUR);
    break;
  case RUN_MODE:
    if(getErrorStatus() != NO_ERROR)
      owl.setOperationMode(ERROR_MODE);
    break;
  case CONFIGURE_MODE:
    owl.setOperationMode(RUN_MODE);
    break;
  case STREAM_MODE:
  case ERROR_MODE:
    setLed(0, counter > PATCH_RESET_COUNTER/2 ? RED_COLOUR : NO_COLOUR);
    break;
  }
  if(--counter == 0)
    counter = PATCH_RESET_COUNTER;

  owl.loop();
}
