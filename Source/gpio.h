#ifndef __GPIO_H
#define __GPIO_H

#include <stdint.h>
#include <stdbool.h>
#include "device.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define setPin(port, pin) HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)
#define clearPin(port, pin) HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)
#define getPin(port, pin) HAL_GPIO_ReadPin(port, pin)

#if 0
/* Every GPIO port has an input and */
/* output data register, ODR and IDR */
/* respectively, which hold the status of the pin */
   inline bool getPin(GPIO_TypeDef* port, uint32_t pin){
     return port->IDR & pin;
/*      return GPIO_ReadInputDataBit(port, pin); */
   }
   inline void setPin(GPIO_TypeDef* port, uint32_t pin){
     port->BSRR = pin;
   }
   inline void clearPin(GPIO_TypeDef* port, uint32_t pin){
     port->BSRR = pin << 16;
   }
   inline void togglePin(GPIO_TypeDef* port, uint32_t pin){
     port->ODR ^= pin;
   }

   /* void configureDigitalInput(GPIO_TypeDef* port, uint32_t pin, GPIOPuPd_TypeDef pull); */
   /* void configureDigitalOutput(GPIO_TypeDef* port, uint32_t pin); */

   /* void configureAnalogInput(GPIO_TypeDef* port, uint32_t pin); */
   /* void configureAnalogOutput(GPIO_TypeDef* port, uint32_t pin); */
#endif

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_H */
