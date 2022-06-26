#include "Owl.h"

#include "OpenWareMidiControl.h"
#include "Pin.h"
#include "errorhandlers.h"
#ifdef USE_SCREEN
#include "Graphics.h"
#include "GeniusParameterController.hpp"
#endif

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

#ifdef USE_SCREEN
GeniusParameterController params;
Graphics graphics DMA_RAM;
#endif

char* progress_message = NULL;
uint16_t progress_counter = 0;

void setProgress(uint16_t value, const char* reason){
  progress_message = (char*)reason;
  progress_counter = value;
}

void onChangeMode(uint8_t new_mode, uint8_t old_mode){
#ifdef USE_SCREEN
  switch(new_mode){
  case STARTUP_MODE:
  case STREAM_MODE:
  case LOAD_MODE:
    setDisplayMode(PROGRESS_DISPLAY_MODE);
    break;
  case CONFIGURE_MODE:
    setDisplayMode(CONFIG_STATS_DISPLAY_MODE);
    break;
  case RUN_MODE:
    setDisplayMode(STANDARD_DISPLAY_MODE);
    break;
  case ERROR_MODE:
    setDisplayMode(ERROR_DISPLAY_MODE);
    break;
  }
#endif
}

void onSetup(){
  progress_counter = 1000;
  tr_out_a_pin.outputMode();
  tr_out_b_pin.outputMode();
  tr_out_a_pin.high();
  tr_out_b_pin.high();
  setAnalogValue(PARAMETER_F, 0);
  setAnalogValue(PARAMETER_G, 0);
#ifdef USE_SCREEN
  params.reset();
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); // OLED off
  extern SPI_HandleTypeDef OLED_SPI;
  graphics.begin(&params, &OLED_SPI);
#endif
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

// void setAnalogValue(uint8_t ch, int16_t value){
//   // if(owl.getOperationMode() == RUN_MODE){
// extern DAC_HandleTypeDef DAC_HANDLE;
//     switch(ch){
//     case PARAMETER_F:
//       HAL_DAC_SetValue(&DAC_HANDLE, DAC_CHANNEL_1, DAC_ALIGN_12B_R, scaleForDac(value));
//       break;
//     case PARAMETER_G:
//       HAL_DAC_SetValue(&DAC_HANDLE, DAC_CHANNEL_2, DAC_ALIGN_12B_R, scaleForDac(value));
//       break;
//     }
//   // }
// }

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
}

void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len){
#ifdef USE_SCREEN
  params.updateValues((int16_t*)smooth_adc_values, adc_len);
  extern DAC_HandleTypeDef DAC_HANDLE;
  int16_t value;
  int8_t assign = params.getAssignedCV(2);
  value = assign == NO_ASSIGN ? 0 : params.getValue(assign);
  HAL_DAC_SetValue(&DAC_HANDLE, DAC_CHANNEL_1, DAC_ALIGN_12B_R, scaleForDac(value));
  assign = params.getAssignedCV(3);
  value = assign == NO_ASSIGN ? 0 : params.getValue(assign);
  HAL_DAC_SetValue(&DAC_HANDLE, DAC_CHANNEL_2, DAC_ALIGN_12B_R, scaleForDac(value));
#endif
}

void updateEncoders(){
  int16_t encoder_values[2] = {(int16_t)__HAL_TIM_GET_COUNTER(&ENCODER_TIM1),
                               (int16_t)__HAL_TIM_GET_COUNTER(&ENCODER_TIM2)};
#ifdef USE_SCREEN
  params.updateEncoders(encoder_values, 2);
#endif
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

// #define DEBUG_USBD_AUDIO
#ifdef DEBUG_USBD_AUDIO
#include "usbd_audio.h"
#include "CircularBuffer.h"
void defaultDrawCallback(uint8_t* pixels, uint16_t width, uint16_t height){
  extern CircularBuffer<audio_t> rx_buffer;
  extern CircularBuffer<audio_t> tx_buffer;
  extern USBD_AUDIO_HandleTypeDef usbd_audio_handle;

  size_t samples = 0; // codec.getSampleCounter();
  float rx = (rx_buffer.getWriteCapacity()+samples)*1.0f/rx_buffer.getSize();
  float tx = (tx_buffer.getReadCapacity()+samples)*1.0f/tx_buffer.getSize();
  float fb = usbd_audio_handle.fb_data.val*1.0f/(1<<14);

  ScreenBuffer& screen = graphics.screen;
  // params.drawTitle(screen);
  // params.drawMessage(26, screen);

  screen.setTextSize(1);
  if(getErrorMessage() != NULL)
    screen.print(2, 16, getErrorMessage());
  if(getDebugMessage() != NULL)
    screen.print(2, 26, getDebugMessage());


  screen.setTextSize(1);
  screen.print(2, 36, "rx/tx ");
  screen.print(rx);
  screen.print(" / ");
  screen.print(tx);
  screen.print(2, 46, "fb ");
  screen.print(fb);
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
