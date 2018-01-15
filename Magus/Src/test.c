#include <stdint.h>
#include "device.h"
#include "HAL_MAX11300.h"
#include "HAL_TLC5946.h"
#include "HAL_OLED.h"

void setup(void);
void loop(void);

void setup(void){
  extern TIM_HandleTypeDef htim3;
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  HAL_TIMEx_PWMN_Start(&htim3,TIM_CHANNEL_4);

  extern SPI_HandleTypeDef hspi5;
  TLC5946_init(&hspi5);
  MAX11300_init(&hspi5);
  OLED_init(&hspi5);

  // Pixi
  MAX11300_setDeviceControl(DCR_DACCTL_ImmUpdate|DCR_DACREF_Int|DCR_ADCCTL_ContSweep/*|DCR_BRST_Contextual*/);

  MAX11300_setPortMode(PORT_1,  PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
  MAX11300_setPortMode(PORT_3,  PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
  MAX11300_setPortMode(PORT_5,  PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
  MAX11300_setPortMode(PORT_7,  PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
  MAX11300_setPortMode(PORT_9,  PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
  MAX11300_setPortMode(PORT_11, PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
  MAX11300_setPortMode(PORT_13, PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
  MAX11300_setPortMode(PORT_15, PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);

  MAX11300_setPortMode(PORT_0,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_1,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
  MAX11300_setPortMode(PORT_2,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_3,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
  MAX11300_setPortMode(PORT_4,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_5,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
  MAX11300_setPortMode(PORT_6,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_7,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
  MAX11300_setPortMode(PORT_8,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_9,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
  MAX11300_setPortMode(PORT_10, PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_11, PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
  MAX11300_setPortMode(PORT_12, PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_13, PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
  MAX11300_setPortMode(PORT_14, PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_15, PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
  MAX11300_setPortMode(PORT_16, PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
	
  // LEDs
  Magus_setRGB_DC(63,63,63);
  for(int x=1; x<17; x++)
    Magus_setRGB(x, 400, 400, 400);

}

void loop(void){

}
