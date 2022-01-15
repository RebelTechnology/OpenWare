#include <math.h>
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

#define PATCH_RESET_COUNTER (768/MAIN_LOOP_SLEEP_MS)
#define PATCH_CONFIG_COUNTER (3072/MAIN_LOOP_SLEEP_MS)
#define PATCH_INDICATOR_COUNTER (256/MAIN_LOOP_SLEEP_MS)

#define BYPASS_MODE       (ERROR_MODE+1)
#define ENTER_CONFIG_MODE (ERROR_MODE+2)

static int16_t knobvalues[2];
static uint8_t patchselect;
#define PATCH_CONFIG_KNOB_THRESHOLD (4096/8)
#define PATCH_CONFIG_PROGRAM_CONTROL ADC_A
#define PATCH_CONFIG_VOLUME_CONTROL  ADC_D

#ifdef OWL_MODULAR
/* analogue values A, B, C, D are inverted on OWL Modular */
#define GET_ABCD(x) (4095 - getAnalogValue(x))
#else
#define GET_ABCD(x) (getAnalogValue(x))
#endif

#if 0
#define SW3_Pin EXP2_T_Pin
#define SW3_GPIO_Port EXP2_T_GPIO_Port
#define SW4_Pin EXP2_R_Pin
#define SW4_GPIO_Port EXP2_R_GPIO_Port

Pin bypass_pin(GPIOA, GPIO_PIN_0);
Pin bufpass_pin(GPIOF, GPIO_PIN_9); // high is bypass
#endif

Pin exp1_tip_pin(EXP1_TIP_GPIO_Port, EXP1_TIP_Pin);
Pin exp1_ring_pin(EXP1_RING_GPIO_Port, EXP1_RING_Pin);

#ifndef OWL_PEDAL_PWM_LEDS
Pin led_green_pin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
Pin led_red_pin(LED_RED_GPIO_Port, LED_RED_Pin);
#endif

// Pin footswitch_pin(GPIOA, GPIO_PIN_0);

static uint32_t counter = 0;
static uint8_t expression_mode = 0;

#if 0
bool getBufferedBypass(){
  return !bufpass_pin.get();
}

void setBufferedBypass(bool value){
  // setLed(0, value ? RED_COLOUR : GREEN_COLOUR);
  bufpass_pin.set(!value);
  // todo; set BYPASS_MODE
}
#endif

bool isBypassed(){
#ifdef OWL_PEDAL_LEGACY
  return HAL_GPIO_ReadPin(FOOTSWITCH_GPIO_Port, FOOTSWITCH_Pin) == GPIO_PIN_SET;
#else
  return HAL_GPIO_ReadPin(FOOTSWITCH_GPIO_Port, FOOTSWITCH_Pin) == GPIO_PIN_RESET;
#endif
  // todo: || getBufferedBypass()
}

bool isPushbuttonPressed(){
  return HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET
#ifdef SW1_ALT_GPIO_Port
    || HAL_GPIO_ReadPin(SW1_ALT_GPIO_Port, SW1_ALT_Pin) == GPIO_PIN_RESET
#endif
    ;
}

#ifdef OWL_PEDAL
void configureExpression(uint8_t mode){
  if(mode != expression_mode){
    switch(mode){
    case EXPRESSION_MODE_EXP_TRS:
      // Ring is available on ADC_F
      exp1_ring_pin.outputMode();
      exp1_ring_pin.high();
      exp1_ring_pin.setPull(PIN_PULL_NONE);
      exp1_tip_pin.analogMode();
      exp1_tip_pin.setPull(PIN_PULL_NONE);
      break;
    case EXPRESSION_MODE_EXP_RTS:
      exp1_tip_pin.outputMode();
      exp1_tip_pin.high();
      exp1_tip_pin.setPull(PIN_PULL_NONE);
      exp1_ring_pin.analogMode();
      exp1_ring_pin.setPull(PIN_PULL_NONE);
      break;
    case EXPRESSION_MODE_FS_TS:
    case EXPRESSION_MODE_FS_TRS:
      exp1_tip_pin.inputMode();
      exp1_tip_pin.setPull(PIN_PULL_UP);
      exp1_ring_pin.inputMode();
      exp1_ring_pin.setPull(PIN_PULL_UP);
      break;
    }
    expression_mode = mode;
    setButtonValue(PUSHBUTTON, isPushbuttonPressed());
    setButtonValue(BUTTON_1, isPushbuttonPressed());
    setButtonValue(BUTTON_2, false);
  }
}
#endif

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

uint32_t getLed(uint8_t led){
#ifdef OWL_PEDAL_PWM_LEDS
  uint32_t r = 1023 - TIM4->CCR3;
  uint32_t g = TIM4->CCR4;
  return (r<<20) | (g<<10);
#else
  if(!led_green_pin.get() && led_red_pin.get())
    return RED_COLOUR;
  else if(led_green_pin.get() && !led_red_pin.get())
    return GREEN_COLOUR;
  else
    return NO_COLOUR;
#endif
}

void toggleLed(uint8_t led){
#ifdef OWL_PEDAL_PWM_LEDS
  setLed(led, getLed(led) == GREEN_COLOUR ? RED_COLOUR : GREEN_COLOUR);
  // uint32_t r = TIM4->CCR3;
  // uint32_t g = TIM4->CCR4;
  // TIM4->CCR3 = 1023 - r;
  // TIM4->CCR4 = 1023 - g;
#else
  led_red_pin.set(led_green_pin.get());
  led_green_pin.toggle();      
#endif
}
    
void setLed(uint8_t led, uint32_t rgb){
#ifdef OWL_PEDAL_PWM_LEDS
  uint32_t r = 1023 - ((rgb>>20) & 0x3ff);
  uint32_t g = ((rgb>>10) & 0x3ff); // green TIM4 has reverse polarity
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

void onChangePin(uint16_t pin){
  if(pin == FOOTSWITCH_Pin){
    setButtonValue(0, isBypassed());
  }
  if(owl.getOperationMode() == RUN_MODE){
    switch(pin){
    // case FOOTSWITCH_Pin: { // bypass / stomp switch
    //   bool state = isBypassed();
    //   setButtonValue(0, state);
    //   setLed(0, state ? NO_COLOUR : GREEN_COLOUR);
    //   break;
    // }
#ifdef SW1_ALT_Pin
    case SW1_ALT_Pin:
#endif
    case SW1_Pin: { // pushbutton
	bool state = isPushbuttonPressed();
	setButtonValue(PUSHBUTTON, state);
	setButtonValue(BUTTON_1, state);
	setLed(0, state ? RED_COLOUR : GREEN_COLOUR);
	midi_tx.sendCc(PATCH_BUTTON, state ? 127 : 0);
	break;
      }
#ifdef OWL_MODULAR
    case PUSH_GATE_IN_Pin: {
      bool state = !(PUSH_GATE_IN_GPIO_Port->IDR & PUSH_GATE_IN_Pin);
      setButtonValue(PUSHBUTTON, state);
      setButtonValue(BUTTON_1, state);
      setLed(0, state ? RED_COLOUR : GREEN_COLOUR);
      break;
    }
#endif
    // case SW2_Pin: { // mode button
    //   bool state = HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_RESET;
    //   setButtonValue(BUTTON_2, state);
    //   break;
    // }
    // case SW3_Pin: { // EXP2 Tip
    //   bool state = HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin) == GPIO_PIN_RESET;
    //   setButtonValue(BUTTON_3, state);
    //   break;
    // }
    // case SW4_Pin: { // EXP2 Ring
    //   bool state = HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == GPIO_PIN_RESET;
    //   setButtonValue(BUTTON_4, state);
    //   break;
    // }
    }
  }
}

void setGateValue(uint8_t ch, int16_t value){
  if(owl.getOperationMode() == RUN_MODE){
    value = (value >> 2) & 0x3ff; // use top 10 bits of 12
    switch(ch){
    case BUTTON_1:
    case PUSHBUTTON:
#ifdef OWL_PEDAL_PWM_LEDS
      setLed(0, (value << 20) | ((1023 - value) << 10));
#else
      setLed(0, value < 512 ? GREEN_COLOUR : RED_COLOUR);
#endif
#ifdef OWL_MODULAR
      HAL_GPIO_WritePin(PUSH_GATE_OUT_GPIO_Port, PUSH_GATE_OUT_Pin, value ? GPIO_PIN_RESET : GPIO_PIN_SET);
#endif
      break;
    case GREEN_BUTTON:
#ifdef OWL_PEDAL_PWM_LEDS
      setLed(0, value << 10);
#else
      setLed(0, value < 512 ? NO_COLOUR : GREEN_COLOUR);
#endif
      break;
    case RED_BUTTON:
#ifdef OWL_PEDAL_PWM_LEDS
      setLed(0, value << 20);
#else
      setLed(0, value < 512 ? NO_COLOUR : RED_COLOUR);
#endif
      break;
    }
  }
  // if(ch == 0)
  //   setBufferedBypass(value);
}

void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len){
  if(owl.getOperationMode() == RUN_MODE){
#ifdef OWL_MODULAR
    parameter_values[0] = (parameter_values[0]*3 + 4095 - adc_values[ADC_A])>>2;
    parameter_values[1] = (parameter_values[1]*3 + 4095 - adc_values[ADC_B])>>2;
    parameter_values[2] = (parameter_values[2]*3 + 4095 - adc_values[ADC_C])>>2;
    parameter_values[3] = (parameter_values[3]*3 + 4095 - adc_values[ADC_D])>>2;
    parameter_values[4] = (parameter_values[4]*3 + adc_values[ADC_E])>>2;
#else
    parameter_values[0] = (parameter_values[0]*3 + adc_values[ADC_A])>>2;
    parameter_values[1] = (parameter_values[1]*3 + adc_values[ADC_B])>>2;
    parameter_values[2] = (parameter_values[2]*3 + adc_values[ADC_C])>>2;
    parameter_values[3] = (parameter_values[3]*3 + adc_values[ADC_D])>>2;

    static uint8_t fs_state = 0;
    uint8_t buttons = 0;
    switch(expression_mode){
    case EXPRESSION_MODE_EXP_TRS:
      parameter_values[4] = (parameter_values[4]*3 + adc_values[ADC_E])>>2;
      break;
    case EXPRESSION_MODE_EXP_ITRS:
      parameter_values[4] = (parameter_values[4]*3 + 4095 - adc_values[ADC_E])>>2;
      break;
    case EXPRESSION_MODE_EXP_RTS:
      parameter_values[4] = (parameter_values[4]*3 + adc_values[ADC_F])>>2;
      break;
    case EXPRESSION_MODE_EXP_IRTS:
      parameter_values[4] = (parameter_values[4]*3 + 4095 - adc_values[ADC_F])>>2;
      break;
    case EXPRESSION_MODE_FS_TRS:
      buttons = (!exp1_ring_pin.get()) << BUTTON_2;
      buttons |= (!exp1_tip_pin.get()) << BUTTON_1;
      if(buttons != fs_state){
	if((buttons & (1 << BUTTON_2)) != (fs_state & (1 << BUTTON_2))){
	  setButtonValue(BUTTON_2, buttons & (1 << BUTTON_2));
	}
	if((buttons & (1 << BUTTON_1)) != (fs_state & (1 << BUTTON_1))){
	  setButtonValue(BUTTON_1, buttons & (1 << BUTTON_1));
	  setButtonValue(PUSHBUTTON, buttons & (1 << BUTTON_1));
	}
	fs_state = buttons;
      }
      break;
    case EXPRESSION_MODE_FS_TS:
      buttons = (!exp1_tip_pin.get()) << BUTTON_1;
      if(buttons != fs_state){
	setButtonValue(BUTTON_1, buttons & (1 << BUTTON_1));
	setButtonValue(PUSHBUTTON, buttons & (1 << BUTTON_1));
	fs_state = buttons;
      }
      break;
    }
#endif
  }
}

void onSetup(){
  initLed();
#if 0
  bypass_pin.outputMode();
  bypass_pin.low();
  bufpass_pin.outputMode();
  bufpass_pin.low();
  setBufferedBypass(false);
#endif
#ifdef OWL_PEDAL
  configureExpression(settings.expression_mode);
#endif
  setLed(0, RED_COLOUR);

  MX_USB_DEVICE_Init();  

}

void onStartProgram(){
  setLed(0, GREEN_COLOUR);
}

void onChangeMode(uint8_t new_mode, uint8_t old_mode){
  counter = 0;
  static uint32_t saved_led = NO_COLOUR;
  if(old_mode == RUN_MODE){
    saved_led = getLed(0); // leaving RUN_MODE, save LED state
  }
  setLed(0, NO_COLOUR);
  if(new_mode == CONFIGURE_MODE){
    knobvalues[0] = GET_ABCD(PATCH_CONFIG_PROGRAM_CONTROL);
    knobvalues[1] = GET_ABCD(PATCH_CONFIG_VOLUME_CONTROL);
    patchselect = program.getProgramIndex();
  }else if(new_mode == RUN_MODE){
    setLed(0, saved_led); // restore to saved LED state
  }
}

// static uint16_t progress = 0;
void setProgress(uint16_t value, const char* msg){
#ifdef OWL_PEDAL_PWM_LEDS
  setLed(0, (value<<18) & (0x3ff<<20));
#else
  toggleLed(0);
#endif
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
    // setLed(0, counter > PATCH_INDICATOR_COUNTER/2 ? GREEN_COLOUR : NO_COLOUR);
    // todo: use setProgress()?
    // if(counter-- == 0)
    //   counter = PATCH_RESET_COUNTER;
    break;
  case BYPASS_MODE:
    if(!isBypassed()){
      owl.setOperationMode(RUN_MODE);
    }else if(isPushbuttonPressed()){
      owl.setOperationMode(ENTER_CONFIG_MODE);	
    }
    break;
  case RUN_MODE:
    if(getErrorStatus() != NO_ERROR){
      owl.setOperationMode(ERROR_MODE);
    }else if(isPushbuttonPressed()){
      if(--counter == 0){ // counter == 0 when we enter RUN_MODE
	owl.setOperationMode(ENTER_CONFIG_MODE);
      }else if(counter < PATCH_RESET_COUNTER && counter % 128 == 0){
	toggleLed(0);
      }
    }else if(isBypassed()){
      // don't change to BYPASS_MODE while pushbutton is pressed
      // (we might have just come from CONFIGURATION_MODE)
      owl.setOperationMode(BYPASS_MODE);
    }else{
      counter = PATCH_CONFIG_COUNTER;
    }
    break;
  case ENTER_CONFIG_MODE:
    if(isPushbuttonPressed()){
      // toggle rapidly for yellow-ish LED
      toggleLed(0);
    }else{
      owl.setOperationMode(CONFIGURE_MODE);
    }
    break;
  case CONFIGURE_MODE:
    if(isPushbuttonPressed()){ // exit configure mode
      if(program.getProgramIndex() != patchselect &&
	 registry.hasPatch(patchselect)){
	program.loadProgram(patchselect); // enters load mode (calls onChangeMode)
	program.resetProgram(false);
      }else{
	owl.setOperationMode(RUN_MODE);
      }
    }else{
      // update patch control
      if(abs(knobvalues[0] - GET_ABCD(PATCH_CONFIG_PROGRAM_CONTROL)) >= PATCH_CONFIG_KNOB_THRESHOLD){
	knobvalues[0] = -PATCH_CONFIG_KNOB_THRESHOLD;
	float pos = 0.5f + (GET_ABCD(PATCH_CONFIG_PROGRAM_CONTROL) * (registry.getNumberOfPatches() - 1)) / 4096.0f;
	uint8_t idx = roundf(pos);
	if(abs(patchselect - pos) > 0.6 && registry.hasPatch(idx)){ // ensure a small dead zone
	  patchselect = idx;
	  midi_tx.sendPc(patchselect);
	  // update patch indicator
	  counter = PATCH_RESET_COUNTER + PATCH_INDICATOR_COUNTER*(2*patchselect + 1);
	}
      }
      // update volume control
      if(abs(knobvalues[1] - GET_ABCD(PATCH_CONFIG_VOLUME_CONTROL)) >= PATCH_CONFIG_KNOB_THRESHOLD){
	knobvalues[1] = -PATCH_CONFIG_KNOB_THRESHOLD;
	uint8_t value = (GET_ABCD(PATCH_CONFIG_VOLUME_CONTROL) >> 6) + 63;
	if(settings.audio_output_gain != value){
	  settings.audio_output_gain = value;
	  codec.setOutputGain(value);
	  midi_tx.sendCc(MIDI_CC_VOLUME, value);
	}
      }
      // indicator LED
      if(counter-- == 0){
	counter = PATCH_RESET_COUNTER + PATCH_INDICATOR_COUNTER*(2*patchselect + 1); // patch indicator
	setLed(0, NO_COLOUR);
      }else if(counter <= PATCH_RESET_COUNTER){
	if(patchselect == 0){
	  // toggle rapidly for yellow-ish LED
	  toggleLed(0);
	}else{
	  setLed(0, NO_COLOUR);
	}
      }else if((counter - PATCH_RESET_COUNTER) % (2*PATCH_INDICATOR_COUNTER) == 0){
	if((counter - PATCH_RESET_COUNTER) >= 10*PATCH_INDICATOR_COUNTER){
	  counter -= 8*PATCH_INDICATOR_COUNTER;
	  setLed(0, RED_COLOUR);
	}else{
	  setLed(0, GREEN_COLOUR);
	}
      }else if((counter - PATCH_RESET_COUNTER) % PATCH_INDICATOR_COUNTER == 0){
	setLed(0, NO_COLOUR);
      }
    }
    break;
  case ERROR_MODE:
    if(isPushbuttonPressed())
      owl.setOperationMode(ENTER_CONFIG_MODE); 
    setLed(0, counter > PATCH_RESET_COUNTER/2 ? RED_COLOUR : NO_COLOUR);
    if(counter-- == 0)
      counter = PATCH_RESET_COUNTER;
    break;
  }
#ifdef OWL_PEDAL
  configureExpression(settings.expression_mode);
#endif
}
