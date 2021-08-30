#include "Owl.h"
#include "device.h"
#include "errorhandlers.h"
#include "Pin.h"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"

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

#endif

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

// Pin led_d(PIN30_GPIO_Port, PIN30_Pin);
// Pin led_d(XIBECA_PIN30);

// Pin hard_sync_a(PIN17_GPIO_Port, PIN17_Pin);
// Pin hard_sync_b(PIN19_GPIO_Port, PIN19_Pin);
// Pin hard_sync_c(PIN21_GPIO_Port, PIN21_Pin);
// Pin hard_sync_d(PIN22_GPIO_Port, PIN22_Pin);


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
      break;
  case SYNC_OUT_B_Pin:
      break;
  case SYNC_OUT_C_Pin:
      break;
  case SYNC_OUT_D_Pin:
      break;
  }
}

void setGateValue(uint8_t ch, int16_t value){
  int idx = 1 + ch - BUTTON_A;
  setLed(idx, value ? RED_COLOUR : NO_COLOUR);
    // switch(ch){
    // case BUTTON_A:
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
  led_tune.outputMode();
  led_mod.outputMode();
  led_notes.outputMode();
  led_cv.outputMode();
  led_a.outputMode();
  led_b.outputMode();
  led_c.outputMode();
  led_d.outputMode();
  led_options.outputMode();

  push_tune.inputMode();
  push_mod.inputMode();
  push_notes.inputMode();
  push_cv.inputMode();
  push_a.inputMode();
  push_b.inputMode();
  push_c.inputMode();
  push_d.inputMode();
  push_options.inputMode();

  push_tune.setPull(PIN_PULL_UP);
  push_mod.setPull(PIN_PULL_UP);
  push_notes.setPull(PIN_PULL_UP);
  push_cv.setPull(PIN_PULL_UP);
  push_a.setPull(PIN_PULL_UP);
  push_b.setPull(PIN_PULL_UP);
  push_c.setPull(PIN_PULL_UP);
  push_d.setPull(PIN_PULL_UP);
  options.setPull(PIN_PULL_UP);
  
#ifdef USE_SCREEN
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); // OLED off
  extern SPI_HandleTypeDef OLED_SPI;
  graphics.begin(&OLED_SPI);
#endif
  

  owl.setup();
}

void loop(void){
  if(push_a.get())
    led_a.low();
  else
    led_a.high();
  led_b.set(push_b.get());
  led_c.set(push_c.get());
  led_d.set(push_d.get());

#ifdef USE_SCREEN
  graphics.draw();
  graphics.display();
#endif /* USE_SCREEN */

  owl.loop();
}
