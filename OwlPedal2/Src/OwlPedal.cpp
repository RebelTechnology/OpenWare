#include "Owl.h"
#include "device.h"
#include "errorhandlers.h"
#include "MidiController.h"
#include "OpenWareMidiControl.h"
#include "Pin.h"

#define PATCH_RESET_COUNTER (1000/MAIN_LOOP_SLEEP_MS)

// Pin footswitch_pin(GPIOA, GPIO_PIN_0);
Pin bypass_pin(GPIOA, GPIO_PIN_0);
Pin bufpass_pin(GPIOF, GPIO_PIN_9);
Pin exp1_ring_pin(GPIOA, GPIO_PIN_2);
Pin led_green_pin(GPIOB, GPIO_PIN_8);
Pin led_red_pin(GPIOB, GPIO_PIN_9);

#define SW3_Pin EXP2_T_Pin
#define SW3_GPIO_Port EXP2_T_GPIO_Port
#define SW4_Pin EXP2_R_Pin
#define SW4_GPIO_Port EXP2_R_GPIO_Port

void setLed(uint8_t led, uint32_t rgb){
  switch(rgb){
  case RED_COLOUR:
    led_green_pin.low();
    led_red_pin.high();
    break;
  case GREEN_COLOUR:
    led_green_pin.high();
    led_red_pin.low();
    break;
  case NO_COLOUR:
    led_green_pin.low();
    led_red_pin.low();
    break;
  }
}

void onChangePin(uint16_t pin){
  switch(pin){
  case FOOTSWITCH_Pin: { // stomp switch
    bool state = HAL_GPIO_ReadPin(FOOTSWITCH_GPIO_Port, FOOTSWITCH_Pin) == GPIO_PIN_RESET;
    setButtonValue(0, state);
    setLed(0, state ? NO_COLOUR : GREEN_COLOUR);
    break;
  }
  case SW1_Pin: { // pushbutton
    bool state = HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET;
    setButtonValue(PUSHBUTTON, state);
    setButtonValue(BUTTON_A, state);
    midi_tx.sendCc(PATCH_BUTTON, state ? 127 : 0);
    setLed(0, state ? RED_COLOUR : GREEN_COLOUR);
    break;
  }
  case SW2_Pin: { // mode button
    bool state = HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_RESET;
    setButtonValue(BUTTON_B, state);
    break;
  }
  case SW3_Pin: { // EXP2 Tip
    bool state = HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin) == GPIO_PIN_RESET;
    setButtonValue(BUTTON_C, state);
    break;
  }
  case SW4_Pin: { // EXP2 Ring
    bool state = HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == GPIO_PIN_RESET;
    setButtonValue(BUTTON_D, state);
    break;
  }
  }
}

void setGateValue(uint8_t ch, int16_t value){
  switch(ch){
  case PUSHBUTTON:
    setLed(0, value ? RED_COLOUR : GREEN_COLOUR);
#ifdef OWL_MODULAR
    HAL_GPIO_WritePin(PUSH_GATE_OUT_GPIO_Port, PUSH_GATE_OUT_Pin, value ? GPIO_PIN_RESET :  GPIO_PIN_SET);
#endif
    break;
  case GREEN_BUTTON:
    setLed(0, value ? GREEN_COLOUR : NO_COLOUR);
    break;
  case RED_BUTTON:
    setLed(0, value ? RED_COLOUR : NO_COLOUR);
    break;
  case BUTTON_1:
    bufpass_pin.set(value);
    break;
  }
}

void setup(){
  bypass_pin.outputMode();
  bypass_pin.low();
  bufpass_pin.outputMode();
  bufpass_pin.low();
  exp1_ring_pin.outputMode();
  exp1_ring_pin.high();

  led_green_pin.outputMode();
  led_red_pin.outputMode();

  setLed(0, RED_COLOUR);
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
