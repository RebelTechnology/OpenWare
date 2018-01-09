// _____ Includes ______________________________________________________________________
#include "stm32f4xx_hal.h"
#include "oled.h"
#include <string.h>

// _____ Defines _______________________________________________________________________
#define pRST_Set()	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, 	OLED_RST_Pin, GPIO_PIN_SET)
#define pDC_Set()		HAL_GPIO_WritePin(OLED_DC_GPIO_Port, 		OLED_DC_Pin, 	GPIO_PIN_SET)
#define pCS_Set()		HAL_GPIO_WritePin(OLED_nCS_GPIO_Port, 	OLED_nCS_Pin, 	GPIO_PIN_SET)

#define pRST_Clr()	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, 	OLED_RST_Pin, GPIO_PIN_RESET)
#define pDC_Clr()		HAL_GPIO_WritePin(OLED_DC_GPIO_Port, 		OLED_DC_Pin, 	GPIO_PIN_RESET)
#define pCS_Clr()		HAL_GPIO_WritePin(OLED_nCS_GPIO_Port, 	OLED_nCS_Pin, 	GPIO_PIN_RESET)

#define OLED_DAT	1
#define OLED_CMD	0

//#define DMA_Comms

// _____ Prototypes ____________________________________________________________________
void OLED_writeCMD(const uint8_t* data, uint16_t length);

// Delay 
static void NopDelay(uint32_t nops)
{
	while (nops--)	
	  __asm("NOP");
}

// _____ Variables _____________________________________________________________________
static const uint8_t OLED_initSequence[] = 
{
	0xfd, 0x12, 	// Command lock
	0xae, 				// Display off
	0xd5, 0xa0, 	// Clock divide ratio / Oscillator Frequency
	0xa8, 0x3f, 	// Multiplex ratio
	0xd3, 0x00, 	// Display offset
	0x40, 				// Start line
	0xa1, 				// segment re-map
	0xc8, 				// scan direction
	0xda, 0x12, 	// COM pins
	0x81, 0xcf, 	// Current control
	0xd9, 0x22, 	// Pre-charge period
	0xdb, 0x34, 	// VCOMH deselect level
	0xa4, 				// Entire display on/off
	0xa6, 				// Normal / inverse display
	0x20, 0x01,   // Vertical addressing mode
	0xaf, 				// Display on
};
 static unsigned char* OLED_Buffer;
static SPI_HandleTypeDef* OLED_SPIInst;
	
// _____ Functions _____________________________________________________________________
void OLED_writeCMD(const uint8_t* data, uint16_t length)
{
	pCS_Clr();	// CS low
	pDC_Clr();	// DC low		
	
	// Send Data	
	HAL_SPI_Transmit(OLED_SPIInst, (uint8_t*)data, length, 1000);
	
	pCS_Set();	// CS high
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi){
  assert_param(0);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
  if(__HAL_DMA_GET_FLAG(OLED_SPIInst,  __HAL_DMA_GET_TC_FLAG_INDEX(OLED_SPIInst))){
    pCS_Set();	// CS high
  }
}

/* void OLED_writeDAT(const uint8_t* data, uint16_t length) */
void OLED_write(const uint8_t* data, uint16_t length)
{
	pCS_Clr();	// CS low
	pDC_Set();	// DC high
	
	// Send Data
	#ifdef DMA_Comms
		HAL_SPI_Transmit_DMA(OLED_SPIInst, (uint8_t*)data, length/8);
	
	#else
		HAL_SPI_Transmit(OLED_SPIInst, (uint8_t*)data, length, 1000); 
		pCS_Set();	// CS high 
	
	#endif	
}

 void OLED_Refresh(void) 
 { 
 	// Write entire buffer to OLED 
 	OLED_write(OLED_Buffer, 1024); 
 } 

 void OLED_ClearScreen(void) 
 { 
	// Clear contents of OLED buffer 
 	memset(OLED_Buffer, 0, 1024); 
 } 

// Configuration
/* void OLED_Config(SPI_HandleTypeDef* spi, unsigned char* buffer){ */
void OLED_init(SPI_HandleTypeDef* spi)
{
	OLED_SPIInst = spi;
	
	// Initialisation
	pRST_Clr();
	NopDelay(2000);
	pRST_Set();
	
	NopDelay(20000);
	OLED_writeCMD(OLED_initSequence, sizeof OLED_initSequence);
	NopDelay(20000);
	
}

// Buffer pixel checking and manipulation
 uint8_t OLED_getPixel(uint8_t x, uint8_t y) 
 {
 	uint8_t  ucByteOffset = 0; 
 	uint16_t usiArrayLoc = 0; 
	
		// Determine array location 
 		usiArrayLoc = (y/8)+(x*8); 
		
 		// Determine byte offset */
 		ucByteOffset = y-((uint8_t)(y/8)*8); 
		
 		// Return bit state from buffer 
 		return OLED_Buffer[usiArrayLoc] & (1 << ucByteOffset); 
 } 

 void OLED_setPixel(uint8_t x, uint8_t y) 
 { 
 	uint8_t  ucByteOffset = 0;
 	uint16_t usiArrayLoc = 0;
	
 	if (x<128 && y<64)
 	{	 
 		// Determine array location 
 		usiArrayLoc = (y/8)+(x*8); 
		
 		// Determine byte offset 
 		ucByteOffset = y-((uint8_t)(y/8)*8); 
		
 		// Set pixel in buffer */
 		OLED_Buffer[usiArrayLoc] |= (1 << ucByteOffset); 
 	} 
 } 
 
 void OLED_clearPixel(uint8_t x, uint8_t y) 
 {
 	uint8_t  ucByteOffset = 0; 
 	uint16_t usiArrayLoc = 0; 
	
 	if (x<128 && y<64) 
 	{	 
 		// Determine array location 
 		usiArrayLoc = (y/8)+(x*8); 
		
 		// Determine byte offset */
 		ucByteOffset = y-((uint8_t)(y/8)*8); 
	
 		// Clear pixel in buffer 
 		OLED_Buffer[usiArrayLoc] &= ~(1 << ucByteOffset); 
 	} 
 } 

 void OLED_togglePixel(uint8_t x, uint8_t y) 
 { 
 	uint8_t  ucByteOffset = 0; 
 	uint16_t usiArrayLoc = 0; 
	
 	if (x<128 && y<64) 
 	{	 
 		// Determine array location 
 		usiArrayLoc = (y/8)+(x*8); 
		
 		// Determine byte offset 
 		ucByteOffset = y-((uint8_t)(y/8)*8); 
		
 		// Toggle pixel in buffer */
 		OLED_Buffer[usiArrayLoc] ^= (1 << ucByteOffset); 
 	} 
 } 
