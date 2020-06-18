#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "Owl.h"
#include "errorhandlers.h"
#include "message.h"

const uint8_t seg_bits[10] =
  {
   0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67
  };
GPIO_TypeDef* seg_ports[8] =
  {
   DISPLAY_A_GPIO_Port,
   DISPLAY_B_GPIO_Port,
   DISPLAY_C_GPIO_Port,
   DISPLAY_D_GPIO_Port,
   DISPLAY_E_GPIO_Port,
   DISPLAY_F_GPIO_Port,
   DISPLAY_G_GPIO_Port,
   DISPLAY_DP_GPIO_Port
  };
const uint16_t seg_pins[8] =
  {
   DISPLAY_A_Pin,
   DISPLAY_B_Pin,
   DISPLAY_C_Pin,
   DISPLAY_D_Pin,
   DISPLAY_E_Pin,
   DISPLAY_F_Pin,
   DISPLAY_G_Pin,
   DISPLAY_DP_Pin
  };

void setSegmentDisplay(int value){
  uint8_t bits = seg_bits[value%10];
  for(int i=0; i<8; ++i)
    HAL_GPIO_WritePin(seg_ports[i], seg_pins[i], (bits & (1<<i)) ? GPIO_PIN_RESET :  GPIO_PIN_SET);
}

void setup(){
  extern TIM_HandleTypeDef htim2;
  // __HAL_TIM_SET_COUNTER(&htim2, INT16_MAX/2);
  HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL);
  setSegmentDisplay(0);

  owl_setup();
}

void loop(void){
  extern TIM_HandleTypeDef htim2;
  int value = __HAL_TIM_GET_COUNTER(&htim2);
  setSegmentDisplay(value>>2);
 
  MX_USB_HOST_Process(); // todo: enable PWR management

  owl_loop();
}
