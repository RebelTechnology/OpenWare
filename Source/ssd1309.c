// _____ Includes ______________________________________________________________________
#include <string.h>
#include "oled.h"
#include "device.h"
#include "message.h"
#include "errorhandlers.h"

/* static void NopDelay(uint32_t nops){ */
/*   while (nops--) */
/*     __asm("NOP"); */
/* } */
/* #define delay(x) NopDelay(x*1000) */
#define delay(x) HAL_Delay(x)
/* #define delay(x) osDelay(x) */

// _____ Defines _______________________________________________________________________
#define pRST_Set()	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET)
#define pDC_Set()		HAL_GPIO_WritePin(OLED_DC_GPIO_Port, 	OLED_DC_Pin, 	GPIO_PIN_SET)
#define pCS_Set()		HAL_GPIO_WritePin(OLED_CS_GPIO_Port, 	OLED_CS_Pin, 	GPIO_PIN_SET)

#define pRST_Clr()	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET)
#define pDC_Clr()		HAL_GPIO_WritePin(OLED_DC_GPIO_Port, 	OLED_DC_Pin, 	GPIO_PIN_RESET)
#define pCS_Clr()		HAL_GPIO_WritePin(OLED_CS_GPIO_Port, 	OLED_CS_Pin, 	GPIO_PIN_RESET)

// _____ Prototypes ____________________________________________________________________
static void OLED_writeCMD(const uint8_t* data, uint16_t length);

// _____ Variables _____________________________________________________________________
#define SCREEN_OFFSET 0
static const uint8_t OLED_initSequence[] =
  {
#ifdef SSD1306
    0xAE, // Display off
    0xD5, // Set display clock divider
    0x80,
    0xA8, // Set multiplex
    0x3F,
    0xD3, // Set display offset
    SCREEN_OFFSET,
    0x40, // Set start line to zero
    0x8D, // Set charge pump
    0x14,
    0x20, 0x01, // Vertical addressing mode
#ifdef OLED_UPSIDE_DOWN
    0xa0,
    0xc1,
#else
    0xa1,
    0xc8,
#endif
    0xDA,       // Set Comm pins
    0x12,
    0x81, // Set contrast
    0xCF,
    0xd9, // Set precharge
    0xF1,
    0xDB, // Set Vcom detect
    0x30,//40,
    0xA4, // Allow display resume
    0xA6, // Set normal display
    0xAF  // Display On
#else
	0xfd, 0x12, 	// Command unlock
	0xae, 		// Display off
	0xd5, 0xa0, 	// Clock divide ratio / Oscillator Frequency
	0xa8, 0x3f, 	// Multiplex ratio 64
			// Scan direction: c0: scan dir normal, c8: reverse
#ifdef OLED_UPSIDE_DOWN
	0xa0,
	0xc1,
#else
	0xa1,
	0xc8,
#endif
	0xd3, 0x00, 	// Display offset
	0x40, 		// Start line
 			// Segment re-map: a0: col0 -> SEG0, a1: col127 -> SEG0	
	0xda, 0x12, 	// COM pins
	0x81, 0xcf, 	// Contrast control: 0 to 0xff. Current increases with contrast.
	0xd9, 0x22, 	// Pre-charge period
	0xdb, 0x34, 	// VCOMH deselect level
	/* 0xa4, 		// Entire display on/off : A4 / A5 */
	0xa6, 		// Normal / inverse display : A6 / A7
	0x20, 0x01,     // Vertical addressing mode
	0x21, 0x00, 0x7f, // Set column address
	0x22, 0x00, 0x07, // Set page address
	0xaf, 		// Display on
	/* 0xa6,		// Set Normal/Inverse Display */
#endif
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
	delay(1);
}

#ifdef OLED_DMA
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi){
  if(hspi->ErrorCode == HAL_SPI_ERROR_OVR){
    __HAL_SPI_CLEAR_OVRFLAG(hspi);
    debugMessage("SPI OVR");
  }else{
    error(RUNTIME_ERROR, "SPI Error");
  }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
  /* if(__HAL_DMA_GET_FLAG(OLED_SPIInst,  __HAL_DMA_GET_TC_FLAG_INDEX(OLED_SPIInst))){ */
  /*   pCS_Set();	// CS high */
  /* } */
  pCS_Set();	// CS high
}
#endif

void oled_write(const uint8_t* data, uint16_t length)
{
#ifdef OLED_DMA
  while(OLED_SPIInst->State != HAL_SPI_STATE_READY); // wait
  pCS_Clr();	// CS low
  pDC_Set();	// DC high
  HAL_SPI_Transmit_DMA(OLED_SPIInst, (uint8_t*)data, length);
#else
  pCS_Clr();	// CS low
  pDC_Set();	// DC high
  HAL_SPI_Transmit(OLED_SPIInst, (uint8_t*)data, length, 1000);
  pCS_Set();	// CS high
#endif
}

// Configuration
void oled_init(SPI_HandleTypeDef* spi){
	OLED_SPIInst = spi;
	// Initialisation
	pRST_Clr();
	delay(2);
	pRST_Set();
	delay(5);
	OLED_writeCMD(OLED_initSequence, sizeof OLED_initSequence);
	delay(10);
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
