#include "ads.h"
// #include "spi.h"
#include "device.h"
// #include "gpio.h"
// #include "delay.h"
#include "ads1298.h"
#include "main.h"

#define ADS_CS_LO()	HAL_GPIO_WritePin(ADC_NCS_GPIO_Port, ADC_NCS_Pin, GPIO_PIN_RESET);
#define ADS_CS_HI()	HAL_GPIO_WritePin(ADC_NCS_GPIO_Port, ADC_NCS_Pin, GPIO_PIN_SET);
#define ADS_RESET_LO()	HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_RESET);
#define ADS_RESET_HI()	HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_SET);

#define ADS_HSPI hspi1
#define ADS_SPI_TIMEOUT 800

uint32_t ads_status = 0;
int ads_timestamp;

uint8_t spi_transfer(uint8_t tx){
  extern SPI_HandleTypeDef ADS_HSPI;
  uint8_t rx;
  HAL_SPI_TransmitReceive(&ADS_HSPI, &tx, &rx, 1, ADS_SPI_TIMEOUT);
  return rx;
}

void spi_cs(bool high){
  if(high){ // chip disable
    ADS_CS_HI();
  }else{
    ADS_CS_LO();
  }
}

void spi_setup(){
  ADS_RESET_HI();
  HAL_Delay(1);
  ADS_RESET_LO();
  spi_cs(true);
}

// #include "stm32f4xx_ll_iwdg.h"
// #define KICK_WDT() LL_IWDG_ReloadCounter()
// #define SYSTEM_US_TICKS		(SystemCoreClock / 1000000)//cycles per microsecon
#define SYSTEM_MS_TICKS		(SystemCoreClock / 1000) // cycles per millisecond

void delay_us(uint32_t uSec){
  while(uSec--)
    asm("NOP");
  // volatile uint32_t DWT_START = DWT->CYCCNT;
  // // keep DWT_TOTAL from overflowing (max 59.652323s w/72MHz SystemCoreClock)
  // if(uSec > (UINT32_MAX / SYSTEM_US_TICKS))
  //   uSec = (UINT32_MAX / SYSTEM_US_TICKS);
  // volatile uint32_t DWT_TOTAL = (SYSTEM_US_TICKS * uSec);
  // while((DWT->CYCCNT - DWT_START) < DWT_TOTAL){
  //   KICK_WDT();
  // }
}

int ads_read_single_sample(){
  // the ADS129x outputs 24 bits of data per channel in binary twos complement format, MSB first.
  // convert 24bit to 32bit signed
  return (spi_transfer(0) << 24) | (spi_transfer(0) << 16) | (spi_transfer(0) << 8);
}

void ads_sample(int32_t* samples, size_t len){
  spi_cs(false); // chip enable
  // 24-bits status header plus 24 bits per channel
  ads_status = (uint32_t)ads_read_single_sample()>>8;
  ads_timestamp = (int)(DWT->CYCCNT / SYSTEM_MS_TICKS);
  for(size_t i=0; i<len; ++i)
    samples[i] = ads_read_single_sample();
  spi_cs(true); // chip disable
}

void ads_send_command(int cmd){
  spi_cs(false); // chip enable
  // clearPin(SPI_PORT, SPI_CS_PIN);
  // digitalWrite(IPIN_CS, LOW);
  spi_transfer(cmd);
  // SPI.transfer(cmd);
  delay_us(1);
  spi_cs(true); // chip disable
  // setPin(SPI_PORT, SPI_CS_PIN);
  // digitalWrite(IPIN_CS, HIGH);
}

void ads_write_reg(int reg, int val){
  //see pages 40,43 of datasheet - 
  spi_cs(false); // chip enable
  // clearPin(SPI_PORT, SPI_CS_PIN);
  // digitalWrite(IPIN_CS, LOW);
  spi_transfer(ADS1298::WREG | reg);
  spi_transfer(0);	// number of registers to be read/written – 1
  spi_transfer(val);
  delay_us(1);
  spi_cs(true); // chip disable
  // setPin(SPI_PORT, SPI_CS_PIN);
  // digitalWrite(IPIN_CS, HIGH);
}

int ads_read_reg(int reg){
  int out = 0;
  spi_cs(false); // chip enable
  // clearPin(SPI_PORT, SPI_CS_PIN);
  // digitalWrite(IPIN_CS, LOW);
  spi_transfer(ADS1298::RREG | reg);
  delay_us(5);
  spi_transfer(0);	// number of registers to be read/written – 1
  delay_us(5);
  out = spi_transfer(0);
  delay_us(1);
  spi_cs(true); // chip disable
  // setPin(SPI_PORT, SPI_CS_PIN);
  // digitalWrite(IPIN_CS, HIGH);
  return(out);
}

