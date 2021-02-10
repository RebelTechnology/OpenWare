#ifndef __HAL_MAX11300_H
#define __HAL_MAX11300_H

#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif

// Port Mapping
#define PORT_1    0
#define PORT_2    1
#define PORT_3    2
#define PORT_4    3
#define PORT_5    4
#define PORT_6    5
#define PORT_7    6
#define PORT_8    7
#define PORT_9    15
#define PORT_10   14
#define PORT_11   13
#define PORT_12   12
#define PORT_13   11
#define PORT_14   10
#define PORT_15   9
#define PORT_16   8
//#define PORT_17   7
//#define PORT_18   6
//#define PORT_19   5
//#define PORT_20		4
 
// RAM addresses
#define ADDR_Interrupt							0x01
#define ADC_Stat15_0								0x02
#define ADC_Stat16_19								0x03
#define ADDR_GPIO15_0								0x0D
#define ADDR_GPIO19_16							0x0E
#define ADDR_DevCont								0x10
#define ADDR_IntMask								0x11

// Base RAM Addresses
#define ADDR_CFGbase    						0x20
#define ADDR_ADCbase    						0x40
#define ADDR_DACbase    						0x60

// Interrupt Masks
#define INT_VMON										(1<<15)
#define INT_TempExt2								(7<<12)
#define INT_TempExt1								(7<<9)
#define INT_TempInt									(7<<6)
#define INT_DAC_OI									(1<<5)
#define INT_GPI_DM									(1<<4)
#define INT_GPI_DR									(1<<3)
#define INT_ADC_DM									(1<<2)
#define INT_ADC_DR									(1<<1)
#define INT_ADC_Flag 								(1<<0)

// Device Configuration Values
#define DCR_ADCCTL_Idle							(0<<0)
#define DCR_ADCCTL_SingSweep				(1<<0)
#define DCR_ADCCTL_SingConv					(2<<0) 
#define DCR_ADCCTL_ContSweep				(3<<0)

#define DCR_DACCTL_SeqUpdate				(0<<2)
#define DCR_DACCTL_ImmUpdate				(1<<2)
#define DCR_DACCTL_AllDAT1					(2<<2)
#define DCR_DACCTL_AllDAT2					(3<<2)

#define DCR_ADCCONV_200ksps					(0<<4)
#define DCR_ADCCONV_250ksps					(1<<4)
#define DCR_ADCCONV_333ksps					(2<<4)
#define DCR_ADCCONV_400ksps					(3<<4)

#define DCR_DACREF_Ext							(0<<6)
#define DCR_DACREF_Int							(1<<6)

#define DCR_THSHDN_Dis							(0<<7)
#define DCR_THSHDN_En								(1<<7)

#define DCR_TMPCTL0_Dis							(0<<8)
#define DCR_TMPCTL0_En							(1<<8)
#define DCR_TMPCTL1_Dis							(0<<9)
#define DCR_TMPCTL1_En							(1<<9)
#define DCR_TMPCTL2_Dis							(0<<10)
#define DCR_TMPCTL2_En							(1<<10)

#define DCR_TMPPER_Default					(0<<11)
#define DCR_TMPPER_Extended					(1<<11)

#define DCR_RS_CANCEL_Dis						(0<<12)
#define DCR_RS_CANCEL_En						(1<<12)

#define DCR_LPEN_Default						(0<<13)
#define DCR_LPEN_Lower							(1<<13)

#define DCR_BRST_Default						(0<<14)
#define DCR_BRST_Contextual					(1<<14)

// Port Configuration Values
#define PCR_ADCSamples_1            (0<<5)
#define PCR_ADCSamples_2            (1<<5)
#define PCR_ADCSamples_4            (2<<5)
#define PCR_ADCSamples_8            (3<<5)
#define PCR_ADCSamples_16           (4<<5)
#define PCR_ADCSamples_32           (5<<5)
#define PCR_ADCSamples_64           (6<<5)
#define PCR_ADCSamples_128          (7<<5)
 
#define PCR_Range_ADC_None          (0<<8)
#define PCR_Range_ADC_0_P10         (1<<8)    	//  0V to +10V
#define PCR_Range_ADC_M5_P5         (2<<8)    	// -5V to +5V
#define PCR_Range_ADC_M10_0         (3<<8)    	// -10 to 0V
#define PCR_Range_ADC_0_P2p5        (4<<8)    	//   0 to +2.5 
 
#define PCR_Range_DAC_None          (0<<8)
#define PCR_Range_DAC_0_P10         (1<<8)    	//  0V to +10V
#define PCR_Range_DAC_M5_P5         (2<<8)    	// -5V to +5V
#define PCR_Range_DAC_M10_0         (3<<8)    	// -10 to 0V
 
#define PCR_ADCref_INT              (0<<11)
#define PCR_ADCref_EXT              (1<<11)
 
#define PCR_GPI_notINV              (0<<11)
#define PCR_GPI_INV                 (1<<11)
 
#define PCR_Mode_HighZ              (0<<12)     // Mode 0  - High Impedance
#define PCR_Mode_GPI                (1<<12)     // Mode 1  - Digital input with programmable threshold
#define PCR_Mode_BidirTrans         (2<<12)     // Mode 2  - Bidirectional level translator terminal
#define PCR_Mode_GPO                (3<<12)     // Mode 3  - Register-driven digital output with DAC-controlled level
#define PCR_Mode_GPO_Unidir         (4<<12)     // Mode 4  - Unidirectional path output with DAC-controlled level
#define PCR_Mode_DAC               	(5<<12)     // Mode 5  - Analog output for DAC
#define PCR_Mode_DAC_ADCMon         (6<<12)     // Mode 6  - Analog output for DAC with ADC monitoring
#define PCR_Mode_ADC_SgEn_PosIn     (7<<12)     // Mode 7  - Positive analog input to single-ended ADC
#define PCR_Mode_ADC_Diff_PosIn     (8<<12)     // Mode 8  - Positive analog input to differential ADC
#define PCR_Mode_ADC_Diff_NegIn     (9<<12)     // Mode 9  - Negative analog input to differential ADC
#define PCR_Mode_DACOut_ADCNegIn    (10<<12)  	// Mode 10 - Analog output for DAC and negative analog input to differential ADC
#define PCR_Mode_GPISwitch_Term     (11<<12)  	// Mode 11 - Terminal to GPI-controlled analog switch
#define PCR_Mode_RegSwitch_Term  		(12<<12)  	// Mode 12 - Terminal to register-controlled analog switch

// Prototypes
void MAX11300_init (SPI_HandleTypeDef*);

void MAX11300_setPortMode(uint8_t, uint16_t);
uint16_t MAX11300_readPortMode(uint8_t);
void MAX11300_setDeviceControl(uint16_t);
void MAX11300_ConfigPort(uint8_t, uint16_t);

uint16_t MAX11300_readADC(uint8_t port);
void MAX11300_setDAC(uint8_t, uint16_t);
void MAX11300_bulkreadADC(void);
void MAX11300_bulksetDAC(void);
uint16_t MAX11300_getADCValue(uint8_t port);
void MAX11300_setDACValue(uint8_t port, uint16_t value);

void MAX11300_TxINTCallback(void);
void MAX11300_RxINTCallback(void);
void MAX11300_TxRxINTCallback(void);

void MAX11300_startContinuous(void);
void MAX11300_stopContinuous(void);

void Nop_delay(uint32_t);

#ifdef __cplusplus
}
#endif
#endif
