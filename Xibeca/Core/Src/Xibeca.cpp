#include "Owl.h"
#include "device.h"
#include "errorhandlers.h"
#include "Pin.h"
#include "ApplicationSettings.h"

#ifdef USE_SCREEN
#include "Graphics.h"
Graphics graphics;
#endif

extern "C" void onResourceUpdate(void);

void setLed(uint8_t led, uint32_t rgb){
}

void onResourceUpdate(void){
}

void setup(){

#ifdef USE_SCREEN
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); // OLED off
  extern SPI_HandleTypeDef OLED_SPI;
  graphics.begin(&OLED_SPI);
#endif
  

  owl.setup();
}

void loop(void){

#ifdef USE_SCREEN
  graphics.draw();
  graphics.display();
#endif /* USE_SCREEN */

  owl.loop();
}
