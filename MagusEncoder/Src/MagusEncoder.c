#include "stm32f0xx_hal.h"
#include "MagusEncoder.h"
#include "main.h"

#define ENCODER_COUNT 7
#define ENCODER_ERROR_DETECTION
#include "quadrature.h"

extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

uint8_t getEncoderState3(){
  return HAL_GPIO_ReadPin(ENC3_A_GPIO_Port, ENC3_A_Pin) << 1 |
    HAL_GPIO_ReadPin(ENC3_B_GPIO_Port, ENC3_B_Pin);
}  

uint8_t getEncoderState5(){
  return HAL_GPIO_ReadPin(ENC5_A_GPIO_Port, ENC5_A_Pin) << 1 |
    HAL_GPIO_ReadPin(ENC5_B_GPIO_Port, ENC5_B_Pin);
}  

uint8_t getEncoderState6(){
  return HAL_GPIO_ReadPin(ENC6_A_GPIO_Port, ENC6_A_Pin) << 1 |
    HAL_GPIO_ReadPin(ENC6_B_GPIO_Port, ENC6_B_Pin);
}

void Encoders_Init (void) {
  // Reset encoder values
  encoder_init(0, 0, 0);
  encoder_init(1, 0, 0x3FFF);
  encoder_init(2, 0, 0x3FFF);
  encoder_init(4, 0, 0x3FFF);
  
  // Configure interrupt encoders 3, 5 and 6
  encoder_init(3, getEncoderState3(), 0x3FFF);
  encoder_init(5, getEncoderState5(), 0x3FFF);
  encoder_init(6, getEncoderState6(), 0x3FFF);
	
  // Configure and start TIM encoders 1, 2 and 4
  __HAL_TIM_SET_COUNTER(&htim1, encoder_data[1]);
  __HAL_TIM_SET_COUNTER(&htim2, encoder_data[2]);
  __HAL_TIM_SET_COUNTER(&htim3, encoder_data[4]);
  HAL_TIM_Encoder_Start_IT(&htim1, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start_IT(&htim3, TIM_CHANNEL_ALL);
}

void HAL_GPIO_EXTI_Callback(uint16_t pin){
  switch(pin) {
  case ENC3_A_Pin: 
  case ENC3_B_Pin:
    encoder_interrupt(3, getEncoderState3());
    break;
  case ENC5_A_Pin: 
  case ENC5_B_Pin:
    encoder_interrupt(5, getEncoderState5());
    break;
  case ENC6_A_Pin:
  case ENC6_B_Pin:
    encoder_interrupt(6, getEncoderState6());
    break;
  case SPI_NCS_Pin:
    send_SPI();
    break;
  }
}
	
void Encoders_Main (void) {
  // Read switches
  uint16_t buttons;
  buttons  = (1-HAL_GPIO_ReadPin(ENC1_SW_GPIO_Port, ENC1_SW_Pin))<<0;
  buttons |= (1-HAL_GPIO_ReadPin(ENC2_SW_GPIO_Port, ENC2_SW_Pin))<<1;
  buttons |= (1-HAL_GPIO_ReadPin(ENC3_SW_GPIO_Port, ENC3_SW_Pin))<<2;
  buttons |= (1-HAL_GPIO_ReadPin(ENC4_SW_GPIO_Port, ENC4_SW_Pin))<<3;
  buttons |= (1-HAL_GPIO_ReadPin(ENC5_SW_GPIO_Port, ENC5_SW_Pin))<<4;
  buttons |= (1-HAL_GPIO_ReadPin(ENC6_SW_GPIO_Port, ENC6_SW_Pin))<<5;
	
  // Update Switch states
  encoder_data[0] = buttons;

  // Update TIM encoders
  encoder_data[1] = __HAL_TIM_GET_COUNTER(&htim1); 
  encoder_data[2] = __HAL_TIM_GET_COUNTER(&htim2);
  encoder_data[4] = __HAL_TIM_GET_COUNTER(&htim3);
}

void send_SPI(void) {	
  // Send data
  HAL_SPI_Transmit_DMA(&hspi1, (uint8_t*)&encoder_data, 14);
}

