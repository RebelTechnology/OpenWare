#include "Owl.h"

#include "OpenWareMidiControl.h"
#include "Graphics.h"
#include "Pin.h"
#include "GeniusParameterController.hpp"

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
  int eeprom_erase_sector(uint32_t sector, uint32_t bank)
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

Pin tr_out_a_pin(GPIOD, GPIO_PIN_4);
Pin tr_out_b_pin(GPIOD, GPIO_PIN_3);

GeniusParameterController params;
Graphics graphics DMA_RAM;

char* progress_message = NULL;
uint16_t progress_counter = 0;

void setProgress(uint16_t value, const char* reason){
  progress_message = (char*)reason;
  progress_counter = value;
}

void onChangeMode(uint8_t new_mode, uint8_t old_mode){
  switch(new_mode){
  case STARTUP_MODE:
  case STREAM_MODE:
  case LOAD_MODE:
    setDisplayMode(PROGRESS_DISPLAY_MODE);
    break;
  case CONFIGURE_MODE:
    setDisplayMode(CONFIGURATION_DISPLAY_MODE);
    break;
  case RUN_MODE:
    setDisplayMode(STANDARD_DISPLAY_MODE);
    break;
  case ERROR_MODE:
    setDisplayMode(ERROR_DISPLAY_MODE);
    break;
  }
}

void onSetup(){
  progress_counter = 1000;
  tr_out_a_pin.outputMode();
  tr_out_b_pin.outputMode();
  tr_out_a_pin.high();
  tr_out_b_pin.high();
  setAnalogValue(PARAMETER_F, 0);
  setAnalogValue(PARAMETER_G, 0);
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); // OLED off
  extern SPI_HandleTypeDef OLED_SPI;
  graphics.begin(&params, &OLED_SPI);
  progress_counter = 2000;
#ifdef USE_USB_HOST
  // enable USB Host power
  HAL_GPIO_WritePin(USB_HOST_PWR_EN_GPIO_Port, USB_HOST_PWR_EN_Pin, GPIO_PIN_SET);
#endif
  __HAL_TIM_SET_COUNTER(&ENCODER_TIM1, INT16_MAX/2);
  __HAL_TIM_SET_COUNTER(&ENCODER_TIM2, INT16_MAX/2);
  HAL_TIM_Encoder_Start_IT(&ENCODER_TIM1, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start_IT(&ENCODER_TIM2, TIM_CHANNEL_ALL);
  progress_counter = 3000;
}

void setGateValue(uint8_t ch, int16_t value){
  switch(ch){
  case PUSHBUTTON:
  case BUTTON_1:
    tr_out_a_pin.set(!value);
    break;
  case BUTTON_2:
    tr_out_b_pin.set(!value);
    break;
  }
}

// 12x12 bit multiplication with unsigned operands and result
#define U12_MUL_U12(a,b) (__USAT(((uint32_t)(a)*(b))>>12, 12))
static uint16_t scaleForDac(int16_t value){
  // return U12_MUL_U12(value + 70, 3521);
  return value;
}

void setAnalogValue(uint8_t ch, int16_t value){
  // if(owl.getOperationMode() == RUN_MODE){
    extern DAC_HandleTypeDef hdac;
    switch(ch){
    case PARAMETER_F:
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, scaleForDac(value));
      break;
    case PARAMETER_G:
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, scaleForDac(value));
      break;
    }
  // }
}

void onChangePin(uint16_t pin){
  switch(pin){
  case TR_IN_A_Pin:
  case SW_A_Pin: {
    bool state = HAL_GPIO_ReadPin(SW_A_GPIO_Port, SW_A_Pin) == GPIO_PIN_RESET;
    state |= HAL_GPIO_ReadPin(TR_IN_A_GPIO_Port, TR_IN_A_Pin) == GPIO_PIN_RESET;
    setButtonValue(PUSHBUTTON, state);
    setButtonValue(BUTTON_A, state);
    // midi_tx.sendCc(PATCH_BUTTON, state ? 127 : 0);
    break;
  }
  case TR_IN_B_Pin:
  case SW_B_Pin: {
    bool state = HAL_GPIO_ReadPin(SW_B_GPIO_Port, SW_B_Pin) == GPIO_PIN_RESET;
    state |= HAL_GPIO_ReadPin(TR_IN_B_GPIO_Port, TR_IN_B_Pin) == GPIO_PIN_RESET;
    setButtonValue(BUTTON_B, state);
    break;
  }
  }
}

extern "C"{
  static uint16_t smooth_adc_values[NOF_ADC_VALUES];
  void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
    // this runs at apprx 3.3kHz
    // with 64.5 cycles sample time, 30 MHz ADC clock, and ClockPrescaler = 32
    extern uint16_t adc_values[NOF_ADC_VALUES];
    for(size_t i=0; i<NOF_ADC_VALUES; ++i){
      // IIR exponential filter with lambda 0.75: y[n] = 0.75*y[n-1] + 0.25*x[n]
      smooth_adc_values[i] = (smooth_adc_values[i]*3 + adc_values[i]) >> 2;
    }
    // tr_out_a_pin.toggle();
  }
  void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc){
    error(CONFIG_ERROR, "ADC error");
  }
  void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len){
    params.updateValues((int16_t*)smooth_adc_values, adc_len);
  }
}

void updateEncoders(){
  int16_t encoder_values[2] = {(int16_t)__HAL_TIM_GET_COUNTER(&ENCODER_TIM1),
                               (int16_t)__HAL_TIM_GET_COUNTER(&ENCODER_TIM2)};
  params.updateEncoders(encoder_values, 2);
  // static int16_t encoder_values[2] = {INT16_MAX/2, INT16_MAX/2};
  // int16_t value = __HAL_TIM_GET_COUNTER(&ENCODER_TIM1);
  // int16_t delta = value - encoder_values[0];
  // if(delta)
  //   graphics.params.encoderChanged(0, delta);
  // encoder_values[0] = value;
  // value = __HAL_TIM_GET_COUNTER(&ENCODER_TIM2);
  // delta = value - encoder_values[1];
  // if(delta)
  //   graphics.params.encoderChanged(1, delta);
  // encoder_values[1] = value;
}

void onLoop(void){
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
}

#ifdef DEBUG_USBD_AUDIO
void defaultDrawCallback(uint8_t* pixels, uint16_t width, uint16_t height){
  extern int usbd_tx_flow;
  extern int usbd_rx_flow;
  extern int usbd_tx_capacity;
  extern int usbd_rx_capacity;
  ScreenBuffer& screen = graphics.screen;
  params.drawTitle(screen);
  params.drawMessage(26, screen);

  screen.setTextSize(1);
  screen.print(2, 36, "rx ");
  screen.print(usbd_rx_flow);
  screen.print(" / ");
  screen.print(usbd_rx_capacity);
  screen.print(2, 46, "tx ");
  screen.print(usbd_tx_flow);
  screen.print(" / ");
  screen.print(usbd_tx_capacity);
}
// #else
// void defaultDrawCallback(uint8_t* pixels, uint16_t width, uint16_t height){
//   ScreenBuffer& screen = graphics.screen;
//   params.drawTitle(screen);
//   params.drawMessage(26, screen);

//   screen.setTextSize(1);
//   encoder_values
// }
#endif
