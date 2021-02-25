#include "gpio.h"
#include "stm32f1xx_hal_gpio.h"

bool getPin(GPIO_TypeDef* port, uint32_t pin){
  return port->IDR & pin;
}

void setPin(GPIO_TypeDef* port, uint32_t pin){
  port->BSRR = pin;
}

void clearPin(GPIO_TypeDef* port, uint32_t pin){
  port->BRR = pin;
}

void togglePin(GPIO_TypeDef* port, uint32_t pin){
  port->ODR ^= pin;
/*      GPIO_ToggleBits(port, pin); */
}

/* void configureDigitalInput(GPIO_TypeDef* port, uint32_t pin, bool pullup){ */
/*   GPIO_InitTypeDef GPIO_InitStructure; */
/*   GPIO_StructInit(&GPIO_InitStructure); */
/*   GPIO_InitStructure.GPIO_Pin = pin; */
/*   if(pullup) */
/*     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; */
/*   else */
/*     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; */
/*   GPIO_Init(port, &GPIO_InitStructure); */
/* } */

/* void configureDigitalOutput(GPIO_TypeDef* port, uint32_t pin){ */
/*   GPIO_InitTypeDef GPIO_InitStructure; */
/*   GPIO_StructInit(&GPIO_InitStructure); */
/*   GPIO_InitStructure.GPIO_Pin = pin; */
/*   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; */
/*   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; */
/*   GPIO_Init(port, &GPIO_InitStructure); */
/* } */
