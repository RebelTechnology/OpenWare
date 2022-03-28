#include "Owl.h"
#include "device.h"
#include "errorhandlers.h"
#include "Pin.h"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"
#include "message.h"
#include "Codec.h"
#include "Storage.h"
#include "ServiceCall.h"
#ifdef USE_USB_DEVICE
#include "usb_device.h"
#endif
#ifdef USE_USB_HOST
#include "usb_host.h"
#endif

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

Pin pin_gpio1(XIBECA_PIN11);
Pin pin_gpio2(XIBECA_PIN12);
Pin pin_gpio3(XIBECA_PIN9);

// GPIO3         EXTI0 (PIN9)
// GPIO2        TIM1CH2 (PIN12)
// GPIO1         TIM1CH1 (PIN11)
// DAC1           (PIN69)
// DAC2        (PIN70)
// ADC1        (PIN31)
// ADC2            (PIN32)

// UART_RX     (NOT CONNECTED)
// UART_TX    (NOT CONNECTED)
// SPI_CLK        SPI3_SCK (PIN80)
// SPI_MOSI       SPI3_MOSI(PIN76)
// SPI_MISO       SPI3_MISO (PIN78)
// SCL_TX        I2C_SCL (PIN73)
// DAA_RX         I2C_SDA (PIN74) 

// Pin pin_b1(XIBECA_PIN3);
// Pin pin_b2(XIBECA_PIN3);
Pin pin_led_b1(XIBECA_PIN17);
Pin pin_led_b2(XIBECA_PIN18);

// Pin led_in1(XIBECA_PIN13);
// Pin led_in2(XIBECA_PIN14);
// Pin led_in3(XIBECA_PIN19);
// Pin led_in4(XIBECA_PIN20);

// Pin led_clip1(XIBECA_PIN10);
// Pin led_clip2(XIBECA_PIN7);
// Pin led_clip3(XIBECA_PIN8);
// Pin led_clip4(XIBECA_PIN5);

// Pin led_out1(XIBECA_PIN21);
// Pin led_out2(XIBECA_PIN22);
// Pin led_out3(XIBECA_PIN23);
// Pin led_out4(XIBECA_PIN24);

void setLed(uint8_t led, uint32_t rgb){
  switch(led){
  case 1:
    if(rgb == NO_COLOUR)
      pin_led_b1.high();
    else
      pin_led_b1.low();
    break;
  case 2:
    if(rgb == NO_COLOUR)
      pin_led_b2.high();
    else
      pin_led_b2.low();
    break;
  }
}

void onChangePin(uint16_t pin){
  switch(pin){
  case BUT_A_Pin: {
    bool state = HAL_GPIO_ReadPin(BUT_A_GPIO_Port, BUT_A_Pin) == GPIO_PIN_RESET;
    setButtonValue(PUSHBUTTON, state);
    setButtonValue(BUTTON_1, state);
    setLed(1, state);
    break;
  }
  case BUT_B_Pin: {
    bool state = HAL_GPIO_ReadPin(BUT_B_GPIO_Port, BUT_B_Pin) == GPIO_PIN_RESET;
    setButtonValue(BUTTON_2, state);
    setLed(2, state);
    break;
  }
  }
}

void setGateValue(uint8_t ch, int16_t value){
  switch(ch){
  case BUTTON_1:
    setLed(1, value);
    break;
  case BUTTON_2:
    setLed(2, value);
    break;
  case BUTTON_3:
    pin_gpio1.set(value == 0);
    break;
  case BUTTON_4:
    pin_gpio2.set(value == 0);
    break;
  case BUTTON_5:
    pin_gpio3.set(value == 0);
    break;
  }
}

static uint16_t smooth_adc_values[NOF_ADC_VALUES];
extern "C"{
  void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
    extern uint16_t adc_values[NOF_ADC_VALUES];
    for(size_t i=0; i<NOF_ADC_VALUES; ++i){
      // IIR exponential filter with lambda 0.75: y[n] = 0.75*y[n-1] + 0.25*x[n]
      smooth_adc_values[i] = ((uint32_t)smooth_adc_values[i]*3 + adc_values[i]) >> 2;
    }
    // ADC 25 Mhz, 64.5 cycles
    // 12-bit Prescaler=128, os=1 : 86 Hz
    // 16-bit Prescaler=8,   os=4 : 334 Hz
    // pin_led_b1.toggle();
  }
}

void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len){
  // parameter_values[0] = (parameter_values[0]*3 + smooth_adc_values[ADC_A])>>2;
  // parameter_values[1] = (parameter_values[1]*3 + smooth_adc_values[ADC_B])>>2;
  // parameter_values[2] = (parameter_values[2]*3 + smooth_adc_values[ADC_C])>>2;
  // parameter_values[3] = (parameter_values[3]*3 + smooth_adc_values[ADC_D])>>2;
  // parameter_values[4] = (parameter_values[4]*3 + smooth_adc_values[ADC_E])>>2;
  // parameter_values[5] = (parameter_values[5]*3 + smooth_adc_values[ADC_F])>>2;
  // parameter_values[6] = (parameter_values[6]*3 + smooth_adc_values[ADC_G])>>2;
  // parameter_values[7] = (parameter_values[7]*3 + smooth_adc_values[ADC_H])>>2;
  for(size_t i=0; i<NOF_ADC_VALUES; ++i)
    parameter_values[i] = smooth_adc_values[i] >> 4;
}

void onSetup(){
  pin_gpio1.outputMode();
  pin_gpio2.outputMode();
  pin_gpio3.outputMode();
  pin_gpio1.high();
  pin_gpio2.high();
  pin_gpio3.high();
  setLed(1, NO_COLOUR);
  setLed(2, NO_COLOUR);
#ifdef DEBUG
  printf("Device ID 0x%lx Revision 0x%lx\n", HAL_GetDEVID(), HAL_GetREVID());
#endif
}

void onLoop(void){
  // do smth
}

#ifdef USE_FAST_POW_RESOURCES
uint32_t fast_log_table_size = 0;
uint32_t fast_pow_table_size = 0;
float fast_log_table[16384] __attribute__ ((section (".d3data")));
uint32_t fast_pow_table[2048] __attribute__ ((section (".d2data")));
void onResourceUpdate(){
  Resource* res = storage.getResourceByName(SYSTEM_TABLE_LOG ".bin");
  if(res && res->isValid()){
    fast_log_table_size = std::min(res->getDataSize()/sizeof(float), 16384U);
    storage.readResource(res->getHeader(), fast_log_table, 0, fast_log_table_size*sizeof(float));
  }else{
    fast_log_table_size = 0;
  }
  res = storage.getResourceByName(SYSTEM_TABLE_POW ".bin");
  if(res && res->isValid()){
    fast_pow_table_size = std::min(res->getDataSize()/sizeof(uint32_t), 2048U);
    storage.readResource(res->getHeader(), fast_pow_table, 0, fast_pow_table_size*sizeof(uint32_t));
  }else{
    fast_pow_table_size = 0;
  }
  debugMessage("log/pow", fast_log_table_size, fast_pow_table_size);
}
#else
#include "FastLogTable.h"
#include "FastPowTable.h"
#endif
