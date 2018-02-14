#include "serial.h"

#if 0 // see bus.cpp
#include "stm32f7xx_hal.h"
#include "errorhandlers.h"
#include "mxconstants.h"

extern UART_HandleTypeDef huart1;

void serial_read(uint8_t* data, uint16_t size){
  /* while(huart1.Lock == HAL_LOCKED); */
  /* if(huart->RxState == HAL_UART_STATE_READY) */
  if(HAL_UART_Receive_IT(&huart1, data, size) != HAL_OK)
    ASSERT(0, "UART rx failed");
}

void serial_write(uint8_t* data, uint16_t size){
  static const int timeout = 100;
  /* while(huart1.Lock == HAL_LOCKED); */
  /* if(huart->gState == HAL_UART_STATE_READY) */
  /* if(HAL_UART_Transmit_IT(&huart1, data, size) != HAL_OK) */
  if(HAL_UART_Transmit(&huart1, data, size, timeout) != HAL_OK)
    ASSERT(0, "UART tx failed");
}
#endif
