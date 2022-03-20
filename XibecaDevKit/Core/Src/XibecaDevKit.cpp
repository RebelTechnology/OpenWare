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

#if 0
#include <cstdio>
#include "test-qspi.h"

extern "C" void setup();
extern "C" int flash_read_block(int mode, uint32_t address, void* data, uint32_t size);

void setup(){
  const char flashdata[] = "We will put this away.";
  printf("QSPI flash test: %s.\n", flashdata);
  extern QSPI_HandleTypeDef QSPI_FLASH_HANDLE;
  flash_init(&QSPI_FLASH_HANDLE);
  size_t size = sizeof(flashdata);
  uint32_t memloc = 0;
  if(flash_erase(memloc, size) != 0)
    printf("QSPI erase failed");
  flash_wait();
  if(flash_write(memloc, (const uint8_t*)flashdata, size) != 0)
    printf("QSPI write failed");
  flash_wait();
  char readback[size] = {};
  if(flash_read_block(-122, memloc, readback, size) != 0)
    printf("QSPI read failed");
  printf("DATA %s.\n", readback);
  int fs1 = flash_status();
  // flash_reset();
  flash_init(&QSPI_FLASH_HANDLE); // re-init
  int fs2 = flash_status();
  flash_memory_map(-122);
  // flash_memory_map(-444);
  printf("FLASH %s %d %d.\n", (const char*)QSPI_FLASH_BASE, fs1, fs2);

  flash_init(&QSPI_FLASH_HANDLE);
  flash_wait();
  if(flash_erase(memloc, size) != 0)
    printf("QSPI erase failed");

#ifdef USE_USB_DEVICE
  MX_USB_DEVICE_Init();
#endif
#ifdef USE_USB_HOST
  MX_USB_HOST_Init();
#endif

  owl.setup();
  onSetup();
}
#endif

#if 0
extern "C" void qspi_abort();
extern "C" void qspi_exit_mapped_mode();
extern "C" void qspi_enter_mapped_mode();
extern "C" int flash_read_block(int mode, uint32_t address, uint8_t* data, size_t size);
#define QSPI_FLASH_BASE              0x90000000

void onSetup(){

  setLed(1, NO_COLOUR);
  setLed(2, NO_COLOUR);

  uint8_t data[32];
  qspi_enter_mapped_mode();
  memcpy(data, (void*)(QSPI_FLASH_BASE+0), sizeof(data));
  printf("qspi %d %d\n", -1, data[0]);
  qspi_exit_mapped_mode();
  int ret = flash_read_block(-122, 0, data, sizeof(data));
  printf("qspi %d %d\n", ret, data[0]);
}
#else
void onSetup(){
  setLed(1, NO_COLOUR);
  setLed(2, NO_COLOUR);
  // uint8_t data[32];
  // int ret = flash_read_block(-122, 0, data, sizeof(data));
  // printf("qspi %d %d\n", ret, data[0]);
}
#endif

void onLoop(void){
  //
// #ifdef USE_USB_HOST
  // done in callbacks.cpp loop()
//   MX_USB_HOST_Process();
// #endif
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
