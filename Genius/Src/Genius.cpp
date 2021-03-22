#include "Owl.h"

#include "Graphics.h"


extern "C"{
#if 0
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
  int eeprom_write_unlock(uint32_t wrp_sectors)
  {return 0;}
  int eeprom_write_lock(uint32_t wrp_sectors)
  {return 0;}
  uint32_t eeprom_write_protection(uint32_t wrp_sectors)
  {return 0;}
#endif
  void setPortMode(uint8_t index, uint8_t mode){}
  uint8_t getPortMode(uint8_t index){
    return 0;
  }
}  

extern TIM_HandleTypeDef ENCODER_TIM1;
extern TIM_HandleTypeDef ENCODER_TIM2;

Graphics graphics;

void setup(){
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); // OLED off
  extern SPI_HandleTypeDef OLED_SPI;
  graphics.begin(&OLED_SPI);

#ifdef USE_USB_HOST
  // enable USB Host power
  HAL_GPIO_WritePin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin, GPIO_PIN_SET);
#endif

  __HAL_TIM_SET_COUNTER(&ENCODER_TIM1, INT16_MAX/2);
  __HAL_TIM_SET_COUNTER(&ENCODER_TIM2, INT16_MAX/2);
  HAL_TIM_Encoder_Start_IT(&ENCODER_TIM1, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start_IT(&ENCODER_TIM2, TIM_CHANNEL_ALL);

  owl.setup();
}

// int16_t* Encoders_get(){
void updateEncoders(){
  static int16_t encoder_values[2] = {INT16_MAX/2, INT16_MAX/2};
  int16_t value = __HAL_TIM_GET_COUNTER(&ENCODER_TIM1);
  int16_t delta = value - encoder_values[0];
  if(delta)
    graphics.params.encoderChanged(0, delta);
  encoder_values[0] = value;
  value = __HAL_TIM_GET_COUNTER(&ENCODER_TIM2);
  delta = value - encoder_values[1];
  if(delta)
    graphics.params.encoderChanged(1, delta);
  encoder_values[1] = value;
}

  // case ENC1_SW_Pin: // GPIO_PIN_14:
  //   setButtonValue(BUTTON_A, !(ENC1_SW_GPIO_Port->IDR & ENC1_SW_Pin));
  //   setButtonValue(PUSHBUTTON, !(ENC1_SW_GPIO_Port->IDR & ENC1_SW_Pin));
  //   break;
  // case ENC2_SW_Pin: // GPIO_PIN_4:
  //   setButtonValue(BUTTON_B, !(ENC2_SW_GPIO_Port->IDR & ENC2_SW_Pin));
  //   break;
  // case TR_IN_A_Pin: // GPIO_PIN_11:
  //   setButtonValue(BUTTON_C, !(TR_IN_A_GPIO_Port->IDR & TR_IN_A_Pin));
  //   break;
  // case TR_IN_B_Pin: // GPIO_PIN_10:
  //   setButtonValue(BUTTON_D, !(TR_IN_B_GPIO_Port->IDR & TR_IN_B_Pin));
  //   break;

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
#if defined USE_DCACHE
  // SCB_CleanInvalidateDCache_by_Addr((uint32_t*)graphics.params.user, sizeof(graphics.params.user));
#endif
  updateEncoders();

#ifdef USE_SCREEN
  graphics.draw();
  graphics.display();
#endif /* USE_SCREEN */

  owl.loop();
}
