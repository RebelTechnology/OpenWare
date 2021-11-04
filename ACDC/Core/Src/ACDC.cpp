#include "Owl.h"
#include "device.h"
#include "errorhandlers.h"
#include "Pin.h"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"
#include "message.h"

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

#define XIBECA_PIN21 GPIOC, GPIO_PIN_6
#define XIBECA_PIN22 GPIOC, GPIO_PIN_7
#define XIBECA_PIN23 GPIOD, GPIO_PIN_12
#define XIBECA_PIN24 GPIOD, GPIO_PIN_13

Pin led_in1(XIBECA_PIN17);
Pin led_in2(XIBECA_PIN18);
Pin led_in3(XIBECA_PIN19);
Pin led_in4(XIBECA_PIN20);

Pin led_clip1(XIBECA_PIN10);
Pin led_clip2(XIBECA_PIN7);
Pin led_clip3(XIBECA_PIN8);
Pin led_clip4(XIBECA_PIN5);

Pin led_out1(XIBECA_PIN21);
Pin led_out2(XIBECA_PIN22);
Pin led_out3(XIBECA_PIN23);
Pin led_out4(XIBECA_PIN24);

void setLed(uint8_t led, uint32_t rgb){
  bool value = rgb;
  switch(led){
  case 1:
    led_in1.set(value);
    break;
  case 2:
    led_in2.set(value);
    break;
  case 3:
    led_in3.set(value);
    break;
  case 4:
    led_in4.set(value);
    break;
  case 5:
    led_clip1.set(value);
    break;
  case 6:
    led_clip2.set(value);
    break;
  case 7:
    led_clip3.set(value);
    break;
  case 8:
    led_clip4.set(value);
    break;
  case 9:
    led_out1.set(value);
    break;
  case 10:
    led_out2.set(value);
    break;
  case 11:
    led_out3.set(value);
    break;
  case 12:
    led_out4.set(value);
    break;
  }
}

void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len){
  parameter_values[0] = (parameter_values[0]*3 + adc_values[ADC_A])>>2;
  parameter_values[1] = (parameter_values[1]*3 + adc_values[ADC_B])>>2;
  parameter_values[2] = (parameter_values[2]*3 + adc_values[ADC_C])>>2;
  parameter_values[3] = (parameter_values[3]*3 + adc_values[ADC_D])>>2;
}

void setup(){
  led_in1.outputMode();
  led_in2.outputMode();
  led_in3.outputMode();
  led_in4.outputMode();

  led_clip1.outputMode();
  led_clip2.outputMode();
  led_clip3.outputMode();
  led_clip4.outputMode();

  led_out1.outputMode();
  led_out2.outputMode();
  led_out3.outputMode();
  led_out4.outputMode();

  for(size_t i=1; i<=12; ++i)
    setLed(i, NO_COLOUR);

  owl.setup();
}

void loop(void){  
  owl.loop();
}
