#include "Owl.h"
#include "Graphics.h"

Graphics graphics;

extern "C"{
  void eeprom_unlock(){}
  int eeprom_write_block(uint32_t address, void* data, uint32_t size)
  {return 0;}
   int eeprom_write_word(uint32_t address, uint32_t data)
  {return 0;}
   int eeprom_write_byte(uint32_t address, uint8_t data)
  {return 0;}
   int eeprom_erase(uint32_t address)
  {return 0;}

   int eeprom_wait()
  {return 0;}
   int eeprom_erase_sector(uint32_t sector)
  {return 0;}

void setPortMode(uint8_t index, uint8_t mode){}
uint8_t getPortMode(uint8_t index){
  return 0;
}
}  

void setup(){
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); // OLED off
  extern SPI_HandleTypeDef OLED_SPI;
  graphics.begin(&OLED_SPI);

#ifdef USE_USB_HOST
  // enable USB Host power
  HAL_GPIO_WritePin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin, GPIO_PIN_SET);
#endif

  owl.setup();
}

void loop(void){

#ifdef USE_USB_HOST
  if(HAL_GPIO_ReadPin(USB_HOST_PWR_FAULT_GPIO_Port, USB_HOST_PWR_FAULT_Pin) == GPIO_PIN_RESET){
    if(HAL_GPIO_ReadPin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin) == GPIO_PIN_SET){
      HAL_GPIO_WritePin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin, GPIO_PIN_RESET);
      error(USB_ERROR, "USBH PWR Fault");
    }
  }else{
    MX_USB_HOST_Process();
  }
#endif
  // graphics.params.updateEncoders(Encoders_get(), 7);
}
