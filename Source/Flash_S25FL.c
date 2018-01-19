#include "Flash_S25FL.h"

// ____ SPI Config 
SPI_HandleTypeDef* FLASH_SPIConfig;

// ____ Test Variables
unsigned char ucTestData, rgTestData[255];

//_____ Byte and String Functions
unsigned char Flash_readByte(unsigned long address)
{
	unsigned char rgAddress[3] = "";
	unsigned char ucData, ucInstruction = INST_READ_EN;
		
	// Build address array
	rgAddress[0] = (address & 0xFF0000) >> 16;
	rgAddress[1] = (address & 0x00FF00) >> 8;
	rgAddress[2] = (address & 0x0000FF) >> 0;
	
	Flash_Select();									// Select device
	Flash_WP_Disable();							// Disable write protect	
	
	__nop();__nop();__nop();
	
	// Send and receive data
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, 1, 100);
	HAL_SPI_Transmit(FLASH_SPIConfig, rgAddress, 			3, 100);
	HAL_SPI_Receive(FLASH_SPIConfig,  &ucData,  	 		1, 100);

	__nop();__nop();__nop();
	
	Flash_WP_Enable();							// Enable write protect
	Flash_Deselect();								// Deselect device
	
	return ucData;									// Return received value
}

void Flash_writeByte(unsigned long address, unsigned char data)
{
	unsigned char rgAddress[3] = "";
	unsigned char ucData = data, ucInstruction = INST_PAGE_PROGRAM;
		
	// Build address array
	rgAddress[0] = (address & 0xFF0000) >> 16;
	rgAddress[1] = (address & 0x00FF00) >> 8;
	rgAddress[2] = (address & 0x0000FF) >> 0;
		
	_Flash_writeEN();								// Write enable sequence

	Flash_Select();									// Select device
	Flash_WP_Disable();							// Disable write protect	
	
	__nop();__nop();__nop();
	
	// Send and receive data
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, 1, 100);
	HAL_SPI_Transmit(FLASH_SPIConfig, rgAddress, 			3, 100);
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucData,  	 		1, 100);

	__nop();__nop();__nop();
	
	Flash_WP_Enable();							// Enable write protect
	Flash_Deselect();								// Deselect device
	
	// Wait for write to finish
	while (Flash_readStatusReg(1) & 0x01){}
	
	// Check that the write enable latch has been cleared
	while (Flash_readStatusReg(1) & 0x02) {_Flash_writeDIS();}
}

void Flash_writeString(unsigned long address, unsigned char *string)
{
	unsigned char rgAddress[3] = "", rgString[255];
	unsigned char ucData, ucInstruction = INST_PAGE_PROGRAM;
	unsigned char ucTest;
	
	strcpy(rgString, string);
		
	// Build address array
	rgAddress[0] = (address & 0xFF0000) >> 16;
	rgAddress[1] = (address & 0x00FF00) >> 8;
	rgAddress[2] = (address & 0x0000FF) >> 0;
		
	_Flash_writeEN();								// Write enable sequence

	Flash_Select();									// Select device
	Flash_WP_Disable();							// Disable write protect	
	
	__nop();__nop();__nop();
	
	// Send and receive data
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, 1, 100);
	HAL_SPI_Transmit(FLASH_SPIConfig, rgAddress, 			3, 100);
	HAL_SPI_Transmit(FLASH_SPIConfig, rgString, 			sizeof rgString, 100);

	__nop();__nop();__nop();
	
	Flash_WP_Enable();							// Enable write protect
	Flash_Deselect();								// Deselect device
	
	// Wait for write to finish
	while (Flash_readStatusReg(1) & 0x01){}
	
	// Check that the write enable latch has been cleared
	while (Flash_readStatusReg(1) & 0x02) {_Flash_writeDIS();}
}

void Flash_readString(unsigned long address, unsigned char *rxBuffer, unsigned char length)
{
	unsigned char rgAddress[3] = "", ucData[255] = "";
	unsigned char ucInstruction = INST_READ_EN;
		
	// Build address array
	rgAddress[0] = (address & 0xFF0000) >> 16;
	rgAddress[1] = (address & 0x00FF00) >> 8;
	rgAddress[2] = (address & 0x0000FF) >> 0;
	
	Flash_Select();									// Select device
	Flash_WP_Disable();							// Disable write protect	
	
	__nop();__nop();__nop();
	
	// Send and receive data
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, 1, 100);
	HAL_SPI_Transmit(FLASH_SPIConfig, rgAddress, 			3, 100);
	HAL_SPI_Receive(FLASH_SPIConfig,  ucData,  	 length, 100);
	
	__nop();__nop();__nop();
	
	Flash_WP_Enable();						// Enable write protect
	Flash_Deselect();							// Deselect device

	strcpy(rxBuffer, ucData);			// Return received value
}

//_____ Service Functions
unsigned char Flash_readStatusReg (unsigned char reg)
{
	unsigned char ucData = 0, ucInstruction;
		
	Flash_Select();										// Select device
	Flash_WP_Disable();								// Disable write protect	

	__nop();__nop();__nop();
	
	// Write enable sequence
	switch(reg)
	{
		case 1: ucInstruction = INST_READ_STATREG_1; break;
		case 2: ucInstruction = INST_READ_STATREG_2; break;
		case 3: ucInstruction = INST_READ_STATREG_3; break;
	}
	
	// Send and receive data
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, sizeof ucInstruction, 1000);
	HAL_SPI_Receive(FLASH_SPIConfig,  &ucData,  	 		sizeof ucData, 				1000);								
	
	__nop();__nop();__nop();
	
	Flash_WP_Enable();						// Enable write protect
	Flash_Deselect();							// Deselect device
	
	return ucData;
}

void Flash_writeStatusReg (unsigned char reg, unsigned char data)
{	
	unsigned char ucData, ucInstruction = INST_WRITE_STATREG;
	
	_Flash_writeEN();
	
	Flash_Select();								// Select device
	Flash_WP_Disable();						// Disable write protect	
	
	__nop();__nop();__nop();
	
	// Send and receive data
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, sizeof ucInstruction, 1000);
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucData,  	 		sizeof ucData, 				1000);
	
	__nop();__nop();__nop();
	
	Flash_WP_Enable();						// Enable write protect
	Flash_Deselect();							// Deselect device
	
	// Wait for write to finish
	while (Flash_readStatusReg(1) & 0x01){}
	
	// Check that the write enable latch has been cleared
	while (Flash_readStatusReg(1) & 0x02) {_Flash_writeDIS();}
}

void Flash_S25FL_init (SPI_HandleTypeDef *spiconfig)
{
	// Copy SPI configuration to local variable
	FLASH_SPIConfig = spiconfig;
		
	Flash_Deselect();		  			// Deslect chip
	Flash_Release();	  				// Disable Hold
	Flash_WP_Enable();		  		// Enable Write Protect
}

void Flash_S25FL_Test(void)
{
	unsigned short usiMemLoc = 0;
	unsigned char ucTestNumber = 1;
	
	switch (ucTestNumber)
	{
		// Byte read
		case 0:
			ucTestData = Flash_readByte(usiMemLoc);
			break;
		
		// Byte read & write
		case 1:
			Flash_writeByte(usiMemLoc, 0xAA);
			ucTestData = Flash_readByte(usiMemLoc);
			break;
		
		// String read & write
		case 2:
			Flash_writeString(usiMemLoc, "Testing Testing 123");
			Flash_readString(usiMemLoc, rgTestData, 20);
			break;
		
		// 4k SubSector Erase
		case 10:
			Flash_SubSectorErase(0);
			break;
		
		// 64k Sector Erase
		case 11:
			Flash_SectorErase(0);
			break;
		
		// Bulk Erase
		case 12:
			Flash_BulkErase();
			break;
	}

	__nop();
}

//_____ Erase Functions 
void Flash_BulkErase (void)
{	
	unsigned char ucInstruction = INST_ERASE_CHIP;
	
	_Flash_writeEN();								// Write enable sequence
	
	Flash_Select();									// Select device
	Flash_WP_Disable();							// Disable write protect	
	
	__nop();__nop();__nop();
	
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, 1, 100);
	
	__nop();__nop();__nop();
	
	Flash_WP_Enable();							// Enable write protect
	Flash_Deselect();								// Deselect device
	
	// Wait for write to finish
	while (Flash_readStatusReg(1) & 0x01){__nop();__nop();__nop();}
	
	// Check that the write enable latch has been cleared
	while (Flash_readStatusReg(1) & 0x02) {_Flash_writeDIS();__nop();}
}

void Flash_SectorErase (unsigned char index)
{	
	unsigned long address 	= (index*0x10000)+1;
	unsigned char rgAddress[3] = "";
	unsigned char ucData, ucInstruction = INST_ERASE_BLOCK;
		
	// Build address array
	rgAddress[0] = (address & 0xFF0000) >> 16;
	rgAddress[1] = (address & 0x00FF00) >> 8;
	rgAddress[2] = (address & 0x0000FF) >> 0;
	
	_Flash_writeEN();
	
	Flash_WP_Disable();						// Disable write protect	
	Flash_Select();								// Select device
	
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, 1, 100);
	HAL_SPI_Transmit(FLASH_SPIConfig, rgAddress, 			3, 100);
	
	Flash_WP_Enable();						// Enable write protect
	Flash_Deselect();							// Deselect device
	
	// Wait for write to finish
	while (Flash_readStatusReg(1) & 0x01){}
	
	// Check that the write enable latch has been cleared
	while (Flash_readStatusReg(1) & 0x02) {_Flash_writeDIS();}
}

void Flash_SubSectorErase (unsigned char index)
{	
	unsigned long address 	= (index*0x1000)+1;
	unsigned char rgAddress[3] = "";
	unsigned char ucData, ucInstruction = INST_ERASE_SECTOR;
		
	// Build address array
	rgAddress[0] = (address & 0xFF0000) >> 16;
	rgAddress[1] = (address & 0x00FF00) >> 8;
	rgAddress[2] = (address & 0x0000FF) >> 0;
		
	_Flash_writeEN();								// Write enable sequence

	Flash_Select();									// Select device
	Flash_WP_Disable();							// Disable write protect	
	
	__nop();__nop();__nop();
	
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, 1, 100);
	HAL_SPI_Transmit(FLASH_SPIConfig, rgAddress, 			3, 100);
	
	__nop();__nop();__nop();
	
	Flash_WP_Enable();						// Enable write protect
	Flash_Deselect();							// Deselect device
	
	// Wait for write to finish
	while (Flash_readStatusReg(1) & 0x01){__nop();__nop();__nop();}
	
	// Check that the write enable latch has been cleared
	while (Flash_readStatusReg(1) & 0x02) {_Flash_writeDIS();__nop();}
}

//_____ Sub Functions
void _Flash_writeEN (void)
{
	unsigned char ucInstruction = INST_WRITE_EN;
	
	Flash_Select();								// Select device
	Flash_WP_Disable();						// Disable write protect	
	
	__nop();__nop();__nop();
	
	// Send Command
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, sizeof ucInstruction, 100);
	
	__nop();__nop();__nop();
	
	Flash_WP_Enable();						// Enable write protect
	Flash_Deselect();							// Deselect device
}

void _Flash_writeDIS (void)
{	
	unsigned char ucInstruction = INST_WRITE_DIS;
	
	Flash_Select();								// Select device
	Flash_WP_Disable();						// Disable write protect	
	
	__nop();__nop();__nop();
	
	// Send Command
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, sizeof ucInstruction, 100);
	
	__nop();__nop();__nop();
	
	Flash_WP_Enable();						// Enable write protect
	Flash_Deselect();							// Deselect device
}
