#include "stm32f1xx_hal.h"
#include "HAL_MAX11300.h"
#include "HAL_TLC5946.h"

void setup(void);
void run(void);

uint16_t rgADCValues[20];
uint16_t rgDACValues[20];

void setup()
{
  MAX11300_setDeviceControl(DCR_DACCTL_ImmUpdate|DCR_DACREF_Int|DCR_ADCCTL_ContSweep/*|DCR_BRST_Contextual*/);

/*
	MAX11300_setPortMode(PORT_1, PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
	MAX11300_setPortMode(PORT_3, PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
	MAX11300_setPortMode(PORT_7, PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
	MAX11300_setPortMode(PORT_9, PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
	MAX11300_setPortMode(PORT_10, PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
	MAX11300_setPortMode(PORT_11, PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
	MAX11300_setPortMode(PORT_12, PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
	MAX11300_setPortMode(PORT_13, PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
	MAX11300_setPortMode(PORT_14, PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
	MAX11300_setPortMode(PORT_15, PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
*/

  MAX11300_setPortMode(PORT_1, PCR_Range_DAC_0_P10|PCR_Mode_DAC);
  MAX11300_setPortMode(PORT_3, PCR_Range_DAC_0_P10|PCR_Mode_DAC);
  MAX11300_setPortMode(PORT_5, PCR_Range_DAC_0_P10|PCR_Mode_DAC);
  MAX11300_setPortMode(PORT_7, PCR_Range_DAC_0_P10|PCR_Mode_DAC);
  MAX11300_setPortMode(PORT_9, PCR_Range_DAC_0_P10|PCR_Mode_DAC);
  MAX11300_setPortMode(PORT_10, PCR_Range_DAC_0_P10|PCR_Mode_DAC);
  MAX11300_setPortMode(PORT_11, PCR_Range_DAC_0_P10|PCR_Mode_DAC);
  MAX11300_setPortMode(PORT_12, PCR_Range_DAC_0_P10|PCR_Mode_DAC);
  MAX11300_setPortMode(PORT_13, PCR_Range_DAC_0_P10|PCR_Mode_DAC);
  MAX11300_setPortMode(PORT_14, PCR_Range_DAC_0_P10|PCR_Mode_DAC);
  MAX11300_setPortMode(PORT_15, PCR_Range_DAC_0_P10|PCR_Mode_DAC);
	
  rgDACValues[PORT_1]  = 244;
  rgDACValues[PORT_3]  = 2048;
  rgDACValues[PORT_5]  = 1024;
  rgDACValues[PORT_7]  = 1024;
  rgDACValues[PORT_9]  = 512;
  rgDACValues[PORT_10] = 1024;
  rgDACValues[PORT_11] = 1024;
  rgDACValues[PORT_12] = 1024;
  rgDACValues[PORT_13] = 1024;
  rgDACValues[PORT_14] = 4095;
  rgDACValues[PORT_15] = 4095;
  rgDACValues[PORT_16] = 4095;

/*	
	rgDACValues[PORT_1]  = 0;
	rgDACValues[PORT_3]  = 0;
	rgDACValues[PORT_5]  = 0;
	rgDACValues[PORT_7]  = 0;
	rgDACValues[PORT_9]  = 0;
	rgDACValues[PORT_10] = 0;
	rgDACValues[PORT_11] = 0;
	rgDACValues[PORT_12] = 0;
	rgDACValues[PORT_13] = 0;
	rgDACValues[PORT_14] = 0;
	rgDACValues[PORT_15] = 0;
	rgDACValues[PORT_16] = 0;
*/

	MAX11300_setBuffers(rgADCValues, rgDACValues);
	MAX11300_startContinuous();
}

void run(){
//  Nop_delay(10000000);
//  MAX11300_bulksetDAC();
			
  Nop_delay(10000000);	
//  MAX11300_bulkreadADC();
}
