
#include "stm32f4xx_hal.h"
#include "HAL_TLC5946.h"

// #define TLC_CONTINUOUS
#define TLC_DEVICES 	3

uint8_t rgGSbuf[TLC_DEVICES][24];
uint8_t rgDCbuf[TLC_DEVICES][12];
SPI_HandleTypeDef* TLC5946_SPIConfig;

uint8_t rgLED_R[16] = {14,12,9,8,7,4,2,0,15,13,11,10,6,5,3,1};
uint8_t rgLED_G[16] = {14,12,10,8,7,4,2,0,15,13,11,9,6,5,3,1};
uint8_t rgLED_B[16] = {14,12,9,8,7,4,2,0,15,13,11,10,6,5,3,1};
	
//_____ Functions _____________________________________________________________________________________________________
void TLC5946_SetOutput_GS(unsigned char IC, unsigned char LED_ID, unsigned short value)
{
	unsigned char temp;
	unsigned char ucBuffLoc = LED_ID + (LED_ID>>1); // (unsigned char)(LED_ID*1.5);
	
	if (value < 4096)
	{
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
	}
}

void TLC5946_SetOutput_DC(unsigned char IC, unsigned char LED_ID, unsigned char value)
{
	unsigned char temp;
	unsigned char ucBuffLoc = (unsigned char)(LED_ID*0.75);
	
	if (value < 64)
	{
		if (LED_ID==0 || LED_ID==4 || LED_ID==8 || LED_ID==12)
		{
			temp 											= rgDCbuf[IC][ucBuffLoc];
			rgDCbuf[IC][ucBuffLoc] 		= (value&0x3F);
			rgDCbuf[IC][ucBuffLoc] 	 |= (temp&0xC0);
		}
		else if (LED_ID==1 || LED_ID==5 ||LED_ID==9 || LED_ID==13)
		{
			temp 											= rgDCbuf[IC][ucBuffLoc];
			rgDCbuf[IC][ucBuffLoc] 		= (value&0x03)<<6;
			rgDCbuf[IC][ucBuffLoc] 	 |= (temp&0x3F);
			
			temp 											= rgDCbuf[IC][ucBuffLoc+1];
			rgDCbuf[IC][ucBuffLoc+1] 	= (value&0x0F);
			rgDCbuf[IC][ucBuffLoc+1] |= (temp&0xF0);
		}
		else if (LED_ID==2 || LED_ID==6 || LED_ID==10 || LED_ID==14)
		{
			temp 											= rgDCbuf[IC][ucBuffLoc];
			rgDCbuf[IC][ucBuffLoc] 		= (value&0x0F)<<4;
			rgDCbuf[IC][ucBuffLoc] 	 |= (temp&0x0F);
			
			temp 											= rgDCbuf[IC][ucBuffLoc+1];
			rgDCbuf[IC][ucBuffLoc+1]	= (value&0xC0)>>6;
			rgDCbuf[IC][ucBuffLoc+1] |= (temp&0xFC);
		}
		else if (LED_ID==3 || LED_ID==7 || LED_ID==11 || LED_ID==15)
		{
			temp 											= rgDCbuf[IC][ucBuffLoc];
			rgDCbuf[IC][ucBuffLoc] 		= value<<2;
			rgDCbuf[IC][ucBuffLoc] 	 |= (temp&0x03);
		}
	}		
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
	pBLANK(1);
	
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
	pBLANK(0);
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
void Magus_setRGB(unsigned char LED_ID, unsigned short val_R, unsigned short val_G, unsigned short val_B)
{
	TLC5946_SetOutput_GS(0, rgLED_R[LED_ID-1], val_R);
	TLC5946_SetOutput_GS(2, rgLED_G[LED_ID-1], val_G);
	TLC5946_SetOutput_GS(1, rgLED_B[LED_ID-1], val_B);
}

void Magus_setRGB_DC(unsigned short val_R, unsigned short val_G, unsigned short val_B)
{
	uint8_t x;
	
	for(x=0; x<16; x++)	{TLC5946_SetOutput_DC(0, x, val_R);}
	for(x=0; x<16; x++)	{TLC5946_SetOutput_DC(2, x, val_G);}
	for(x=0; x<16; x++)	{TLC5946_SetOutput_DC(1, x, val_B);}
	
	TLC5946_Refresh_DC();
}


//_____ Initialisaion _________________________________________________________________________________________________
void TLC5946_init (SPI_HandleTypeDef *spiconfig)
{
	// Copy SPI handle to local variable
	TLC5946_SPIConfig = spiconfig;
	
	// 
}
