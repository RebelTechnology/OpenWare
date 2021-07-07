#include "Owl.h"
#include "device.h"
#include "errorhandlers.h"
#include "Graphics.h"
#include "Pin.h"
#include "ApplicationSettings.h"

Graphics graphics;

extern "C" void onResourceUpdate(void);

void setLed(uint8_t led, uint32_t rgb){
}

void onResourceUpdate(void){
}

void setup(){

  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); // OLED off
  extern SPI_HandleTypeDef OLED_SPI;
  graphics.begin(&OLED_SPI);

// #ifdef USE_USB_HOST
//   // enable USB Host power
//   HAL_GPIO_WritePin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin, GPIO_PIN_SET);
// #endif

  owl.setup();
}

void loop(void){

// #ifdef USE_USB_HOST
//   if(HAL_GPIO_ReadPin(USB_HOST_PWR_FAULT_GPIO_Port, USB_HOST_PWR_FAULT_Pin) == GPIO_PIN_RESET){
//     if(HAL_GPIO_ReadPin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin) == GPIO_PIN_SET){
//       HAL_GPIO_WritePin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin, GPIO_PIN_RESET);
//       error(USB_ERROR, "USBH PWR Fault");
//     }
//   }else{
//     MX_USB_HOST_Process();
//   }
// #endif

#ifdef USE_SCREEN
  graphics.draw();
  graphics.display();
#endif /* USE_SCREEN */

  owl.loop();
}
