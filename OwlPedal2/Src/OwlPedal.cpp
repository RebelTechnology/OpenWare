#include "Owl.h"
#include "errorhandlers.h"
#include "Codec.h"
#include "MidiController.h"
#include "ProgramManager.h"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"
#include "PatchRegistry.h"
#include "Pin.h"
#include "usb_device.h"

#define PATCH_RESET_COUNTER (1000/MAIN_LOOP_SLEEP_MS)
#define PATCH_CONFIG_COUNTER (3000/MAIN_LOOP_SLEEP_MS)

static int16_t knobvalues[2];
static uint8_t patchselect;
#define PATCH_CONFIG_KNOB_THRESHOLD (4096/4)
#define PATCH_CONFIG_PROGRAM_CONTROL 0
#define PATCH_CONFIG_VOLUME_CONTROL 3

// Pin footswitch_pin(GPIOA, GPIO_PIN_0);
Pin bypass_pin(GPIOA, GPIO_PIN_0);
Pin bufpass_pin(GPIOF, GPIO_PIN_9); // high is bypass
Pin exp1_tip_pin(GPIOA, GPIO_PIN_2);
Pin exp1_ring_pin(GPIOA, GPIO_PIN_3);
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

bool getBufferedBypass(){
  return !bufpass_pin.get();
}

void setBufferedBypass(bool value){
  setLed(0, value ? RED_COLOUR : GREEN_COLOUR);
  bufpass_pin.set(!value);
}

bool isPushbuttonPressed(){
  return HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET ||
    HAL_GPIO_ReadPin(SW1_ALT_GPIO_Port, SW1_ALT_Pin) == GPIO_PIN_RESET;
}

void onChangePin(uint16_t pin){
  switch(pin){
  case FOOTSWITCH_Pin: { // bypass / stomp switch
    bool state = HAL_GPIO_ReadPin(FOOTSWITCH_GPIO_Port, FOOTSWITCH_Pin) == GPIO_PIN_RESET;
    setButtonValue(0, state);
    setLed(0, state ? NO_COLOUR : GREEN_COLOUR);

    // todo: save LED state
    // todo: only allow config mode in bypass?
    break;
  }
  case SW1_Pin:
  case SW1_ALT_Pin: { // pushbutton
    bool state = isPushbuttonPressed();
    setButtonValue(PUSHBUTTON, state);
    setButtonValue(BUTTON_A, state);
    setLed(0, state ? RED_COLOUR : GREEN_COLOUR);
    midi_tx.sendCc(PATCH_BUTTON, state ? 127 : 0);
    // if(state){
    //   // toggle buffered bypass 
    //   setBufferedBypass(!getBufferedBypass());
    // }
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
    setBufferedBypass(value);
    break;
  }
}

void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len){
  parameter_values[0] = (parameter_values[0]*3 + adc_values[ADC_A])>>2;
  parameter_values[1] = (parameter_values[1]*3 + adc_values[ADC_B])>>2;
  parameter_values[2] = (parameter_values[2]*3 + adc_values[ADC_C])>>2;
  parameter_values[3] = (parameter_values[3]*3 + adc_values[ADC_D])>>2;
  parameter_values[4] = (parameter_values[4]*3 + 4095-adc_values[ADC_E])>>2;
}

void onSetup(){
  bypass_pin.outputMode();
  bypass_pin.low();
  bufpass_pin.outputMode();
  bufpass_pin.low();
  // Ring is available on ADC_F
  exp1_ring_pin.outputMode();
  exp1_ring_pin.high();
  // exp1_tip_pin.analogMode();

  led_green_pin.outputMode();
  led_red_pin.outputMode();

  setLed(0, RED_COLOUR);

  MX_USB_DEVICE_Init();  

  setBufferedBypass(false);
}

static uint32_t counter = 0;
void onChangeMode(OperationMode new_mode, OperationMode old_mode){
  counter = 0;
  setLed(0, NO_COLOUR);
  if(new_mode == CONFIGURE_MODE){
    knobvalues[0] = getAnalogValue(PATCH_CONFIG_PROGRAM_CONTROL);
    knobvalues[1] = getAnalogValue(PATCH_CONFIG_VOLUME_CONTROL);
    patchselect = program.getProgramIndex();
  }else if(new_mode == RUN_MODE){
    if(HAL_GPIO_ReadPin(FOOTSWITCH_GPIO_Port, FOOTSWITCH_Pin) == GPIO_PIN_RESET){
      setButtonValue(0, true);
    }else{
      setLed(0, GREEN_COLOUR); // todo: restore to saved state
    }
  }
}


void onLoop(){
  switch(owl.getOperationMode()){
  case STARTUP_MODE:
  case STREAM_MODE:
    setLed(0, counter > PATCH_RESET_COUNTER/2 ? GREEN_COLOUR : NO_COLOUR);
    if(counter-- == 0)
      counter = PATCH_RESET_COUNTER;
    break;
  case LOAD_MODE:
    setLed(0, counter > PATCH_RESET_COUNTER/10 ? GREEN_COLOUR : NO_COLOUR);
    if(counter-- == 0)
      counter = PATCH_RESET_COUNTER;
    break;
  case RUN_MODE:
    if(getErrorStatus() != NO_ERROR)
      owl.setOperationMode(ERROR_MODE);
    if(isPushbuttonPressed()){
      if(counter-- == 0){
	owl.setOperationMode(CONFIGURE_MODE);	
      }else if(counter < PATCH_RESET_COUNTER){
	setLed(0, counter > PATCH_RESET_COUNTER/10 ? GREEN_COLOUR : NO_COLOUR);
      }
    }else{
      counter = PATCH_CONFIG_COUNTER;
    }
    break;
  case CONFIGURE_MODE:
    if(isPushbuttonPressed()){
      if(abs(knobvalues[0] - getAnalogValue(PATCH_CONFIG_PROGRAM_CONTROL)) >= PATCH_CONFIG_KNOB_THRESHOLD){
	knobvalues[0] = -PATCH_CONFIG_KNOB_THRESHOLD;
	uint8_t idx = 1 + (getAnalogValue(PATCH_CONFIG_PROGRAM_CONTROL) * registry.getNumberOfPatches()) / 4096;
	if(registry.hasPatch(idx)){
	  patchselect = idx;
	  midi_tx.sendPc(idx);
	  setLed(0, idx & 0x01 ? RED_COLOUR : GREEN_COLOUR);
	}
      }
      if(abs(knobvalues[1] - getAnalogValue(PATCH_CONFIG_VOLUME_CONTROL)) >= PATCH_CONFIG_KNOB_THRESHOLD){
	knobvalues[1] = -PATCH_CONFIG_KNOB_THRESHOLD;
	uint8_t value = (getAnalogValue(PATCH_CONFIG_VOLUME_CONTROL) >> 6) + 63;
	if(settings.audio_output_gain != value){
	  settings.audio_output_gain = value;
	  codec.setOutputGain(value);
	  midi_tx.sendCc(MIDI_CC_VOLUME, value);
	}
      }
    }else{      
      if(program.getProgramIndex() != patchselect &&
	 registry.hasPatch(patchselect)){
	// change patch on mode button release
	program.loadProgram(patchselect); // enters load mode (calls onChangeMode)
	program.resetProgram(false);
      }else{
	owl.setOperationMode(RUN_MODE);
      }
    }
    break;
  case ERROR_MODE:
    setLed(0, counter > PATCH_RESET_COUNTER/2 ? RED_COLOUR : NO_COLOUR);
    if(counter-- == 0)
      counter = PATCH_RESET_COUNTER;
    break;
  }
}
