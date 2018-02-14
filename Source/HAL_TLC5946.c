#include "HAL_TLC5946.h"
#include <string.h>

// #define TLC_CONTINUOUS
#define TLC_DEVICES 	3

static uint8_t rgGSbuf[TLC_DEVICES][24];
static uint8_t rgDCbuf[TLC_DEVICES][12];
static SPI_HandleTypeDef* TLC5946_SPIConfig;

static const uint8_t rgLED_R[16] = {14,12,9,8,7,4,2,0,15,13,11,10,6,5,3,1};
static const uint8_t rgLED_G[16] = {14,12,10,8,7,4,2,0,15,13,11,9,6,5,3,1};
static const uint8_t rgLED_B[16] = {14,12,9,8,7,4,2,0,15,13,11,10,6,5,3,1};

// see also https://yurichev.com/blog/FAT12/ for optimised ASM

//_____ Functions _____________________________________________________________________________________________________
void TLC5946_SetOutput_GS(uint8_t IC, uint8_t LED_ID, uint16_t value)
{
#if 1
  uint8_t temp;
  uint8_t ucBuffLoc = LED_ID + (LED_ID>>1); // (uint8_t)(LED_ID*1.5);
  if(LED_ID & 0x01)	// bbbbaaaa aaaaaaaa
    {
      temp			= rgGSbuf[IC][ucBuffLoc];
      rgGSbuf[IC][ucBuffLoc] 	= (value&0xF00)>>8;
      rgGSbuf[IC][ucBuffLoc]   |= (temp&0xF0);
      rgGSbuf[IC][ucBuffLoc+1]  = (value&0x0FF);
    }
  else            	// aaaaaaaa aaaabbbb
    {
      rgGSbuf[IC][ucBuffLoc] 	= (value&0xFF0)>>4;
      temp 			= rgGSbuf[IC][ucBuffLoc+1];
      rgGSbuf[IC][ucBuffLoc+1]  = (value&0x00F)<<4;
      rgGSbuf[IC][ucBuffLoc+1] |= (temp&0x0F);
    }
#elif 0
  uint32_t bitshift = LED_ID*12;
  uint32_t word = bitshift/8;
  uint32_t bit = bitshift % 8;
  uint8_t* data = (uint8_t*)(rgGSbuf[IC]);
  data[word+1] = (data[word+1] & ~(0xfffu << bit)) | ((value & 0xfffu) << bit);
  data[word] = (data[word] & ~(0xfffu >> (8-bit))) | (value & 0xfffu) >> (8-bit);
#else
  uint32_t bitshift = LED_ID*12;
  uint32_t word = bitshift/32;
  uint32_t bit = bitshift % 32;
  uint32_t* data = (uint32_t*)(rgGSbuf[IC]);
  data[word] = (data[word] & ~(0xfffu << bit)) | ((value & 0xfffu) << bit);
  if(bit > 20)
    data[word+1] = (data[word+1] & ~(0xfffu >> (32-bit))) | (value & 0xfffu) >> (32-bit);
#endif
}

void TLC5946_SetOutput_DC(uint8_t IC, uint8_t LED_ID, uint8_t value)
{
#if 0
  uint8_t temp;
  /* uint8_t ucBuffLoc = (uint8_t)(LED_ID*0.75); */
  uint8_t ucBuffLoc = (LED_ID*3)>>2;
  switch(LED_ID){
  case 0:
  case 4:
  case 8:
  case 12:
    temp = rgDCbuf[IC][ucBuffLoc];
    rgDCbuf[IC][ucBuffLoc] = (value&0x3F);
    rgDCbuf[IC][ucBuffLoc] |= (temp&0xC0);
    break;
  case 1:
  case 5:
  case 9:
  case 13:
    temp = rgDCbuf[IC][ucBuffLoc];
    rgDCbuf[IC][ucBuffLoc]          = (value&0x03)<<6;
    rgDCbuf[IC][ucBuffLoc]   |= (temp&0x3F);
    temp = rgDCbuf[IC][ucBuffLoc+1];
    rgDCbuf[IC][ucBuffLoc+1]        = (value&0x0F);
    rgDCbuf[IC][ucBuffLoc+1] |= (temp&0xF0);
    break;
  case 2:
  case 6:
  case 10:
  case 14:
    temp = rgDCbuf[IC][ucBuffLoc];
    rgDCbuf[IC][ucBuffLoc]          = (value&0x0F)<<4;
    rgDCbuf[IC][ucBuffLoc]   |= (temp&0x0F);
    temp = rgDCbuf[IC][ucBuffLoc+1];
    rgDCbuf[IC][ucBuffLoc+1]        = (value&0xC0)>>6;
    rgDCbuf[IC][ucBuffLoc+1] |= (temp&0xFC);
    break;
  case 3:
  case 7:
  case 11:
  case 15:
    temp = rgDCbuf[IC][ucBuffLoc];
    rgDCbuf[IC][ucBuffLoc]          = value<<2;
    rgDCbuf[IC][ucBuffLoc]   |= (temp&0x03);
    break;
  }               
#elif 1
  uint32_t bitshift = LED_ID*6;
  uint32_t word = bitshift/8;
  uint32_t bit = bitshift % 8;
  uint8_t* data = (uint8_t*)(rgDCbuf[IC]);
  data[word] = (data[word] & ~(0x3fu << bit)) | ((value & 0x3fu) << bit);
  if(bit > 2)
    data[word+1] = (data[word+1] & ~(0x3fu >> (8-bit))) | (value & 0x3fu) >> (8-bit);
#else
  uint32_t bitshift = LED_ID*6;
  uint32_t word = bitshift/32;
  uint32_t bit = bitshift % 32;
  uint32_t* data = (uint32_t*)(rgDCbuf[IC]);
  data[word] = (data[word] & ~(0x3fu << bit)) | ((value & 0x3fu) << bit);
  if(bit > 26)
    data[word+1] = (data[word+1] & ~(0x3fu >> (32-bit))) | (value & 0x3fu) >> (32-bit);
#endif
}


void TLC5946_TxINTCallback(void)
{
	// Latch pulse
	pXLAT(1);
	pBLANK(1);
	pXLAT(0);

#ifdef TLC_CONTINUOUS
	pBLANK(0);
	HAL_SPI_Transmit_IT(TLC5946_SPIConfig, rgGSbuf[0], sizeof rgGSbuf[0]);	// IC 0 
	HAL_SPI_Transmit_IT(TLC5946_SPIConfig, rgGSbuf[1], sizeof rgGSbuf[1]);	// IC 1
	HAL_SPI_Transmit_IT(TLC5946_SPIConfig, rgGSbuf[2], sizeof rgGSbuf[2]);	// IC 3
#endif
}

void TLC5946_Refresh_GS(void)
{
	// Update Grayscale
	pMODE(Mode_GS);
	pXLAT(0);
	/* pBLANK(1); */
	
#ifdef TLC_CONTINUOUS	
	HAL_SPI_Transmit_IT(TLC5946_SPIConfig, rgGSbuf[0], sizeof rgGSbuf[0]);		// IC 0 
	HAL_SPI_Transmit_IT(TLC5946_SPIConfig, rgGSbuf[1], sizeof rgGSbuf[1]);		// IC 1
	HAL_SPI_Transmit_IT(TLC5946_SPIConfig, rgGSbuf[2], sizeof rgGSbuf[2]);		// IC 3
#else
	HAL_SPI_Transmit(TLC5946_SPIConfig, rgGSbuf[0], sizeof rgGSbuf[0], 100);	// IC 0 
	HAL_SPI_Transmit(TLC5946_SPIConfig, rgGSbuf[1], sizeof rgGSbuf[1], 100);	// IC 1
	HAL_SPI_Transmit(TLC5946_SPIConfig, rgGSbuf[2], sizeof rgGSbuf[2], 100);	// IC 3
	
	// Latch pulse
	pXLAT(1);
	/* HAL_Delay(1); */
	/* pBLANK(0); */
#endif
}

void TLC5946_Refresh_DC(void)
{
	pXLAT(0);
	pMODE(Mode_DC);
	
	// Update Dot Correction
	HAL_SPI_Transmit(TLC5946_SPIConfig, rgDCbuf[0], sizeof rgDCbuf[0], 100);	// IC 0
	HAL_SPI_Transmit(TLC5946_SPIConfig, rgDCbuf[1], sizeof rgDCbuf[1], 100);	// IC 1
	HAL_SPI_Transmit(TLC5946_SPIConfig, rgDCbuf[2], sizeof rgDCbuf[2], 100);	// IC 3
	
	// Latch pulse
	pXLAT(1);
	HAL_Delay(1);
	pXLAT(0);
}

// _____ Magus Functions _____
void TLC5946_setRGB(uint8_t LED_ID, uint16_t val_R, uint16_t val_G, uint16_t val_B)
{
	TLC5946_SetOutput_GS(0, rgLED_R[LED_ID-1], val_R);
	TLC5946_SetOutput_GS(2, rgLED_G[LED_ID-1], val_G);
	TLC5946_SetOutput_GS(1, rgLED_B[LED_ID-1], val_B);
}

void TLC5946_setRGB_DC(uint16_t val_R, uint16_t val_G, uint16_t val_B)
{
	uint8_t x;
	
	for(x=0; x<16; x++)	{TLC5946_SetOutput_DC(0, x, val_R);}
	for(x=0; x<16; x++)	{TLC5946_SetOutput_DC(2, x, val_G);}
	for(x=0; x<16; x++)	{TLC5946_SetOutput_DC(1, x, val_B);}
	
	TLC5946_Refresh_DC();
}

void TLC5946_setAll(uint16_t val_R, uint16_t val_G, uint16_t val_B){
  for(int i=0; i<16; i++){
    TLC5946_SetOutput_GS(0, i, val_R);
    TLC5946_SetOutput_GS(2, i, val_G);
    TLC5946_SetOutput_GS(1, i, val_B);
  }
}


//_____ Initialisaion _________________________________________________________________________________________________
void TLC5946_init (SPI_HandleTypeDef *spiconfig)
{
	// Copy SPI handle to local variable
	TLC5946_SPIConfig = spiconfig;
	
	// 
}
