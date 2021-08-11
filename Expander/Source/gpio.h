#ifndef __GPIO_H
#define __GPIO_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
 extern "C" {
#endif

   typedef enum { PULL_NONE, PULL_UP, PULL_DOWN } GpioPull;

   bool getPin(GPIO_TypeDef* port, uint32_t pin);
   void setPin(GPIO_TypeDef* port, uint32_t pin);
   void clearPin(GPIO_TypeDef* port, uint32_t pin);
   void togglePin(GPIO_TypeDef* port, uint32_t pin);

   void configureDigitalInput(GPIO_TypeDef* port, uint32_t pin, bool pullup);
   void configureDigitalOutput(GPIO_TypeDef* port, uint32_t pin);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_H */
