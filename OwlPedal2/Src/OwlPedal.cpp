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

#define PATCH_RESET_COUNTER (1024/MAIN_LOOP_SLEEP_MS)
#define PATCH_CONFIG_COUNTER (3072/MAIN_LOOP_SLEEP_MS)
#define PATCH_INDICATOR_COUNTER (128/MAIN_LOOP_SLEEP_MS)

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

void initLed(){
#ifdef OWL_PEDAL_PWM_LEDS
  extern TIM_HandleTypeDef htim4;
  HAL_TIM_Base_Start(&htim4);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
#else
  led_green_pin.outputMode();
  led_red_pin.outputMode();
#endif
}

void setLed(uint8_t led, uint32_t rgb){
#ifdef OWL_PEDAL_PWM_LEDS
  uint32_t r = 1023 - ((rgb>>20) & 0x3ff);
  uint32_t g = 1023 - ((rgb>>10) & 0x3ff);  
  TIM4->CCR3 = r;
  TIM4->CCR4 = g;
#else
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
 default:
   break;
  }
#endif
}

bool getBufferedBypass(){
  return !bufpass_pin.get();
}

void setBufferedBypass(bool value){
  setLed(0, value ? RED_COLOUR : GREEN_COLOUR);
  bufpass_pin.set(!value);
}

bool isBypassed(){
  return HAL_GPIO_ReadPin(FOOTSWITCH_GPIO_Port, FOOTSWITCH_Pin) == GPIO_PIN_RESET;
}

bool isPushbuttonPressed(){
  return HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET ||
    HAL_GPIO_ReadPin(SW1_ALT_GPIO_Port, SW1_ALT_Pin) == GPIO_PIN_RESET;
}

void onChangePin(uint16_t pin){
  switch(pin){
  case FOOTSWITCH_Pin: { // bypass / stomp switch
    bool state = isBypassed();
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
    if(state && owl.getOperationMode() == CONFIGURE_MODE){
      // exit configure mode
      owl.setOperationMode(RUN_MODE);
    }
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
  initLed();
  bypass_pin.outputMode();
  bypass_pin.low();
  bufpass_pin.outputMode();
  bufpass_pin.low();
  // Ring is available on ADC_F
  exp1_ring_pin.outputMode();
  exp1_ring_pin.high();
  // exp1_tip_pin.analogMode();

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
    counter = PATCH_RESET_COUNTER;
  }else if(new_mode == BYPASS_MODE){
    setLed(0, NO_COLOUR); // todo: save LED state
  }else if(new_mode == RUN_MODE){
    if(old_mode == CONFIGURE_MODE){
      if(program.getProgramIndex() != patchselect &&
	 registry.hasPatch(patchselect)){
	// change patch on mode button release
	program.loadProgram(patchselect); // enters load mode (calls onChangeMode)
	program.resetProgram(true); // true if setOperationMode() is called from button IRQ
      // }else{
      // 	owl.setOperationMode(RUN_MODE);
      }
    }
    setLed(0, GREEN_COLOUR); // todo: restore to saved state
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
    setLed(0, counter > PATCH_INDICATOR_COUNTER/2 ? GREEN_COLOUR : NO_COLOUR);
    // todo: use setProgress()?
    if(counter-- == 0)
      counter = PATCH_RESET_COUNTER;
    break;
  case BYPASS_MODE:
    if(!isBypassed()){
      owl.setOperationMode(RUN_MODE);      
    }else if(isPushbuttonPressed()){
      if(counter-- == 0){
	owl.setOperationMode(CONFIGURE_MODE);	
      }else if(counter % 100 == 0){
	led_green_pin.toggle();
	led_red_pin.toggle();
      }
    }else{
      counter = PATCH_RESET_COUNTER;
    }
    break;
  case RUN_MODE:
    if(getErrorStatus() != NO_ERROR){
      owl.setOperationMode(ERROR_MODE);
    }else if(isBypassed()){
      owl.setOperationMode(BYPASS_MODE);      
    }else if(isPushbuttonPressed()){
      if(counter-- == 0){
	owl.setOperationMode(CONFIGURE_MODE);	
      }else if(counter < PATCH_RESET_COUNTER && counter % 100 == 0){
	led_green_pin.toggle();
	led_red_pin.toggle();
      }
    }else{
      counter = PATCH_CONFIG_COUNTER;
    }
    break;
  case CONFIGURE_MODE:
    // if(isPushbuttonPressed()){
      if(abs(knobvalues[0] - getAnalogValue(PATCH_CONFIG_PROGRAM_CONTROL)) >= PATCH_CONFIG_KNOB_THRESHOLD){
	knobvalues[0] = -PATCH_CONFIG_KNOB_THRESHOLD;
	float idx = 1 + (getAnalogValue(PATCH_CONFIG_PROGRAM_CONTROL) * registry.getNumberOfPatches()) / 4096.0f;
	if(abs(patchselect - idx) > 0.6 && registry.hasPatch(idx)){ // ensure a small dead zone
	  patchselect = idx;
	  midi_tx.sendPc(idx);
	  // setLed(0, idx & 0x01 ? RED_COLOUR : GREEN_COLOUR);
	}
      // }else{
      // 	led_green_pin.toggle();
      // 	led_red_pin.toggle();
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
      if(counter-- == 0){
	counter = PATCH_RESET_COUNTER + PATCH_INDICATOR_COUNTER + PATCH_INDICATOR_COUNTER*2*patchselect;
	setLed(0, NO_COLOUR);
      }else if(counter <= PATCH_RESET_COUNTER){
	// toggle rapidly during 1 second for yellow-ish LED
	led_red_pin.set(led_green_pin.get());
	led_green_pin.toggle();
      }else if(counter % PATCH_INDICATOR_COUNTER == 0){
	if(counter >= PATCH_RESET_COUNTER + 5*2*PATCH_INDICATOR_COUNTER){
	  counter -= 4*PATCH_INDICATOR_COUNTER;
	  led_red_pin.toggle();
	}else{
	  led_green_pin.toggle();
	}
      }
    // if(isPushbuttonPressed()){ // exit configure mode
    //   if(program.getProgramIndex() != patchselect &&
    // 	 registry.hasPatch(patchselect)){
    // 	// change patch on mode button release
    // 	program.loadProgram(patchselect); // enters load mode (calls onChangeMode)
    // 	program.resetProgram(false);
    //   }else{
    // 	owl.setOperationMode(RUN_MODE);
    //   }
    // }
    break;
  case ERROR_MODE:
    setLed(0, counter > PATCH_RESET_COUNTER/2 ? RED_COLOUR : NO_COLOUR);
    if(counter-- == 0)
      counter = PATCH_RESET_COUNTER;
    break;
  }
}
