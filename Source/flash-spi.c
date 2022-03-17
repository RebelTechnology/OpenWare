#include "device.h"
#include "errorhandlers.h"
#include "flash.h"
#include <string.h>

#ifdef USE_SPI_FLASH

#define INST_ERASE_SECTOR			0x20
#define INST_ERASE_BLOCK			0xD8
#define INST_ERASE_HALF_BLOCK			0x52
#define INST_ERASE_CHIP				0xC7

#define ERASE_4KB                               INST_ERASE_SECTOR
#define ERASE_32KB                              INST_ERASE_HALF_BLOCK
#define ERASE_64KB                              INST_ERASE_BLOCK

static uint8_t flash_readStatusReg (uint8_t reg);
static uint32_t flash_readIdentification();
static void flash_writeStatusReg (uint8_t reg, uint8_t data);
static void flash_BulkErase (void);

//_____ External Definitions _______________________________________________________________________
// Pin Mappings
#define flash_Select()				HAL_GPIO_WritePin(FLASH_nCS_GPIO_Port,  FLASH_nCS_Pin, 	GPIO_PIN_RESET)
#define flash_Deselect()			HAL_GPIO_WritePin(FLASH_nCS_GPIO_Port,  FLASH_nCS_Pin, 	GPIO_PIN_SET)	
#define flash_Hold()					HAL_GPIO_WritePin(FLASH_HOLD_GPIO_Port, FLASH_HOLD_Pin, GPIO_PIN_RESET)
#define flash_Release()				HAL_GPIO_WritePin(FLASH_HOLD_GPIO_Port, FLASH_HOLD_Pin, GPIO_PIN_SET)
#define flash_WP_Disable()		HAL_GPIO_WritePin(FLASH_WP_GPIO_Port,  	FLASH_WP_Pin, 	GPIO_PIN_RESET)
#define flash_WP_Enable()			HAL_GPIO_WritePin(FLASH_WP_GPIO_Port,  	FLASH_WP_Pin, 	GPIO_PIN_SET)

// Instruction Commands
#define INST_WRITE_STATREG			0x01
#define INST_PAGE_PROGRAM			0x02
#define INST_READ_EN				0x03
#define INST_FAST_READ_EN			0x0b
#define INST_WRITE_DIS				0x04
#define INST_READ_STATREG_1			0x05
#define INST_WRITE_EN				0x06
#define INST_READ_STATREG_3			0x33
#define INST_READ_STATREG_2			0x35
#define INST_WRITE_EN_VSTATREG			0x50
#define INST_BURSTWRAP_SET			0x77
#define INST_READ_RDID   			0x9F

#ifndef __nop
#define __nop() __asm("NOP")
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

static void _flash_writeEN (void);
static void _flash_writeDIS (void);
static void spi_read_page(uint32_t address, uint8_t* data, uint16_t length);

SPI_HandleTypeDef* FLASH_SPIConfig;
#ifdef DEBUG
uint32_t flash_rdid = 0;
#endif

int flash_read(uint32_t address, uint8_t* data, size_t length){
  /* if(sizeof(((SPI_HandleTypeDef*)NULL)->RxXferSize) == 2){ */
  /* if(sizeof(FLASH_SPIConfig->RxXferSize) == 2){ */
    size_t remain = length;
    while(remain){
      uint16_t len = min(remain, 0x8000); // read 32K at a time because...
      spi_read_page(address, data, len);
      address += len;
      data += len;
      remain -= len;
    }
  /* }else{ */
  /*   spi_read_page(address, data, length); */
  /* } */
  return length;
}

/**
 * HAL_SPI_Transmit and HAL_SPI_Receive only work with 16-bit size parameters.
 */
void spi_read_page(uint32_t address, uint8_t* data, uint16_t length){
  uint8_t cmd[4];
  flash_Select();	
  /* The address can start at any byte location of the memory array. The address is automatically incremented to the next higher address */
  /* in sequential order after each byte of data is shifted out. The entire memory can therefore be read out with one single read */
  /* instruction and address 000000h provided. When the highest address is reached, the address counter will wrap around and roll back */
  /* to 000000h, allowing the read sequence to be continued indefinitely. */

  /* There is also a FAST_READ 0xb instruction which requires dummy cycles after address, 
     default 8 cycles */

  cmd[1] = (address & 0xFF0000) >> 16;
  cmd[2] = (address & 0x00FF00) >> 8;
  cmd[3] = (address & 0x0000FF) >> 0;
  
#if 0
  // READ 1-1-1, 0x03, no dummy cycles, up to 50Mhz
  cmd[0] = INST_READ_EN;
  /* flash_WP_Disable(); // why? */
  // Send and receive data
  HAL_SPI_Transmit(FLASH_SPIConfig, cmd, 4, 100);
  HAL_SPI_Receive(FLASH_SPIConfig,  data, length, 100);
  /* flash_WP_Enable(); // why? */
#else // if this works, turn SPI speed up to max 108MHz
  // FAST_READ 1-1-1, 0x0b, up to 108Mhz
  cmd[0] = INST_FAST_READ_EN;
  HAL_SPI_Transmit(FLASH_SPIConfig, cmd, 4, 100);
  HAL_SPI_Receive(FLASH_SPIConfig, data, 1, 100); // 8 dummy cycles
  HAL_SPI_Receive(FLASH_SPIConfig, data, length, 100);
#endif
  
  flash_Deselect();
}

// address must be on a 256-byte boundary
/* "The Page Program command accepts from 1-byte up to 256 consecutive bytes of data (page) to be programmed in one operation. Programming means that bits can either be left at 1, or programmed from 1 to 0. Changing bits from 0 to 1 requires an erase operation." */  
int flash_write(uint32_t address, const uint8_t* data, size_t length){
  uint8_t rgAddress[3];

  // PP Page Program 1-1-1, 0x02, up to 108Mhz
  uint8_t ucInstruction = INST_PAGE_PROGRAM;
  size_t ret = length;
  while(length){

    _flash_writeEN();
    flash_WP_Disable();
	
    // wait for write enable latch WEL
    while (!(flash_readStatusReg(INST_READ_STATREG_1) & 0x02)){}

    size_t len = length > 256 ? 256 : length;

    // Build address array
    rgAddress[0] = (address & 0xFF0000) >> 16;
    rgAddress[1] = (address & 0x00FF00) >> 8;
    rgAddress[2] = (address & 0x0000FF) >> 0;

    flash_Select();

    // Send and receive data
    HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, 1, 100);
    HAL_SPI_Transmit(FLASH_SPIConfig, rgAddress, 3, 100);
    HAL_SPI_Transmit(FLASH_SPIConfig, (uint8_t*)data, len, 100);

    flash_Deselect();

    length -= len;
    data += len;
    address += len;

    // Wait for write to finish
    while (flash_readStatusReg(INST_READ_STATREG_1) & 0x01){}

    /* While not obvious from the data sheet, multiple page programming only works if enable command is also sent. */
    /* "After a programming command is issued, the programming operation status can be checked using the  */
    /*    Read Status Register 1 command. The WIP bit (SR1V[0]) will indicate  when the programming operation  */
    /*    is completed. The P_ERR bit (SR2V[5]) will indicate if an error occurs in the programming operation */
    /*    that prevents successful completion of programming" */
											   	
    flash_WP_Enable();
		
    // Check that the write enable latch has been cleared
    while (flash_readStatusReg(INST_READ_STATREG_1) & 0x02) {_flash_writeDIS();}
  }
  return ret;
}

uint32_t flash_readIdentification(){
  uint32_t ucData = 0;
  uint8_t ucInstruction = INST_READ_RDID;
  flash_Select();
  flash_WP_Disable();
	
  // Send and receive data
  HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, sizeof ucInstruction, 1000);
  HAL_SPI_Receive(FLASH_SPIConfig,  (uint8_t*)&ucData, sizeof ucData, 1000);
	
  flash_WP_Enable();
  flash_Deselect();
	
  return ucData;
}

//_____ Service Functions
uint8_t flash_readStatusReg (uint8_t ucInstruction)
{
  uint8_t ucData = 0;
		
  flash_Select();
  flash_WP_Disable();
	
  // Send and receive data
  HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, sizeof ucInstruction, 1000);
  HAL_SPI_Receive(FLASH_SPIConfig,  &ucData, sizeof ucData, 1000);
	
  flash_WP_Enable();
  flash_Deselect();
	
  return ucData;
}

void flash_writeStatusReg (uint8_t reg, uint8_t data)
{	
	uint8_t ucInstruction = INST_WRITE_STATREG;
	
	_flash_writeEN();
	
	flash_Select();								// Select device
	flash_WP_Disable();						// Disable write protect	
	
	// Send and receive data
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, sizeof ucInstruction, 1000);
	HAL_SPI_Transmit(FLASH_SPIConfig, &data, sizeof data, 1000);
	
	flash_WP_Enable();						// Enable write protect
	flash_Deselect();							// Deselect device
	
	// Wait for write to finish
	while (flash_readStatusReg(INST_READ_STATREG_1) & 0x01){}
	
	// Check that the write enable latch has been cleared
	while (flash_readStatusReg(INST_READ_STATREG_1) & 0x02) {_flash_writeDIS();}
}

void flash_init(void* handle) {
  // Copy SPI configuration to local variable
	FLASH_SPIConfig = (SPI_HandleTypeDef*)handle;
		
	flash_Deselect();		  			// Deselect chip
	flash_Release();	  				// Disable Hold
	flash_WP_Enable();		  		// Enable Write Protect

#ifdef DEBUG
	flash_rdid = flash_readIdentification();
#endif
}

//_____ Erase Functions 
/* entire chip erase */
void flash_BulkErase (void)
{	
	uint8_t ucInstruction = INST_ERASE_CHIP;
	
	_flash_writeEN();								// Write enable sequence
	
	flash_Select();									// Select device
	flash_WP_Disable();							// Disable write protect	
	
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, 1, 100);
	
	flash_WP_Enable();							// Enable write protect
	flash_Deselect();								// Deselect device
	
	// Wait for write to finish
	while (flash_readStatusReg(INST_READ_STATREG_1) & 0x01){__nop();__nop();__nop();}
	
	// Check that the write enable latch has been cleared
	while (flash_readStatusReg(INST_READ_STATREG_1) & 0x02) {_flash_writeDIS();__nop();}
}

/* individual 4 KB sector erase, 32 KB half block sector, 64 KB block sector erase */		
int flash_erase(uint32_t address, size_t size){
  uint8_t cmd;
  switch(size){
  case 4*1024:
    cmd = ERASE_4KB;
    break;
  case 32*1024:
    cmd = ERASE_32KB;
    break;
  case 64*1024:
    cmd = ERASE_64KB;
    break;
  default:
    return -1;
  }

  uint8_t data[4];
  data[0] = cmd;
  data[1] =  (address & 0xFF0000) >> 16;
  data[2] =  (address & 0x00FF00) >> 8;
  data[3] =  (address & 0x0000FF) >> 0;
	
  _flash_writeEN();
  flash_Select();
  flash_WP_Disable();
	
  HAL_SPI_Transmit(FLASH_SPIConfig, data, sizeof(data), 100);
	
  flash_WP_Enable();
  flash_Deselect();
	
  // Wait for write to finish
  while (flash_readStatusReg(INST_READ_STATREG_1) & 0x01){__nop();__nop();__nop();}
	
  // Check that the write enable latch has been cleared
  while (flash_readStatusReg(INST_READ_STATREG_1) & 0x02) {_flash_writeDIS();__nop();}

  return size;
}

//_____ Sub Functions
void _flash_writeEN (void)
{
	uint8_t ucInstruction = INST_WRITE_EN;
	
	flash_Select();								// Select device
	flash_WP_Disable();						// Disable write protect	
	
	// Send Command
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, sizeof ucInstruction, 100);
	
	flash_WP_Enable();						// Enable write protect
	flash_Deselect();							// Deselect device
}

void _flash_writeDIS (void)
{	
	uint8_t ucInstruction = INST_WRITE_DIS;
	
	flash_Select();								// Select device
	flash_WP_Disable();						// Disable write protect	
	
	// Send Command
	HAL_SPI_Transmit(FLASH_SPIConfig, &ucInstruction, sizeof ucInstruction, 100);
	
	flash_WP_Enable();						// Enable write protect
	flash_Deselect();							// Deselect device
}

#endif
