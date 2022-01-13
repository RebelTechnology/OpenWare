#include "Owl.h"
#include "device.h"
#include "errorhandlers.h"
#include "Pin.h"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"
#include "message.h"
#include "Codec.h"

#define XIBECA_PIN3  GPIOD, GPIO_PIN_2
#define XIBECA_PIN4  GPIOG, GPIO_PIN_10
#define XIBECA_PIN5  GPIOC, GPIO_PIN_9
#define XIBECA_PIN6  GPIOC, GPIO_PIN_8
#define XIBECA_PIN7  GPIOC, GPIO_PIN_13
#define XIBECA_PIN8  GPIOI, GPIO_PIN_11
#define XIBECA_PIN9  GPIOC, GPIO_PIN_14
#define XIBECA_PIN10 GPIOA, GPIO_PIN_0
#define XIBECA_PIN11 GPIOH, GPIO_PIN_4
#define XIBECA_PIN12 GPIOC, GPIO_PIN_15
#define XIBECA_PIN13 GPIOA, GPIO_PIN_7
#define XIBECA_PIN14 GPIOA, GPIO_PIN_3

#define XIBECA_PIN17 GPIOG, GPIO_PIN_12
#define XIBECA_PIN18 GPIOG, GPIO_PIN_11
#define XIBECA_PIN19 GPIOA, GPIO_PIN_15
#define XIBECA_PIN20 GPIOA, GPIO_PIN_1

#define XIBECA_PIN21 GPIOC, GPIO_PIN_6
#define XIBECA_PIN22 GPIOC, GPIO_PIN_7
#define XIBECA_PIN23 GPIOD, GPIO_PIN_12
#define XIBECA_PIN24 GPIOD, GPIO_PIN_13

// Pin led_in1(XIBECA_PIN13);
// Pin led_in2(XIBECA_PIN14);
// Pin led_in3(XIBECA_PIN19);
// Pin led_in4(XIBECA_PIN20);

Pin led_clip1(XIBECA_PIN10);
Pin led_clip2(XIBECA_PIN7);
Pin led_clip3(XIBECA_PIN8);
Pin led_clip4(XIBECA_PIN5);

// Pin led_out1(XIBECA_PIN21);
// Pin led_out2(XIBECA_PIN22);
// Pin led_out3(XIBECA_PIN23);
// Pin led_out4(XIBECA_PIN24);

void setLed(uint8_t led, uint32_t rgb){
  uint32_t pwm = 1023 - (__USAT(rgb>>2, 10)); // expects 12-bit parameter value
  switch(led){
  case 1:
    if(rgb == RED_COLOUR){
      led_clip1.low();
      TIM2->CCR1 = 0xFFFFFFFFU;
    }else{
      led_clip1.high();
      TIM2->CCR1 = pwm;
    }
    break;
  case 2:
    if(rgb == RED_COLOUR){
      led_clip2.low();
      TIM2->CCR4 = 0xFFFFFFFFU;
    }else{
      led_clip2.high();
      TIM2->CCR4 = pwm;
    }      
    break;
  case 3:
    if(rgb == RED_COLOUR){
      led_clip3.low();
      TIM3->CCR2 = 0xFFFFFFFFU;
    }else{
      led_clip3.high();
      TIM3->CCR2 = pwm;
    }
    break;
  case 4:
    if(rgb == RED_COLOUR){
      led_clip4.low();
      TIM2->CCR2 = 0xFFFFFFFFU;
    }else{
      led_clip4.high();
      TIM2->CCR2 = pwm;
    }
    break;
  case 5:
    TIM8->CCR1 = pwm;
    break;
  case 6:
    TIM8->CCR2 = pwm;
    break;
  case 7:
    TIM4->CCR1 = pwm;
    break;
  case 8:
    TIM4->CCR2 = pwm;
    break;
  }
}

void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len){
  parameter_values[0] = (parameter_values[0]*3 + adc_values[ADC_A])>>2;
  parameter_values[1] = (parameter_values[1]*3 + adc_values[ADC_B])>>2;
  parameter_values[2] = (parameter_values[2]*3 + adc_values[ADC_C])>>2;
  parameter_values[3] = (parameter_values[3]*3 + adc_values[ADC_D])>>2;
}

void initLed(){
  extern TIM_HandleTypeDef htim2;
  extern TIM_HandleTypeDef htim3;
  extern TIM_HandleTypeDef htim4;
  extern TIM_HandleTypeDef htim8;
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_Base_Start(&htim4);
  HAL_TIM_Base_Start(&htim8);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); // in1
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2); // in4
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4); // in2
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); // in3
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1); // out3
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2); // out4
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1); // out1
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2); // out2

  led_clip1.outputMode();
  led_clip2.outputMode();
  led_clip3.outputMode();
  led_clip4.outputMode();
}

void onSetup(){
  initLed();
  for(size_t i=1; i<=8; ++i)
    setLed(i, NO_COLOUR);
  // codec.set((1<<22)-1);
}

void onLoop(void){  
  for(size_t i=0; i<4; ++i){
    int16_t value = getParameterValue(PARAMETER_AA+i);
    setLed(i+1, value >= 4000 ? RED_COLOUR : value);
    setLed(i+5, getParameterValue(PARAMETER_BA+i));
  }
}
