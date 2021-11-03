#include "Owl.h"
#include "device.h"
#include "errorhandlers.h"
#include "Pin.h"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"
#include "XibecaParameterController.hpp"
#include "message.h"

#if 0
#define XIBECA_PIN3  GPIOD, GPIO_PIN_2
#define XIBECA_PIN4  GPIOG, GPIO_PIN_10
#define XIBECA_PIN5  GPIOC, GPIO_PIN_9
#define XIBECA_PIN6  GPIOC, GPIO_PIN_8
#define XIBECA_PIN7  GPIOC, GPIO_PIN_13
#define XIBECA_PIN8  GPIOI, GPIO_PIN_11
#define XIBECA_PIN9  GPIOC, GPIO_PIN_14
#define XIBECA_PIN10 GPIOA, GPIO_PIN_0
#define XIBECA_PIN11 GPIOH, GPIO_PIN_4
#define XIBECA_PIN12 GPIOC, GPIO_PIN_15
#define XIBECA_PIN13 GPIOA, GPIO_PIN_7
#define XIBECA_PIN14 GPIOA, GPIO_PIN_3

#define PUSH_A_Pin GPIO_PIN_0
#define PUSH_A_GPIO_Port GPIOA
#define PUSH_B_Pin GPIO_PIN_4
#define PUSH_B_GPIO_Port GPIOB
#define PUSH_C_Pin GPIO_PIN_6
#define PUSH_C_GPIO_Port GPIOA
#define PUSH_D_Pin GPIO_PIN_4
#define PUSH_D_GPIO_Port GPIOC

#define PUSH_TUNE_Pin GPIO_PIN_10
#define PUSH_TUNE_GPIO_Port GPIOG
#define PUSH_MOD_Pin GPIO_PIN_15
#define PUSH_MOD_GPIO_Port GPIOC
#define PUSH_NOTES_Pin GPIO_PIN_13
#define PUSH_NOTES_GPIO_Port GPIOB
#define PUSH_CV_Pin GPIO_PIN_14
#define PUSH_CV_GPIO_Port GPIOB
#endif

#define SYNC_OUT_A_Pin GPIO_PIN_2
#define SYNC_OUT_A_GPIO_Port GPIOD
#define SYNC_OUT_B_Pin GPIO_PIN_9
#define SYNC_OUT_B_GPIO_Port GPIOC
#define SYNC_OUT_C_Pin GPIO_PIN_13
#define SYNC_OUT_C_GPIO_Port GPIOC
#define SYNC_OUT_D_Pin GPIO_PIN_14
#define SYNC_OUT_D_GPIO_Port GPIOC

#ifdef USE_SCREEN
#include "Graphics.h"
Graphics graphics;
#endif
static XibecaParameterController params;

#define ENCODER_TIM1 htim4
extern TIM_HandleTypeDef ENCODER_TIM1;

// void updateEncoders(){
//   static int16_t encoder_values[2] = {INT16_MAX/2, INT16_MAX/2};
//   int16_t value = __HAL_TIM_GET_COUNTER(&ENCODER_TIM1);
//   int16_t delta = value - encoder_values[0];
//   if(delta)
//     graphics.params.encoderChanged(0, delta);
//   encoder_values[0] = value;
//   value = __HAL_TIM_GET_COUNTER(&ENCODER_TIM2);
//   delta = value - encoder_values[1];
//   if(delta)
//     graphics.params.encoderChanged(1, delta);
//   encoder_values[1] = value;
// }
//   updateEncoders();

extern "C" void onResourceUpdate(void);

void onResourceUpdate(void){
}

Pin led_tune(GPIOC, GPIO_PIN_8);
Pin led_mod(GPIOA, GPIO_PIN_3);
Pin led_notes(GPIOA, GPIO_PIN_7);
Pin led_cv(GPIOH, GPIO_PIN_4);
Pin led_a(GPIOI, GPIO_PIN_11);
Pin led_b(GPIOG, GPIO_PIN_13);
Pin led_c(GPIOA, GPIO_PIN_5);
Pin led_d(GPIOB, GPIO_PIN_0);
Pin led_options(GPIOB, GPIO_PIN_6);

Pin push_tune(GPIOG, GPIO_PIN_10);
Pin push_mod(GPIOC, GPIO_PIN_15);
Pin push_notes(GPIOB, GPIO_PIN_13);
Pin push_cv(GPIOB, GPIO_PIN_14);
Pin push_a(GPIOA, GPIO_PIN_0);
Pin push_b(GPIOB, GPIO_PIN_4);
Pin push_c(GPIOA, GPIO_PIN_6);
Pin push_d(GPIOC, GPIO_PIN_4);
Pin push_options(GPIOB, GPIO_PIN_5);
Pin push_enc(GPIOB, GPIO_PIN_12);

// Pin led_d(PIN30_GPIO_Port, PIN30_Pin);
// Pin led_d(XIBECA_PIN30);

// Pin hard_sync_a(PIN17_GPIO_Port, PIN17_Pin);
// Pin hard_sync_b(PIN19_GPIO_Port, PIN19_Pin);
// Pin hard_sync_c(PIN21_GPIO_Port, PIN21_Pin);
// Pin hard_sync_d(PIN22_GPIO_Port, PIN22_Pin);

Pin hard_sync_a(GPIOG, GPIO_PIN_12);
Pin hard_sync_b(GPIOA, GPIO_PIN_15);
Pin hard_sync_c(GPIOC, GPIO_PIN_6);
Pin hard_sync_d(GPIOD, GPIO_PIN_7);

void setLed(uint8_t led, uint32_t rgb){
  bool value = rgb;
  switch(led){
  case 1:
    led_tune.set(value);
    break;
  case 2:
    led_mod.set(value);
    break;
  case 3:
    led_notes.set(value);
    break;
  case 4:
    led_cv.set(value);
    break;
  case 5:
    led_a.set(value);
    break;
  case 6:
    led_b.set(value);
    break;
  case 7:
    led_c.set(value);
    break;
  case 8:
    led_d.set(value);
    break;
  case 9:
    led_options.set(value);
    break;
  }
}

uint16_t adc = 0;
extern "C"{
  void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
    adc = hadc->State;
    // 
  }
  void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc){
    error(CONFIG_ERROR, "ADC error");
  }
}

#define MIN_PERIOD ((AUDIO_SAMPLINGRATE * ARM_CYCLES_PER_SAMPLE) / 32000)
#define MAX_PERIOD ((AUDIO_SAMPLINGRATE * ARM_CYCLES_PER_SAMPLE) / 20)
static uint32_t periods[4] = {};
void updatePeriod(uint8_t idx){
  static uint32_t times[4] = {};
  uint32_t previous = times[idx];
  uint32_t now = DWT->CYCCNT;
  if(now > previous){
    uint32_t elapsed = now - previous;
    if(elapsed > MIN_PERIOD && elapsed < MAX_PERIOD){
      periods[idx] = elapsed;
    // AUDIO_SAMPLINGRATE
    //   ARM_CYCLES_PER_SAMPLE  
      setParameterValue(PARAMETER_BB + idx, (AUDIO_SAMPLINGRATE * ARM_CYCLES_PER_SAMPLE) / elapsed);
      // led_tune.toggle();
    }
  }
  times[idx] = now;
}

void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len){
  parameter_values[0] = (parameter_values[0]*3 + adc_values[ADC_A])>>2;
  parameter_values[1] = (parameter_values[1]*3 + adc_values[ADC_B])>>2;
  parameter_values[2] = (parameter_values[2]*3 + adc_values[ADC_C])>>2;
  parameter_values[3] = (parameter_values[3]*3 + adc_values[ADC_D])>>2;
}

void onChangePin(uint16_t pin){
  switch(pin){
  // case PUSH_TUNE_Pin:
  //     break;
  // case PUSH_MOD_Pin:
  //     break;
  // case PUSH_NOTES_Pin:
  //     break;
  // case PUSH_CV_Pin:
  //     break;
  // case PUSH_A_Pin:
  //     break;
  // case PUSH_B_Pin:
  //     break;
  // case PUSH_C_Pin:
  //     break;
  // case PUSH_D_Pin:
  //     break;
  case SYNC_OUT_A_Pin:
    updatePeriod(0);
    break;
  case SYNC_OUT_B_Pin:
    updatePeriod(1);
      break;
  case SYNC_OUT_C_Pin:
    updatePeriod(2);
      break;
  case SYNC_OUT_D_Pin:
    updatePeriod(3);
      break;
  }
}

void setGateValue(uint8_t ch, int16_t value){
  int idx = 1 + ch - BUTTON_A;
  setLed(idx, value ? RED_COLOUR : NO_COLOUR);
  switch(ch){
  case BUTTON_E:
    hard_sync_a.set(value ? true : false);
    break;
  case BUTTON_F:
    hard_sync_b.set(value ? true : false);
    break;
  case BUTTON_G:
    hard_sync_c.set(value ? true : false);
    break;
  case BUTTON_H:
    hard_sync_d.set(value ? true : false);
    break;
  }
    // switch(ch){
    //   setLed(1, value ? RED_COLOUR : NO_COLOUR);
    //   break;
    // case BUTTON_B:
    //   setLed(2, value ? RED_COLOUR : NO_COLOUR);
    //   break;
    // case BUTTON_C:
    //   setLed(3, value ? RED_COLOUR : NO_COLOUR);
    //   break;
    // case BUTTON_D:
    //   setLed(4, value ? RED_COLOUR : NO_COLOUR);
    //   break;
    // }
}

void setup(){
  // DWT cycle count enable
  // do it here because DEBUG_DWT is disabled
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  
  // __HAL_TIM_SET_COUNTER(&ENCODER_TIM1, INT16_MAX/2);
  __HAL_TIM_SET_COUNTER(&ENCODER_TIM1, 2048);
  HAL_TIM_Encoder_Start_IT(&ENCODER_TIM1, TIM_CHANNEL_ALL);

  led_tune.outputMode();
  led_mod.outputMode();
  led_notes.outputMode();
  led_cv.outputMode();
  led_a.outputMode();
  led_b.outputMode();
  led_c.outputMode();
  led_d.outputMode();
  led_options.outputMode();

  hard_sync_a.outputMode();
  hard_sync_b.outputMode();
  hard_sync_c.outputMode();
  hard_sync_d.outputMode();
  
  push_tune.inputMode();
  push_mod.inputMode();
  push_notes.inputMode();
  push_cv.inputMode();
  push_a.inputMode();
  push_b.inputMode();
  push_c.inputMode();
  push_d.inputMode();
  push_options.inputMode();
  push_enc.inputMode();

  push_tune.setPull(PIN_PULL_UP);
  push_mod.setPull(PIN_PULL_UP);
  push_notes.setPull(PIN_PULL_UP);
  push_cv.setPull(PIN_PULL_UP);
  push_a.setPull(PIN_PULL_UP);
  push_b.setPull(PIN_PULL_UP);
  push_c.setPull(PIN_PULL_UP);
  push_d.setPull(PIN_PULL_UP);
  push_options.setPull(PIN_PULL_UP);
  push_enc.setPull(PIN_PULL_UP);
  
#ifdef USE_SCREEN
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); // OLED off
  extern SPI_HandleTypeDef OLED_SPI;
  graphics.begin(&params, &OLED_SPI);
#endif

  owl.setup();
}

void loop(void){

  led_tune.set(!push_tune.get());
  led_mod.set(!push_mod.get());
  led_notes.set(!push_notes.get());
  led_cv.set(!push_cv.get());
  led_options.set(!push_options.get());
  led_a.set(!push_a.get());
  led_b.set(!push_b.get());
  led_c.set(!push_c.get());
  led_d.set(!push_d.get());

  setButtonValue(BUTTON_A, !push_enc.get());
  setParameterValue(PARAMETER_BA, __HAL_TIM_GET_COUNTER(&ENCODER_TIM1) / 2);

#ifdef USE_SCREEN
  graphics.draw();
  graphics.display();
#endif /* USE_SCREEN */

  owl.loop();
}
