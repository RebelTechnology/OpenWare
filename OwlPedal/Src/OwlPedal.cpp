#include "Owl.h"
#include "device.h"
#include "errorhandlers.h"
#include "MidiController.h"
#include "OpenWareMidiControl.h"
#include "ApplicationSettings.h"
#include "PatchRegistry.h"
#include "ProgramManager.h"

#define PROGRAM_CHANGE_PUSHBUTTON_TICKS 2000
// Number of buffers before patch switching kicks in, takes 2.66s

#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif

extern uint32_t ledstatus;

void initLed(){
}

static bool pushButtonPressed = 0;
static uint16_t pushButtonPressDuration = 0;
static uint16_t pushButtonAnimation = 0;
static uint8_t owlSelectedProgramId = 0;

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
    pushButtonPressed = !(PUSHBUTTON_GPIO_Port->IDR & PUSHBUTTON_Pin);
    if (pushButtonPressed) {
      // Rising edge
      pushButtonPressDuration = 0;
      pushButtonAnimation = 0x96;
      owlSelectedProgramId = settings.program_index;
    }
    else {
      // Falling edge
      if (pushButtonPressDuration > PROGRAM_CHANGE_PUSHBUTTON_TICKS &&
        settings.program_index != owlSelectedProgramId) {
        // This runs unless button was depressed too early or the same
        // patch as before was selected.
        settings.program_index = owlSelectedProgramId;
        program.loadProgram(owlSelectedProgramId);
        program.resetProgram(false);
      }
    }
    setButtonValue(BUTTON_A, pushButtonPressed);
    setButtonValue(PUSHBUTTON, pushButtonPressed);
    midi_tx.sendCc(PUSHBUTTON, pushButtonPressed ? 127 : 0);
    setLed(0, pushButtonPressed ? RED_COLOUR : GREEN_COLOUR);
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

inline int16_t getProgramSelection(){
  // This code is copied from original firmware. The only difference is that
  // it won't be running in a separate OS task. So we won't be waiting for
  // usable patch being selected. Instead of that, we return -1 for invalid
  // selection - it should be ignored by the caller.
  // NOTE: Bank/prog must be static as they act as feedback values for hysteresis
  #ifdef OWL_MODULAR
  static int bank = (4095 - getAnalogValue(PARAMETER_A)) * 5 / 4096;
  static int prog = (4095 - getAnalogValue(PARAMETER_B)) * 8 / 4096 + 1;
  float a = (4095 - getAnalogValue(PARAMETER_A)) * 5 / 4096.0 - 0.5 / 5;
  float b = (4095 - getAnalogValue(PARAMETER_B)) * 8 / 4096.0 - 0.5 / 8;
  #else
  static int bank = getAnalogValue(PARAMETER_A) * 5 / 4096;
  static int prog = getAnalogValue(PARAMETER_B) * 8 / 4096 + 1;
  float a = getAnalogValue(PARAMETER_A) * 5 / 4096.0 - 0.5 / 5;
  float b = getAnalogValue(PARAMETER_B) * 8 / 4096.0 - 0.5 / 8;
  #endif
  //if(a - (int)a < 0.8) // deadband each segment: [0.8-1.0)
  if(a > 0 && abs(a - (int)a - 0.1) > 0.2) // deadband each segment: [0.9-1.1]
    bank = (int)a;
  if(b > 0 && abs(b - (int)b - 0.1) > 0.2)
    prog = (int)b+1;
  int pc = bank * 8 + prog;
  // We must check that patch exist and verify that it's considered valid
  // to avoid loading from a gap in the patches list
  if (pc < (int)registry.getNumberOfPatches() && registry.getPatchDefinition(pc) != NULL)
    return pc;
  else
    return -1;
}

void setGateValue(uint8_t ch, int16_t value){
  switch (ch) {
  case BUTTON_A:
    setLed(0, value ? RED_COLOUR : GREEN_COLOUR);
    break;
  case PUSHBUTTON:
    setLed(0, value ? RED_COLOUR : GREEN_COLOUR);
    // deliberate fall-through - this synchronizes LED to pushbutton value
  case BUTTON_B:
#ifdef OWL_MODULAR
    HAL_GPIO_WritePin(PUSH_GATE_OUT_GPIO_Port, PUSH_GATE_OUT_Pin, value ? GPIO_PIN_RESET : GPIO_PIN_SET);
#endif
    break;
  case GREEN_BUTTON:
    setLed(0, GREEN_COLOUR);
    break;
  case RED_BUTTON:
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
    else if (pushButtonPressed) {
      if (pushButtonPressDuration++> PROGRAM_CHANGE_PUSHBUTTON_TICKS){
        pushButtonPressDuration &= 0x3fff;
        // Pushbutton kept pressed above threshold, overflow is prevented
        if (pushButtonAnimation){
          // Pulse as we unlock patch selection
          if ((pushButtonAnimation-- & 0xf) == 0){
            HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
            HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
          }
        }
        // Ready to check for patch selection
        int16_t newOwlProgramId = getProgramSelection();
        if (newOwlProgramId != owlSelectedProgramId && newOwlProgramId >= 0){
          owlSelectedProgramId = newOwlProgramId;
          // Toggle pushbutton LEDs if patch number was changed
          HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
          HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        }
      }
    }
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
