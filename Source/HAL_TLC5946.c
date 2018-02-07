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
	
//_____ Functions _____________________________________________________________________________________________________
void TLC5946_SetOutput_GS(uint8_t IC, uint8_t LED_ID, uint16_t value)
{
	uint8_t temp;
	uint8_t ucBuffLoc = LED_ID + (LED_ID>>1); // (uint8_t)(LED_ID*1.5);
	
	  if(LED_ID & 0x01)	// bbbbaaaa aaaaaaaa
		{
			temp											= rgGSbuf[IC][ucBuffLoc];
			rgGSbuf[IC][ucBuffLoc] 	 	= (value&0xF00)>>8;
			rgGSbuf[IC][ucBuffLoc]   |= (temp&0xF0);
			rgGSbuf[IC][ucBuffLoc+1]  = (value&0x0FF);
		}
	  else            	// aaaaaaaa aaaabbbb
		{
			rgGSbuf[IC][ucBuffLoc] 	  = (value&0xFF0)>>4;
			temp 								  		= rgGSbuf[IC][ucBuffLoc+1];
			rgGSbuf[IC][ucBuffLoc+1]  = (value&0x00F)<<4;
			rgGSbuf[IC][ucBuffLoc+1] |= (temp&0x0F);
		}

  /* uint8_t bitshift = LED_ID*12; */
  /* uint8_t word = bitshift/32; */
  /* uint8_t bit = bitshift % 32; */
  /* uint32_t* data = (uint32_t*)(rgGSbuf[IC]); */
  /* data[word] = (data[word] & ~(0xfffu << bit)) | ((value & 0xfff) << bit); */
  /* if(bit > 20) */
  /*   data[word+1] = (data[word+1] & ~(0xfffu >> (32-bit))) | (value & 0xfff) >> (32-bit); */
}

void TLC5946_SetOutput_DC(uint8_t IC, uint8_t LED_ID, uint8_t value)
{
  uint8_t bitshift = LED_ID*6;
  uint8_t word = bitshift/32;
  uint8_t bit = bitshift % 32;
  uint32_t* data = (uint32_t*)(rgDCbuf[IC]);
  data[word] = (data[word] & ~(0x3fu << bit)) | ((value & 0x3f) << bit);
  if(bit > 26)
    data[word+1] = (data[word+1] & ~(0x3fu >> (32-bit))) | (value & 0x3f) >> (32-bit);
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
