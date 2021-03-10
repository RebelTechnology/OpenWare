#include "Owl.h"
#include "device.h"
#include "errorhandlers.h"
#include "MidiController.h"
#include "OpenWareMidiControl.h"

extern uint32_t ledstatus;

void initLed(){
}

void setLed(uint8_t led, uint32_t rgb){
  // rgb should be a 3x 10 bit value
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
  case PUSHBUTTON_Pin: {
    bool isSet = !(PUSHBUTTON_GPIO_Port->IDR & PUSHBUTTON_Pin);
    setButtonValue(PUSHBUTTON, isSet);
    midi_tx.sendCc(PUSHBUTTON, isSet ? 127 : 0);
    setLed(0, isSet ? RED_COLOUR : GREEN_COLOUR);
    break;
  }
#ifdef OWL_PEDAL
  case BYPASS_Pin: {
    bool isSet = !(BYPASS_GPIO_Port->IDR & BYPASS_Pin);
    setLed(0, isSet ? NO_COLOUR : GREEN_COLOUR);
    break;
  }
#endif
#ifdef OWL_MODULAR
  case PUSH_GATE_IN_Pin: {
    bool isSet = !(PUSH_GATE_IN_GPIO_Port->IDR & PUSH_GATE_IN_Pin);
    setButtonValue(PUSHBUTTON, isSet);
    setLed(0, isSet ? RED_COLOUR : GREEN_COLOUR);
    break;
  }
#endif
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
#ifdef OWL_PEDAL
  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  // if (HAL_GetREVID() == 0x1001)
  // {
  //   /* Enable the Flash prefetch */
  //   __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  // }
  // enable expression pedal reference voltage
  HAL_GPIO_WritePin(EXPRESSION_PEDAL_TIP_GPIO_Port, EXPRESSION_PEDAL_TIP_Pin, GPIO_PIN_SET);
  // todo: on OWL Modular the ADC should read Exp pin PA2 instead of PA3
#endif
  owl.setup();
 }
 
#define PATCH_RESET_COUNTER (1000/MAIN_LOOP_SLEEP_MS)

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
