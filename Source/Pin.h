#ifndef __PIN_H
#define __PIN_H

#include <stdint.h>
#include <stdbool.h>
#include "device.h"

#define POSITION_VAL(VAL)     (__CLZ(__RBIT(VAL)))

enum PinPull {
   PIN_PULL_NONE       = GPIO_NOPULL,
   PIN_PULL_UP         = GPIO_PULLUP,
   PIN_PULL_DOWN       = GPIO_PULLDOWN
};

enum PinSpeed {
   PIN_SPEED_LOW       = GPIO_SPEED_FREQ_LOW,
   PIN_SPEED_MEDIUM    = GPIO_SPEED_FREQ_MEDIUM,
   PIN_SPEED_HIGH      = GPIO_SPEED_FREQ_HIGH,
   PIN_SPEED_VERY_HIGH = GPIO_SPEED_FREQ_VERY_HIGH	       
};

typedef uint16_t pin_t;

class Pin {
private:
  GPIO_TypeDef* port;
  pin_t pin;
public:
  Pin(GPIO_TypeDef* port, uint16_t pin) : port(port), pin(POSITION_VAL(pin)) {}
  bool get(){
    return port->IDR & (1<<pin);
  }
  void high(){
    port->BSRR = (1<<pin);
  }
  void low(){
    port->BSRR = (1<<(pin+16U));
  }
  void set(bool on){
    port->BSRR = (1<<(pin+16U*!on));
    // if(on)
    //   high(pin);
    // else
    //   low(pin);
  }
  void toggle(){
    port->ODR ^= 1<<pin;
  }
#ifdef STM32H7xx
  void analogMode(){
    // Analog mode is used by ADC and DAC
    uint32_t tmp = port->MODER;
    tmp |= GPIO_MODER_MODE0 << (pin*2U);
    port->MODER = tmp;
  }
  void inputMode(){
    uint32_t tmp = port->MODER;
    tmp &= ~(GPIO_MODER_MODE0 << (pin*2U));
    port->MODER = tmp;
  }
  void outputMode(){
    uint32_t tmp = port->MODER;
    tmp |= GPIO_MODER_MODE0_0 << (pin*2U);
    tmp &= ~(GPIO_MODER_MODE0_1 << (pin*2U));
    port->MODER = tmp;
  }
  void afMode(){
    uint32_t tmp = port->MODER;
    tmp &= ~(GPIO_MODER_MODE0_0 << (pin*2U));
    tmp |= GPIO_MODER_MODE0_1 << (pin*2U);
    port->MODER = tmp;
  }
  void setAlternateFunction(uint32_t af){
    /* Configure Alternate function mapped with the current IO */
    uint32_t tmp = port->AFR[pin >> 3U];
    tmp &= ~(0xFU << ((pin & 0x07U) * 4U));
    tmp |= (af << ((pin & 0x07U) * 4U));
    port->AFR[pin >> 3U] = tmp;
  }
  void pushPullMode(){
    port->OTYPER |= 1<<pin;
  }
  void openDrainMode(){
    port->OTYPER |= 1<<pin;
  }
  void setPull(uint32_t pull){
    uint32_t tmp = port->PUPDR;
    tmp &= ~(GPIO_PUPDR_PUPD0 << (pin*2U));
    tmp |= (pull << (pin*2U));
    port->PUPDR = tmp;
  }
  uint32_t getPull(){
    return (port->PUPDR & (GPIO_PUPDR_PUPD0 << (pin*2U))) >> (pin*2U);
  }
  void setSpeed(PinSpeed speed){
    uint32_t tmp = port->OSPEEDR; 
    tmp &= ~(GPIO_OSPEEDR_OSPEED0 << (pin * 2U));
    tmp |= (speed << (pin * 2U));
    port->OSPEEDR = tmp;
  }
#else
    void analogMode(){
    // Analog mode is used by ADC and DAC
    uint32_t tmp = port->MODER;
    tmp |= GPIO_MODER_MODER0 << (pin*2U);
    port->MODER = tmp;
  }
  void inputMode(){
    uint32_t tmp = port->MODER;
    tmp &= ~(GPIO_MODER_MODER0 << (pin*2U));
    port->MODER = tmp;
  }
  void outputMode(){
    uint32_t tmp = port->MODER;
    tmp |= GPIO_MODER_MODER0_0 << (pin*2U);
    tmp &= ~(GPIO_MODER_MODER0_1 << (pin*2U));
    port->MODER = tmp;
  }
  void afMode(){
    uint32_t tmp = port->MODER;
    tmp &= ~(GPIO_MODER_MODER0_0 << (pin*2U));
    tmp |= GPIO_MODER_MODER0_1 << (pin*2U);
    port->MODER = tmp;
  }
  void setAlternateFunction(uint32_t af){
    /* Configure Alternate function mapped with the current IO */
    uint32_t tmp = port->AFR[pin >> 3U];
    tmp &= ~(0xFU << ((pin & 0x07U) * 4U));
    tmp |= (af << ((pin & 0x07U) * 4U));
    port->AFR[pin >> 3U] = tmp;
  }
  void pushPullMode(){
    port->OTYPER |= 1<<pin;
  }
  void openDrainMode(){
    port->OTYPER |= 1<<pin;
  }
  void setPull(uint32_t pull){
    uint32_t tmp = port->PUPDR;
    tmp &= ~(GPIO_PUPDR_PUPDR0 << (pin*2U));
    tmp |= (pull << (pin*2U));
    port->PUPDR = tmp;
  }
  uint32_t getPull(){
    return (port->PUPDR & (GPIO_PUPDR_PUPDR0 << (pin*2U))) >> (pin*2U);
  }
  void setSpeed(PinSpeed speed){
    uint32_t tmp = port->OSPEEDR; 
    tmp &= ~(GPIO_OSPEEDER_OSPEEDR0 << (pin * 2U));
    tmp |= (speed << (pin * 2U));
    port->OSPEEDR = tmp;
  }
#endif
};

// typedef uint8_t pin_t;
// template<GPIO_TypeDef* port, unsigned int pin>
// class Pin {
//   static bool get(pin_t pin){
//     return port->IDR & (1<<pin);
//   }
//   static void high(pin_t pin){
//     port->BSRR = (1<<pin);
//   }
//   static void low(pin_t pin){
//     port->BSRR = (1<<(pin+16U));
//   }
//   static void set(pin_t pin, bool on){
//     port->BSRR = (1<<(pin+16U*!on));
//     // if(on)
//     //   high(pin);
//     // else
//     //   low(pin);
//   }
//   static void toggle(pin_t pin){
//     port->ODR ^= 1<<pin;
//   }
// };

// template<GPIO_TypeDef* port, unsigned int pin>
// class Pin {
//   static bool get(){
//     return port->IDR & (1<<pin);
//   }
//   static void high(){
//     port->BSRR = (1<<pin);
//   }
//   static void low(){
//     port->BSRR = (1<<(pin+16U));
//   }
//   static void set(bool on){
//     port->BSRR = (1<<(pin+16U*!on));
//     // if(on)
//     //   high(pin);
//     // else
//     //   low(pin);
//   }
//   static void toggle(){
//     port->ODR ^= 1<<pin;
//   }
//   static void analogMode(){
//     // Analog mode is used by ADC and DAC
//     uint32_t tmp = port->MODER;
//     tmp |= GPIO_MODER_MODER0 << (pin*2U);
//     port->MODER = tmp;
//   }
//   static void inputMode(){
//     uint32_t tmp = port->MODER;
//     tmp &= ~(GPIO_MODER_MODER0 << (pin*2U));
//     port->MODER = tmp;
//   }
//   static void outputMode(){
//     uint32_t tmp = port->MODER;
//     tmp |= GPIO_MODER_MODER0_0 << (pin*2U);
//     tmp &= ~(GPIO_MODER_MODER0_1 << (pin*2U));
//     port->MODER = tmp;
//   }
//   static void afMode(){
//     uint32_t tmp = port->MODER;
//     tmp &= ~(GPIO_MODER_MODER0_0 << (pin*2U));
//     tmp |= GPIO_MODER_MODER0_1 << (pin*2U);
//     port->MODER = tmp;
//   }
//   static void setAlternateFunction(uint32_t af){
//     /* Configure Alternate function mapped with the current IO */
//     uint32_t tmp = port->AFR[pin >> 3U];
//     tmp &= ~(0xFU << ((pin & 0x07U) * 4U));
//     tmp |= (af << ((pin & 0x07U) * 4U));
//     port->AFR[pin >> 3U] = tmp;
//   }
//   static void pushPullMode(){
//     port->OTYPER |= 1<<pin;
//   }
//   static void openDrainMode(){
//     port->OTYPER |= 1<<pin;
//   }
//   static void setPull(uint32_t pull){
//     uint32_t tmp = port->PUPDR;
//     tmp &= ~(GPIO_PUPDR_PUPDR0 << (pin*2U));
//     tmp |= (pull << (pin*2U));
//     port->PUPDR = tmp;
//   }
//   static uint32_t getPull(){
//     return (port->PUPDR & (GPIO_PUPDR_PUPDR0 << (pin*2U))) >> (pin*2U);
//   }
//   static void setSpeed(PinSpeed speed){
//     uint32_t tmp = port->OSPEEDR; 
//     tmp &= ~(GPIO_OSPEEDER_OSPEEDR0 << (pin * 2U));
//     tmp |= (speed << (pin * 2U));
//     port->OSPEEDR = tmp;
//   }
// };

#endif /* __PIN_H */
