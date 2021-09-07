#include <stdint.h>
#include "device.h"
#include "HAL_MAX11300.h"
#include "HAL_TLC5946.h"
#include "HAL_OLED.h"
#include "HAL_Encoders.h"
//#include "FreeRTOS.h"

#define pUSBH_PwrEN(state)		HAL_GPIO_WritePin(USB_HOST_PWR_EN_GPIO_Port,  USB_HOST_PWR_EN_Pin,  (GPIO_PinState)state)

uint16_t usiResult;
extern uint16_t rgENC_Values[7];

void setup(void);
void loop(void);

void usbh_midi_reset(){}
   void midi_device_rx(uint8_t *buffer, uint32_t length){}
   void usbh_midi_rx(uint8_t *buffer, uint32_t length){}

   void vApplicationMallocFailedHook(void) {
    // taskDISABLE_INTERRUPTS();
    for(;;);
  }
	 
  void vApplicationIdleHook(void) {
  }
typedef void * TaskHandle_t;
  void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    // taskDISABLE_INTERRUPTS();
    for(;;);
  }

void setup(void){
  extern TIM_HandleTypeDef htim3;
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

  extern SPI_HandleTypeDef hspi5;
  TLC5946_init(&hspi5);
  MAX11300_init(&hspi5);
  OLED_init(&hspi5);
	Encoders_init(&hspi5);

  // Pixi
	MAX11300_setDeviceControl(DCR_RESET);
	HAL_Delay(1000);
  MAX11300_setDeviceControl(DCR_DACCTL_ImmUpdate|DCR_DACREF_Int|DCR_ADCCTL_ContSweep/*|DCR_BRST_Contextual*/);

  MAX11300_setPortMode(1,  	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);
  MAX11300_setPortMode(2,  	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);
  MAX11300_setPortMode(3,  	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);
  MAX11300_setPortMode(4,  	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);
  MAX11300_setPortMode(5,  	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);
  MAX11300_setPortMode(6, 	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);
  MAX11300_setPortMode(7, 	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);
  MAX11300_setPortMode(8, 	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);
	MAX11300_setPortMode(9,  	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);
  MAX11300_setPortMode(10, 	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);
  MAX11300_setPortMode(1,  	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);
  MAX11300_setPortMode(12, 	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);
  MAX11300_setPortMode(13, 	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);
  MAX11300_setPortMode(14, 	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);
  MAX11300_setPortMode(15, 	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);
	MAX11300_setPortMode(16, 	PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_4|DCR_ADCCONV_400ksps|PCR_ADCref_INT);

//  MAX11300_setPortMode(0,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_1,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
//  MAX11300_setPortMode(2,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_3,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
//  MAX11300_setPortMode(4,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_5,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
//  MAX11300_setPortMode(6,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_7,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
//  MAX11300_setPortMode(8,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_9,  PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
//  MAX11300_setPortMode(10, PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_11, PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
 // MAX11300_setPortMode(12, PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_13, PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
 // MAX11300_setPortMode(14, PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
  /* MAX11300_setPortMode(PORT_15, PCR_Range_DAC_M5_P5|PCR_Mode_DAC); */
 // MAX11300_setPortMode(16, PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
	
  // LEDs
  Magus_setRGB_DC(63,63,63);
  
	OLED_ClearScreen();
	
	pUSBH_PwrEN(1);
}

void loop(void)
{
	
	for(int x=16; x>0; x--)
	{
		usiResult = 1000;
		Magus_setRGB(x, usiResult, usiResult, usiResult);
	}
	
	OLED_Refresh();
	TLC5946_Refresh_GS();
	Encoders_readAll();
	HAL_Delay(10);
}
