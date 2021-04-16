#include "HAL_TLC5946.h"
#include <string.h>
#include "main.h"

// #define TLC_CONTINUOUS
#define TLC_DEVICES 	3

#define TLC_GS_BYTES 24
#define TLC_DC_BYTES 12

static uint8_t rgGSbuf[TLC_DEVICES*TLC_GS_BYTES+1];
static uint8_t rgDCbuf[TLC_DEVICES*TLC_DC_BYTES+1] =
  {
   12, 48, 195, 12, 48, 195, 12, 48, 195, 12, 48, 195, // DC=3 Red
   12, 48, 195, 12, 48, 195, 12, 48, 195, 12, 48, 195, // DC=3 Blue
   12, 48, 195, 12, 48, 195, 12, 48, 195, 12, 48, 195, // DC=3 Green
  };
static SPI_HandleTypeDef* TLC5946_SPIConfig;

static const uint8_t rgLED_R[16] = {14,12,9,8,7,4,2,0,15,13,11,10,6,5,3,1};
static const uint8_t rgLED_G[16] = {14,12,10,8,7,4,2,0,15,13,11,9,6,5,3,1};
static const uint8_t rgLED_B[16] = {14,12,9,8,7,4,2,0,15,13,11,10,6,5,3,1};

// see also https://yurichev.com/blog/FAT12/ for optimised ASM

//_____ Functions _____________________________________________________________________________________________________
void TLC5946_SetOutput_GS(uint8_t IC, uint8_t led, uint16_t value)
{
  // 12-bit pwm value
  // aaaaaaaa aaaabbbb bbbbbbbb
  uint32_t bitshift = led*12;
  uint32_t word = bitshift/8;
  uint32_t pos = 8 - (bitshift+4) % 8;
  uint8_t* data = rgGSbuf+IC*TLC_GS_BYTES+word;
  uint8_t mask = 0xfffu >> pos;
  *data = (*data & ~mask) | ((value >> pos) & mask);
  pos = 8 - pos;
  mask = 0xfffu << pos;
  data++;
  *data = (*data & ~mask) | ((value << pos) & mask);
}

void TLC5946_SetOutput_DC(uint8_t ic, uint8_t led, uint8_t value)
{
  // 6-bit dot correction
  // aaaaaabb bbbbcccc ccdddddd
  value <<= 2;
  uint32_t bitshift = led*6;
  uint32_t word = bitshift/8;
  uint32_t pos = bitshift % 8;
  uint8_t mask = 0x3fu >> pos;
  uint8_t* data = rgDCbuf+ic*TLC_DC_BYTES+word;
  *data = (*data & ~mask) | ((value >> pos) & mask);
  pos = 8 - pos;
  mask = 0x3fu << pos;
  data++;
  *data = (*data & ~mask) | ((value << pos) & mask);
}


void TLC5946_TxINTCallback(void)
{
	// Latch pulse
	pXLAT(1);
	/* pBLANK(1); */
	pXLAT(0);

#ifdef TLC_CONTINUOUS
	/* pBLANK(0); */
	HAL_SPI_Transmit_IT(TLC5946_SPIConfig, rgGSbuf, TLC_GS_BYTES*TLC_DEVICES);
#endif
}

void TLC5946_Refresh_GS(void)
{
	// Update Grayscale
	pMODE(Mode_GS);
	pXLAT(0);
	/* pBLANK(1); */
	
#ifdef TLC_CONTINUOUS	
	HAL_SPI_Transmit_IT(TLC5946_SPIConfig, rgGSbuf, TLC_GS_BYTES*TLC_DEVICES);
#else
	HAL_SPI_Transmit(TLC5946_SPIConfig, rgGSbuf, TLC_GS_BYTES*TLC_DEVICES, 100);
	
	// Latch pulse
	pXLAT(1);
	/* HAL_Delay(1); */
	/* pBLANK(0); */
#endif
}

void TLC5946_Refresh_DC(void)
{
	pMODE(Mode_DC);
	pXLAT(0);
	
	// Update Dot Correction
	HAL_SPI_Transmit(TLC5946_SPIConfig, (uint8_t*)rgDCbuf, TLC_DC_BYTES*TLC_DEVICES, 100);
	
	// Latch pulse
	pXLAT(1);
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
