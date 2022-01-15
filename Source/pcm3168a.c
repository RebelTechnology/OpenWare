#include "pcm3168a.h"
#include "gpio.h"
#include "device.h"
#include "main.h"

#define delay(x) HAL_Delay(x)

#define ADC_TIMEOUT       1000

#define setCS()    setPin(ADC_NCS_GPIO_Port, ADC_NCS_Pin)
#define clearCS()  clearPin(ADC_NCS_GPIO_Port, ADC_NCS_Pin)
/* #define setCS()    HAL_GPIO_WritePin(ADC_NCS_GPIO_Port, ADC_NCS_Pin, GPIO_PIN_SET); */
/* #define clearCS()  HAL_GPIO_WritePin(ADC_NCS_GPIO_Port, ADC_NCS_Pin, GPIO_PIN_RESET); */

void codec_write(uint8_t reg, uint8_t data){
 /*  All single write/read operations through the serial control port use 16-bit data words. */
 /* The first bit is for read/write controls; 0 indicates a write operation and 1 indicates a read */
  /* operation. Following the first bit are seven other bits, labeled ADR[6:0] that set the register address for the */
  /* write/read operation. The eight least significant bits (LSBs), D[7:0] on MDI or MDO, contain the data to be written */
  /* to the register specified by ADR[6:0], or the data read from the register specified by ADR[6:0]. */
  clearCS();
  uint8_t buf[2];
  buf[0] = 0b00000000 | (reg & 0x7f); // ADR7 is low to write
  buf[1] = data;
  /* i2c_write(CODEC_ADDR,2,buf); */
  extern SPI_HandleTypeDef CODEC_SPI;
  HAL_SPI_Transmit(&CODEC_SPI, buf, 2, ADC_TIMEOUT);
  setCS();
}


uint8_t codec_read(uint8_t reg){
  clearCS();
  uint8_t buf[2];
  buf[0] = 0b10000000 | (reg & 0x7f); // ADR7 is high to read
  buf[1] = 0;
  extern SPI_HandleTypeDef CODEC_SPI;
  HAL_SPI_TransmitReceive(&CODEC_SPI, buf, buf, 2, ADC_TIMEOUT);
  setCS();
  return buf[1];
}

void codec_reset(){
  /* codec_write(64, PCM3168A_MRST|PCM3168A_SRST); */
/* This bit controls system reset, the relation between system clock and sampling clock re-synchronization, and ADC */
/* operation and DAC operation restart. The mode control register is not reset and the PCM3168A device does not go into a */
/* power-down state. The fade-in sequence is supported in the resume process, but pop-noise may be generated. Returning */
/* the SRST bit to 1 is unneccesary; it is automatically set to 1 after triggering a system reset. */
}

// todo: remove
uint32_t pcm3168a_dz = 0;
uint32_t pcm3168a_adc_ovf = 0;
uint32_t pcm3168a_check = 0;

void codec_init(){
  clearPin(ADC_RESET_GPIO_Port, ADC_RESET_Pin);
  /* HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_RESET); */
  delay(10); // 100nS minimum
  setCS();
  setPin(ADC_RESET_GPIO_Port, ADC_RESET_Pin);
  /* HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_SET); */

  /* the internal reset is released after 3846 SCKI clock cycles from */
  /* power-on if RST is kept high and SCKI is provided. */
  /* for(int i=0; i<10000; ++i){ */
  /*   HAL_GPIO_TogglePin(SAI_MCLK_GPIO_Port, SAI_MCLK_Pin); */
  /*   delay(1); */
  /* } */
  delay(10);

  codec_write(64, 0b11000001); // Normal operation, Single rate

  /* Register: DAC Control 1 */
  codec_write(65, 0b00000110); // Power Save Enable, Slave mode, 24-bit I2S mode TDM format
  /* codec_write(65, 0b10000110); // Power Save Disable, Slave mode, 24-bit I2S mode TDM format */
  /* codec_write(65, 0b10000110); // Power Save Disable, Slave mode, 24-bit I2S mode TDM format */
  /* codec_write(65, 0b10000111); // 24-bit left-justified mode TDM format */
  
  /* Register: DAC Control 2 */
#if AUDIO_CHANNELS == 4
  codec_write(66, 0b11000000); // disable DAC channels 7/8 and 5/6
#elif AUDIO_CHANNELS == 6
  codec_write(66, 0b10000000); // disable DAC channels 7/8
/* These bits control the DAC operation mode. In operation disable mode, the DAC output is cut off from DIN with a fade-out */
/* sequence, and the internal DAC data is reset. DAC output is forced into VCOMDA if PSMDA = 1, or DAC output is forced */
/* into AGNDDA and goes into a power-down state if PSMDA = 0. For normal operating mode, a fade-in sequence is */
/* applied on the DAC output in resume process. The serial mode control is effective during operation disable mode */							 
#endif

  /* Register: DAC Output Phase */
#ifdef CODEC_DAC_INVERT
  codec_write(67, 0xff); // phase invert all DAC channels
#endif

  /* Register: DAC Soft Mute Control */
  /* codec_write(68, 0xff); // enable soft mute for all DAC channels */

  /* 70-79: DAC attenuation control */
/*   Each DAC channel (VOUTx) has a digital attenuator function. The attenuation level can be set from 0 dB to –100 dB in */
/* 0.5-dB steps, and also can be set to infinite attenuation (mute). The attenuation level change from current value to target */
/* value is performed by incrementing or decrementing with s-curve responses and a time set by ATSPDA. */
  codec_write(70, 0b10000000); // ATMDDA=1 All channels with preset (independent) data + master (common) data in decibel number

  /* Data formats, see Table 11 p.33 */

  /* Register: ADC Control 1 */
  codec_write(80, 0b00000001); // Single rate mode
  codec_write(81, 0b00000110); // Slave mode 24-bit I2S mode TDM format 
  /* codec_write(81, 0b00000111); // Slave mode 24-bit left-justified mode TDM format */

  uint8_t value;
#if AUDIO_CHANNELS == 4
  value = 0b01000000; // disable ADC channels 5/6
#else
  value = 0b00000000;
#endif
#ifndef CODEC_HP_FILTER
  value |= 0b00000111;  // disable HPF for all ADC channels
#endif
  /* Register: ADC Control 2 */
  codec_write(82, value);

  /* Register: ADC Input Configuration */
  codec_write(83, 0b00111111); // singled ended inputs on all ADC channels

  /* Register: ADC Input Phase */
#ifdef CODEC_ADC_INVERT
  codec_write(84, 0b00111111); // phase invert all ADC channels
#endif

  /* Register: ADC Overflow Flag */
/*   ADC Overflow flag (read-only) */
/* These bits indicate the status information of an overflow detect circuit for each ADC channel; these bits are read only. 1 */
/* means an overflow has been detected in the past, and reading this register resets all OVF bits. */
  /* codec_read(86); */

  /* 87-94 ADC Attenuation control */
/*   Each ADC channel has a digital attenuator function with 20-dB gain. The attenuation level can be set from 20 dB to –100 */
/* dB in 0.5-dB steps, and also can be set to infinite attenuation (mute). The attenuation level change from current value to */
/* target value is performed by increment or decrement with s-curve response and time set by ATSPAD. */
  codec_write(87, 0b1000000); // ATMDAD=1 All channels with preset (independent) data + master (common) data in decibel number

  /* codec_reset(); */

  pcm3168a_dz = codec_read(69); // DAC Zero flag
  pcm3168a_adc_ovf = codec_read(86); // ADC Overflow
  pcm3168a_check = codec_read(81); // check write success
}

void codec_bypass(int bypass){
  // todo
}

void codec_mute(bool mute){  
  /* Register 68: DAC Soft Mute Control */
  codec_write(68, mute ? 0xff : 0x00);
}

/**
 * Adjustable from 127 = +20dB to -113 = -100dB in 0.5dB steps. Less than -113 is mute.
 */
void codec_set_gain_in(int8_t level){
  /* Register 88: ADC Digital attenuation level setting */
  uint8_t value = level+128;
  codec_write(88, value); // set master attenuation level ATAD0[7:0]
}

/**
 * Adjustable from 127 = 0dB to -73 = -100dB in 0.5dB steps. Less than -73 is mute.
 */
void codec_set_gain_out(int8_t level){
  /* Register 71: DAC Digital attenuation level setting */
  uint8_t value = level+128;
  codec_write(71, value); // set master attenuation level ATDA0[7:0]
}

