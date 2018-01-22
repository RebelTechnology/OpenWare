// _____ Includes ______________________________________________________________________
#include "oled.h"
#include "device.h"
#include <string.h>

static void NopDelay(uint32_t nops){
  while (nops--)
    __asm("NOP");
}
#define delay(x) NopDelay(x*1000)
/* #define delay(x) HAL_Delay(x) */
/* #define delay(x) osDelay(x) */

// _____ Defines _______________________________________________________________________
#define pRST_Set()	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET)
#define pDC_Set()		HAL_GPIO_WritePin(OLED_DC_GPIO_Port, 	OLED_DC_Pin, 	GPIO_PIN_SET)
#define pCS_Set()		HAL_GPIO_WritePin(OLED_CS_GPIO_Port, 	OLED_CS_Pin, 	GPIO_PIN_SET)

#define pRST_Clr()	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET)
#define pDC_Clr()		HAL_GPIO_WritePin(OLED_DC_GPIO_Port, 	OLED_DC_Pin, 	GPIO_PIN_RESET)
#define pCS_Clr()		HAL_GPIO_WritePin(OLED_CS_GPIO_Port, 	OLED_CS_Pin, 	GPIO_PIN_RESET)

#define OLED_DAT	1
#define OLED_CMD	0

// _____ Prototypes ____________________________________________________________________
static void OLED_writeCMD(const uint8_t* data, uint16_t length);

// _____ Variables _____________________________________________________________________
static const uint8_t OLED_initSequence[] = 
{
	0xfd, 0x12, 	// Command unlock
	0xae, 		// Display off
	0xd5, 0xa0, 	// Clock divide ratio / Oscillator Frequency
	0xa8, 0x3f, 	// Multiplex ratio 64
	0xc8, 		// Scan direction: c0: scan dir normal, c8: reverse
	0xd3, 0x00, 	// Display offset
	0x40, 		// Start line
	0xa1, 		// Segment re-map: a0: col0 -> SEG0, a1: col127 -> SEG0
	/* 0xa0,        // writes right-to-left */
	0xda, 0x12, 	// COM pins
	0x81, 0xcf, 	// Current control
	0xd9, 0x22, 	// Pre-charge period
	0xdb, 0x34, 	// VCOMH deselect level
	0xa4, 		// Entire display on/off
	0xa6, 		// Normal / inverse display
	0x20, 0x01,     // Vertical addressing mode
	0x21, 0x00, 0x7f, // Set column address
	/* 0x22, 0x00, 0x07, // Set page address */
	0xaf, 		// Display on
	0xa6,		// Set Normal/Inverse Display
};

/* static unsigned char* OLED_Buffer; */
static SPI_HandleTypeDef* OLED_SPIInst;
	
// _____ Functions _____________________________________________________________________
static void OLED_writeCMD(const uint8_t* data, uint16_t length)
{
	pCS_Clr();	// CS low
	pDC_Clr();	// DC low		
	
	// Send Data	
	HAL_SPI_Transmit(OLED_SPIInst, (uint8_t*)data, length, 1000);
	
	pCS_Set();	// CS high
}

#ifdef OLED_DMA
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi){
  assert_param(0);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
  if(__HAL_DMA_GET_FLAG(OLED_SPIInst,  __HAL_DMA_GET_TC_FLAG_INDEX(OLED_SPIInst))){
    pCS_Set();	// CS high
  }
}
#endif

/* void OLED_writeDAT(const uint8_t* data, uint16_t length) */
void oled_write(const uint8_t* data, uint16_t length)
{
	pCS_Clr();	// CS low
	pDC_Set();	// DC high

#ifdef OLED_DMA
	// Send Data
	HAL_SPI_Transmit_DMA(OLED_SPIInst, (uint8_t*)data, length);
#else
	HAL_SPI_Transmit(OLED_SPIInst, (uint8_t*)data, length, 1000);
	pCS_Set();	// CS high
#endif
}

/* void OLED_Refresh(void) */
/* { */
/* 	// Write entire buffer to OLED */
/* 	OLED_writeDAT(OLED_Buffer, 1024); */
/* } */

/* void OLED_ClearScreen(void) */
/* { */
/* 	// Clear contents of OLED buffer */
/* 	memset(OLED_Buffer, 0, 1024); */
/* } */

// Configuration
/* void OLED_Config(SPI_HandleTypeDef* spi, unsigned char* buffer){ */
void oled_init(SPI_HandleTypeDef* spi){
	OLED_SPIInst = spi;
	/* OLED_Buffer = buffer; */

#if 0 // should be done by Cube in main.c
	GPIO_InitTypeDef GPIO_InitStruct;
	// Configure RST and DC Pins
	GPIO_InitStruct.Pin   = OLED_RST_Pin | OLED_DC_Pin;					
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	// Configure CS Pin
	GPIO_InitStruct.Pin   = OLED_CS_Pin;					
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(OLED_CS_GPIO_Port, &GPIO_InitStruct);
	
	// Set up SPI	
	OLED_SPIInst->Instance = SPI2;
	OLED_SPIInst->Init.Mode = SPI_MODE_MASTER;
	OLED_SPIInst->Init.Direction = SPI_DIRECTION_2LINES;
	OLED_SPIInst->Init.DataSize = SPI_DATASIZE_8BIT;
	OLED_SPIInst->Init.CLKPolarity = SPI_POLARITY_LOW;
	OLED_SPIInst->Init.CLKPhase = SPI_PHASE_1EDGE;
	OLED_SPIInst->Init.NSS = SPI_NSS_SOFT;
#ifndef OWL_MAGUS
	OLED_SPIInst->Init.NSSPMode = SPI_NSS_PULSE_DISABLED;
#endif

	OLED_SPIInst->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	OLED_SPIInst->Init.FirstBit = SPI_FIRSTBIT_MSB;
	OLED_SPIInst->Init.TIMode = SPI_TIMODE_DISABLED;
	OLED_SPIInst->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
	OLED_SPIInst->Init.CRCPolynomial = 7;
#ifndef OWL_MAGUS	
	OLED_SPIInst->Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
#endif
	HAL_SPI_Init(OLED_SPIInst);
#endif	
	// Initialisation
	pRST_Clr();
	delay(2);
	pRST_Set();
	delay(5);
	OLED_writeCMD(OLED_initSequence, sizeof OLED_initSequence);
	delay(100);
}

// Buffer pixel checking and manipulation
/* uint8_t OLED_getPixel(uint8_t x, uint8_t y) */
/* { */
/* 	uint8_t  ucByteOffset = 0; */
/* 	uint16_t usiArrayLoc = 0; */
	
/* 		// Determine array location */
/* 		usiArrayLoc = (y/8)+(x*8); */
		
/* 		// Determine byte offset */
/* 		ucByteOffset = y-((uint8_t)(y/8)*8); */
		
/* 		// Return bit state from buffer */
/* 		return OLED_Buffer[usiArrayLoc] & (1 << ucByteOffset); */
/* } */

/* void OLED_setPixel(uint8_t x, uint8_t y) */
/* { */
/* 	uint8_t  ucByteOffset = 0; */
/* 	uint16_t usiArrayLoc = 0; */
	
/* 	if (x<128 && y<64) */
/* 	{	 */
/* 		// Determine array location */
/* 		usiArrayLoc = (y/8)+(x*8); */
		
/* 		// Determine byte offset */
/* 		ucByteOffset = y-((uint8_t)(y/8)*8); */
		
/* 		// Set pixel in buffer */
/* 		OLED_Buffer[usiArrayLoc] |= (1 << ucByteOffset); */
/* 	} */
/* } */

/* void OLED_clearPixel(uint8_t x, uint8_t y) */
/* { */
/* 	uint8_t  ucByteOffset = 0; */
/* 	uint16_t usiArrayLoc = 0; */
	
/* 	if (x<128 && y<64) */
/* 	{	 */
/* 		// Determine array location */
/* 		usiArrayLoc = (y/8)+(x*8); */
		
/* 		// Determine byte offset */
/* 		ucByteOffset = y-((uint8_t)(y/8)*8); */
	
/* 		// Clear pixel in buffer */
/* 		OLED_Buffer[usiArrayLoc] &= ~(1 << ucByteOffset); */
/* 	} */
/* } */

/* void OLED_togglePixel(uint8_t x, uint8_t y) */
/* { */
/* 	uint8_t  ucByteOffset = 0; */
/* 	uint16_t usiArrayLoc = 0; */
	
/* 	if (x<128 && y<64) */
/* 	{	 */
/* 		// Determine array location */
/* 		usiArrayLoc = (y/8)+(x*8); */
		
/* 		// Determine byte offset */
/* 		ucByteOffset = y-((uint8_t)(y/8)*8); */
		
/* 		// Toggle pixel in buffer */
/* 		OLED_Buffer[usiArrayLoc] ^= (1 << ucByteOffset); */
/* 	} */
/* } */

/* static const uint8_t u8x8_d_ssd1309_128x64_flip0_seq[] = { */
/*   U8X8_START_TRANSFER(),             	/\* enable chip, delay is part of the transfer start *\/ */
/*   U8X8_C(0x0a1),				/\* segment remap a0/a1*\/ */
/*   U8X8_C(0x0c8),				/\* c0: scan dir normal, c8: reverse *\/ */
/*   U8X8_END_TRANSFER(),             	/\* disable chip *\/ */
/*   U8X8_END()             			/\* end of sequence *\/ */
/* }; */

/* static const uint8_t u8x8_d_ssd1309_128x64_flip1_seq[] = { */
/*   U8X8_START_TRANSFER(),             	/\* enable chip, delay is part of the transfer start *\/ */
/*   U8X8_C(0x0a0),				/\* segment remap a0/a1*\/ */
/*   U8X8_C(0x0c0),				/\* c0: scan dir normal, c8: reverse *\/ */
/*   U8X8_END_TRANSFER(),             	/\* disable chip *\/ */
/*   U8X8_END()             			/\* end of sequence *\/ */
/* }; */
