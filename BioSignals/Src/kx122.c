#include <stdint.h>
#include "kx122.h"
#include "device.h"
#include "message.h"
#include "stm32f4xx_hal.h"

/* #define SPI_IT */

/* #define SPI_DMA */

extern SPI_HandleTypeDef KX122_HSPI;
static int32_t* kx122_samples;

#define REG_XHP_L         0x00
#define REG_XHP_H         0x01
#define REG_YHP_L         0x02
#define REG_YHP_H         0x03
#define REG_ZHP_L         0x04
#define REG_ZHP_H         0x05
#define REG_XOUT_L        0x06
#define REG_XOUT_H        0x07
#define REG_YOUT_L        0x08
#define REG_YOUT_H        0x09
#define REG_ZOUT_L        0x0A
#define REG_ZOUT_H        0x0B

#define REG_COTR	  0x0C
#define REG_WHO_AM_I      0x0F

#define	REG_INS2	  0x13
#define	REG_STATUS	  0x15
#define REG_INT_REL	  0x17
#define REG_CTRL_REG1     0x18
#define REG_CTRL_REG2     0x19
#define REG_CTRL_REG3     0x1A
#define REG_CTRL_OD	  0x1B
#define REG_INC1	  0x1C
#define REG_INC2	  0x1D
#define REG_INC3	  0x1E
#define REG_INC4	  0x1F
#define REG_INC5	  0x20
#define REG_INC6	  0x21

#define REG_TDTRC	  0x24

#define X_Axis  1
#define Y_Axis  2
#define Z_Axis  3

#define ACCEL_CS_LO()				HAL_GPIO_WritePin(ACC_NCS_GPIO_Port, ACC_NCS_Pin, GPIO_PIN_RESET);
#define ACCEL_CS_HI()				HAL_GPIO_WritePin(ACC_NCS_GPIO_Port, ACC_NCS_Pin, GPIO_PIN_SET);

static uint8_t kx122_buffer[8];

static uint8_t KX122_readReg(uint8_t address)
{
  uint8_t ucAddress = (address & 0x3F) | 1<<7;
  uint8_t ucData = 0;
    
  ACCEL_CS_LO();
  HAL_SPI_Transmit(&KX122_HSPI, &ucAddress, 1, 1000);
  HAL_SPI_Receive(&KX122_HSPI,  &ucData, 	 1, 1000);
  ACCEL_CS_HI();

  return ucData;
}

static void KX122_setReg(uint8_t address, uint8_t data)
{
  uint8_t ucAddress = (address & 0x3F);
    
  ACCEL_CS_LO();
  HAL_SPI_Transmit(&KX122_HSPI, &ucAddress, 1, 1000);
  HAL_SPI_Transmit(&KX122_HSPI, &data, 	    1, 1000);
  ACCEL_CS_HI();
}

void kx122_setup(int32_t* samples)
{
  kx122_samples = samples;

  KX122_setReg(REG_CTRL_REG1,	0x00);	// Standby
  HAL_Delay(1); // most Control register bits can only be set in standby mode
  KX122_setReg(REG_CTRL_REG2, 0x40);	// Set COTC (changes COTR register value from 0x55 to 0xAA)
  // Check if accelerometer is functional
  if(KX122_readReg(REG_COTR) == 0xAA){
    uint8_t ctrl1 = 0x60; // High resolution, interrupt on, +/-2g, tap off, wakeup off, tilt off
    /* uint8_t ctrl1 = 0x68; // High resolution, interrupt on, +/-4g, tap off, wakeup off, tilt off */
    /* uint8_t ctrl1 = 0x70; // High resolution, interrupt on, +/-8g, tap off, wakeup off, tilt off */
    KX122_setReg(REG_CTRL_REG1, ctrl1); // Standby
    KX122_setReg(REG_CTRL_REG2, 0x00);	// Disable all tilt interrupts
    KX122_setReg(REG_CTRL_REG3, 0x98);	// Set to reset value
#if KX122_AUDIO_FREQ == 25600
    KX122_setReg(REG_CTRL_OD,   0x0F);	// Acceleration output to 25.6kHz
#elif KX122_AUDIO_FREQ == 12800
    KX122_setReg(REG_CTRL_OD,   0x0E);	// Acceleration output to 12.8kHz
#elif KX122_AUDIO_FREQ == 6400
    KX122_setReg(REG_CTRL_OD,   0x0D);	// Acceleration output to 6400kHz
#elif KX122_AUDIO_FREQ == 3200
    KX122_setReg(REG_CTRL_OD,   0x0C);	// Acceleration output to 3200Hz
#elif KX122_AUDIO_FREQ == 1600
    KX122_setReg(REG_CTRL_OD,   0x07);	// Acceleration output to 1600Hz
#elif KX122_AUDIO_FREQ == 800
    KX122_setReg(REG_CTRL_OD,   0x06);	// Acceleration output to 800Hz
#elif KX122_AUDIO_FREQ == 100
    KX122_setReg(REG_CTRL_OD,   0x03);	// Acceleration output to 100Hz
#elif KX122_AUDIO_FREQ == 50
    KX122_setReg(REG_CTRL_OD,   0x02);	// Acceleration output to 50Hz
#elif KX122_AUDIO_FREQ == 25
    KX122_setReg(REG_CTRL_OD,   0x01);	// Acceleration output to 25Hz
#elif KX122_AUDIO_FREQ == 12.5
    KX122_setReg(REG_CTRL_OD,   0x00);	// Acceleration output to 12.5Hz
#else
#error "Invalid configuration: missing sample rate KX122_AUDIO_FREQ"
#endif
    KX122_setReg(REG_TDTRC, 0x00);	// Disable tap interrupts
    KX122_setReg(REG_INC1,  0x38);	// INT1 enabled, active high, single pulse
    KX122_setReg(REG_INC4,  0x10);	// DRDY on INT1
    KX122_setReg(REG_INC5,  0x01);	// Auto clear INT1 interrupt		
    KX122_setReg(REG_CTRL_REG1, 0x80|ctrl1);  // Run
  }else{
    error(CONFIG_ERROR, "Could not detect accelerometer");
  }
}

void kx122_set_range(uint8_t range){
  uint8_t reg = KX122_readReg(REG_CTRL_REG1);
  reg &= ~0x18; // mask out two range bits: GSEL1, GSEL0
  switch(range){
  case 2:
    reg |= 0x00;
    break;
  case 4:
    reg |= 0x08;
    break;
  case 8:
    reg |= 0x10;
    break;
  default:
    return;    
  }
  KX122_setReg(REG_CTRL_REG1, reg);
}

void kx122_cplt(){
  ACCEL_CS_HI();
  kx122_samples[0] = ((kx122_buffer[1]<<16) | (kx122_buffer[2]<<24)) >> 8;
  kx122_samples[1] = ((kx122_buffer[3]<<16) | (kx122_buffer[4]<<24)) >> 8;
  kx122_samples[2] = ((kx122_buffer[5]<<16) | (kx122_buffer[6]<<24)) >> 8;
}

void kx122_drdy(){
  ACCEL_CS_LO();
  kx122_buffer[0] = 0x06 | (1<<7);
	
#ifdef SPI_IT
  HAL_SPI_TransmitReceive_IT(&KX122_HSPI, kx122_buffer, kx122_buffer, 7);
#elif defined SPI_DMA
  HAL_SPI_TransmitReceive_DMA(&KX122_HSPI, kx122_buffer, kx122_buffer, 7);
#else
  HAL_SPI_TransmitReceive(&KX122_HSPI, kx122_buffer, kx122_buffer, 7, 10);
  kx122_cplt();
#endif
}
