/*
  Pixi.cpp - Library for Analog shield with Maxim PIXI A/D chip.
  Created by Wolfgang Friedrich, July 29. 2014.
  Will be released into the public domain.
*/

#include "Pixi.h"
#include "stm32f1xx_hal.h"
#include "device.h"
#include "gpio.h"

#define MAX_SOFT_CS
// #define MAX_BITBANG
// #define MAX_HAL
#define MAX_TIMEOUT 100
extern "C"{
  extern SPI_HandleTypeDef hspi1;
}

#ifdef MAX_SOFT_CS
#define setCS()    setPin(MAX_CS_GPIO_Port, MAX_CS_Pin)
#define clearCS()  clearPin(MAX_CS_GPIO_Port, MAX_CS_Pin)
#else
#define setCS()    
#define clearCS()
#endif

#define MAX_SPI        SPI1
#define MAX_DUMMY_BYTE 0x00

void delay(uint32_t ms){
  // spin
  // for(uint32_t i=0; i<ms*100000; ++i)
  //   asm("NOP");

  // DWT_Delay(ms*1000);
}

void spiwrite(uint8_t* data, size_t size){
  HAL_SPI_Transmit(&hspi1, data, size, MAX_TIMEOUT);
}

void spiread(uint8_t* data, size_t size){
  HAL_SPI_Receive(&hspi1, data, size, MAX_TIMEOUT);
}

// void spiwrite(const uint8_t* data, size_t size){
//   while(hspi->State != HAL_SPI_STATE_READY);
//   // clearPin(OLED_SCK_GPIO_Port, OLED_SCK_Pin);
//   // clearCS();
//   HAL_StatusTypeDef ret = HAL_SPI_Transmit(hspi, (uint8_t*)data, size, OLED_TIMEOUT);
//   assert_param(ret == HAL_OK);
//   // setCS();
// }

// Config SPI for communication witht the PIXI
Pixi::Pixi(){}

#ifndef MAX_BITBANG
#define __HAL_RCC_SPI1_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN);\
                                        /* Delay after an RCC peripheral clock enabling */\
                                        tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN);\
                                        /* UNUSED(tmpreg); */  \
                                      } while(0)
#endif

void Pixi::begin(){
#ifdef MAX_SOFT_CS
  /*Configure GPIO pin : MAX_CS_Pin */
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = MAX_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(MAX_CS_GPIO_Port, &GPIO_InitStruct);
#endif

#if 0
#ifndef MAX_BITBANG
  __HAL_RCC_SPI1_CLK_ENABLE();
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
#endif

#ifdef MAX_SOFT_CS
  configureDigitalOutput(MAX_CS_GPIO_Port, MAX_CS_Pin);
#endif

#ifdef MAX_BITBANG
  configureDigitalOutput(MAX_CS_GPIO_Port, MAX_CS_Pin);
  configureDigitalOutput(MAX_SCLK_GPIO_Port, MAX_SCLK_Pin);
  configureDigitalOutput(MAX_DIN_GPIO_Port, MAX_DIN_Pin);
  configureDigitalInput(MAX_DOUT_GPIO_Port, MAX_DOUT_Pin, false);
#else
  GPIO_InitTypeDef GPIO_InitStruct;
#ifdef MAX_SOFT_CS
  GPIO_InitStruct.GPIO_Pin = MAX_SCLK_Pin|MAX_DIN_Pin;
#else
  GPIO_InitStruct.GPIO_Pin = MAX_CS_Pin|MAX_SCLK_Pin|MAX_DIN_Pin;
#endif
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.GPIO_Pin = MAX_DOUT_Pin;
  // GPIO_InitStruct.GPIO_Pull = GPIO_NOPULL;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(MAX_DOUT_GPIO_Port, &GPIO_InitStruct);

  SPI_InitTypeDef SPI_InitStructure;
  SPI_StructInit(&SPI_InitStructure);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  // SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7; // SPI_CRCCALCULATION_DISABLED;
  SPI_Init(MAX_SPI, &SPI_InitStructure);

  SPI_Cmd(MAX_SPI, ENABLE);
#endif
#endif /* 0 */
}

/*
Read register and return value
if the debug parameter is set to TRUE, the register value is printed in format
SPI read adress: 0x0 : h 0x4 : l 0x24
SPI read result 0x424
*/
uint16_t Pixi::ReadRegister(uint8_t address, bool debug = false)
{
    uint16_t result;
    // uint8_t read1 = 0;
    // uint8_t read2 = 0;
    
// if (debug) {
// 		Serial.print("SPI read adress: 0x");
// 		Serial.print(address,HEX);
// 		Serial.print(" : h 0x");
// };          

  // take the SS pin low to select the chip:
  // digitalWrite(slaveSelectPin,LOW);
  clearCS();
  //  send in the address and return value via SPI:

  uint8_t data[2] = { (address << 1) | PIXI_READ };
  spiwrite(data, 1); // write 8-bit register id
  spiread(data, 2); // read 16-bit value
  result = (data[0]<<8) | data[1];
  // SPI_Write( (address << 0x01) | PIXI_READ );
  // result = SPI_Read();
  // read1 = SPI_Read();
  // if (debug) {
  //   Serial.print(read1,HEX);
  // };
  // read2 = SPI_Read();
  // if (debug) {
  //		Serial.print(" : l 0x");
  //		Serial.println(read2,HEX);
  // };

  // result = (read1 << 8) | read2;
  // take the SS pin high to de-select the chip:
  // digitalWrite(slaveSelectPin,HIGH); 
  setCS();

// if (debug) {
// 		Serial.print("SPI read result 0x");
// 		Serial.println(result,HEX);
// }
  return result;
}


/*
write value into register
Address needs to be shifted up 1 bit, LSB is read/write flag
hi data byte is sent first, lo data byte last.
*/
void Pixi::WriteRegister(uint8_t address, uint16_t value)
{
//    uint16_t result = 0;
    // int8_t value_hi =0;
    
  // take the SS pin low to select the chip:
  // digitalWrite(slaveSelectPin,LOW);
  clearCS();
  //  send in the address and value via SPI:
  uint8_t data[3];
  data[0] = (address << 0x01) | PIXI_WRITE;
  data[1] = value >> 8;
  data[2] = value & 0xFF;
  spiwrite(data, 3);
  // SPI_Write( (address << 0x01) | PIXI_WRITE );
  // SPI_Write( uint8_t( value >> 8));
  // SPI_Write( uint8_t( value & 0xFF));

  // take the SS pin high to de-select the chip:
  setCS();
  // digitalWrite(slaveSelectPin,HIGH); 

  return; // (result);
}


/*
General Config for the PixiShield
Read ID register to make sure the shield is connected.
*/
uint16_t Pixi::config()
{
  uint16_t result = 0;
  uint16_t info = 0;

  result = ReadRegister( PIXI_DEVICE_ID, true );

  // if (result == 0x0424) {
// enable default burst, thermal shutdown, leave conversion rate at 200k
     WriteRegister (PIXI_DEVICE_CTRL, (!BRST) | THSHDN ); // ADCCONV = 00 default.
// enable internal temp sensor
    // disable series resistor cancelation
    info = ReadRegister ( PIXI_DEVICE_CTRL, false );
    WriteRegister (PIXI_DEVICE_CTRL, info | !RS_CANCEL );
    // keep TMPINTMONCFG at default 4 samples
	
    // Set int temp hi threshold
    WriteRegister (PIXI_TEMP_INT_HIGH_THRESHOLD, 0x0230 );		// 70 deg C in .125 steps
    // Keep int temp lo threshold at 0 deg C, negative values need function to write a two's complement number.
    // enable internal and both external temp sensors
    info = ReadRegister ( PIXI_DEVICE_CTRL, false );
    WriteRegister (PIXI_DEVICE_CTRL, info | TMPCTLINT | TMPCTLEXT1 | TMPCTLEXT2 );	
  // }
  return (result);  
}



/*
Channel Config
Parameters that are not used for the selected channel are ignored.
*/

uint16_t Pixi::configChannel( int channel, int channel_mode, uint16_t dac_dat, uint16_t range, uint8_t adc_ctl )
{
  uint16_t result = 0;
  uint16_t info = 0;

if ( ( channel <= 19 ) && ( channel_mode <= 12 ) )
{

if (channel_mode == CH_MODE_1  ||
    channel_mode == CH_MODE_3  ||
    channel_mode == CH_MODE_4  ||
    channel_mode == CH_MODE_5  ||
    channel_mode == CH_MODE_6  ||
    channel_mode == CH_MODE_10 ) 
    {
      // config DACREF (internal reference),DACCTL (sequential update)
      info = ReadRegister ( PIXI_DEVICE_CTRL, true );
      WriteRegister (PIXI_DEVICE_CTRL, info | DACREF | !DACCTL );
      delay (1);
      info = ReadRegister ( PIXI_DEVICE_CTRL, true );
      // Enter DACDAT
      WriteRegister ( PIXI_DAC_DATA + channel, dac_dat);
      // Mode1: config FUNCID, FUNCPRM (non-inverted default)
      if (channel_mode == CH_MODE_1)
      {
        WriteRegister ( PIXI_PORT_CONFIG + channel, ( ( (CH_MODE_1 << 12 ) & FUNCID ) |
                                                      ( (range << 8 ) & FUNCPRM_RANGE ) ) );
        
      };
      delay (1);
      // Mode3: config GPO_DAT, leave channel at logic level 0
      if (channel_mode == CH_MODE_3)
      {
        if ( channel <= 15 )
        {
          WriteRegister ( PIXI_GPO_DATA_0_15, 0x00);
        }
        else if (channel >= 16 )
        {
          WriteRegister ( PIXI_GPO_DATA_16_19, ( 0x00 )  );
        };
      }
      // Mode3,4,5,6,10: config FUNCID, FUNCPRM (non-inverted default)
      if (channel_mode == CH_MODE_3  ||
          channel_mode == CH_MODE_5  ||
          channel_mode == CH_MODE_6  ||
          channel_mode == CH_MODE_10)
      {
        WriteRegister ( PIXI_PORT_CONFIG + channel, ( ( (channel_mode << 12 ) & FUNCID ) |
                                                      ( (range << 8 ) & FUNCPRM_RANGE ) ) );
        
      }
      else if (channel_mode == CH_MODE_4  )
      {
        WriteRegister ( PIXI_PORT_CONFIG + channel, ( ( (channel_mode << 12 ) & FUNCID ) |
                                                      ( (range << 8 ) & FUNCPRM_RANGE )
                                                       // assoc port & FUNCPRM_ASSOCIATED_PORT                                                          
                                                      ) );
        
      }
      delay (1);
       // Mode1: config GPIMD (leave at default INT never asserted
      if (channel_mode == CH_MODE_1)
      {
//        WriteRegister ( PIXI_GPI_IRQ_MODE_0_7, 0 );
        
      }
      delay (1);
       
    }

else if (channel_mode == CH_MODE_7  ||
         channel_mode == CH_MODE_8  ||
         channel_mode == CH_MODE_9  ){

      // Mode9: config FUNCID, FUNCPRM
      if (channel_mode == CH_MODE_9)
      {
        WriteRegister ( PIXI_PORT_CONFIG + channel, ( ( (channel_mode << 12 ) & FUNCID ) |
                                                      ( (range << 8 ) & FUNCPRM_RANGE )
                                                      ) );
      }
      delay (1);
      if (channel_mode == CH_MODE_7  ||
          channel_mode == CH_MODE_8)
      {
        WriteRegister ( PIXI_PORT_CONFIG + channel, ( ( (channel_mode << 12 ) & FUNCID ) |
                                                      ( (range << 8 ) & FUNCPRM_RANGE ) 
                                                      ) );
      }
      delay (1);

      // config ADCCTL 
      info = ReadRegister ( PIXI_DEVICE_CTRL, false );
      WriteRegister (PIXI_DEVICE_CTRL, info | ( adc_ctl & ADCCTL ) );
      delay (1);
         
}
else if (channel_mode == CH_MODE_2  ||
         channel_mode == CH_MODE_11 ||
         channel_mode == CH_MODE_12 ){

        WriteRegister ( PIXI_PORT_CONFIG + channel, ( ( (channel_mode << 12 ) & FUNCID ) |
                                                      ( (range << 8 ) & FUNCPRM_RANGE ) 
                                                      ) );

         
         
};

}
  return (result);
};


/*
void Pixi::configTempChannel()
{

}
*/

/*
void Pixi::configInterrupt()
{

}
*/

/*
Readout of raw register value for given temperature channel
*/
uint16_t Pixi::readRawTemperature(int temp_channel)
{
  uint16_t result = 0;
  
  result = ReadRegister( PIXI_INT_TEMP_DATA + temp_channel, false ); // INT_TEMP_DATA is the lowest temp data adress, channel runs from 0 to 2.

  return (result);

}  

/*
Readout of given temperature channel and conversion into degC float return value
*/
float Pixi::readTemperature(int temp_channel)
{
  float result = 0;
  uint16_t rawresult = 0;
  bool sign = 0;
  
  rawresult =  ReadRegister( PIXI_INT_TEMP_DATA + temp_channel, false ); // INT_TEMP_DATA is the lowest temp data adress, channel runs from 0 to 2.

  sign = ( rawresult & 0x0800 ) >> 11; 
  
  if (sign == 1){
	rawresult = ( ( rawresult & 0x07FF ) xor 0x07FF ) + 1;	// calc absolut value from 2's comnplement
  }
  
  result = 0.125 * ( rawresult & 0x0007 ) ;	// pick only lowest 3 bit for value left of decimal point  
											// One LSB is 0.125 deg C
  result = result + ( ( rawresult >> 3) & 0x01FF ) ;
  
  if (sign == 1)
    result = result * -1;	// fix sign
  return (result);
}  

uint16_t Pixi::readAnalog(int ch){
  return ReadRegister( PIXI_ADC_DATA+CHANNEL_0+ch, false);
}

/*
 * output analog value when channel is configured in mode 5
 */
uint16_t Pixi::writeAnalog(int channel, uint16_t value){
  uint16_t result = 0;
  uint16_t channel_func = 0;
 channel += CHANNEL_0;

 channel_func = ReadRegister (  PIXI_PORT_CONFIG + channel, false ); 
 channel_func = ( channel_func & FUNCID ) >> 12 ;

 if(channel_func == 5){
   WriteRegister ( PIXI_DAC_DATA + channel, value );
   result = ReadRegister ( PIXI_DAC_DATA + channel, false );
 }
 return result;
}

