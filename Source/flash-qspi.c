#include "device.h"
#include "flash.h"
#include "main.h"

#ifdef USE_QSPI_FLASH

// =======================
// SPI Flash Commands info
// =======================
#define QSPI_WRR_CMD                 (0x01) // Single Register Write
#define QSPI_PP_CMD                  (0x02) // Page Program
#define QSPI_READ_CMD                (0x03)
#define QSPI_WRDI_CMD                (0x04)
#define QSPI_RDSR_CMD                (0x05)
#define QSPI_WREN_CMD                (0x06)
#define QSPI_RDSR2_CMD               (0x07)  // Read Status Register-2
#define QSPI_FAST_READ_CMD           (0x0B)
#define QSPI_FAST_READ_4B_CMD        (0x0C)
#define QSPI_PP_4B_CMD               (0x12)
#define QSPI_READ_4B_CMD             (0x13)
#define QSPI_RDCR2_CMD               (0x15) // Read configuration register-2
#define QSPI_IRPRD_CMD               (0x2B) // IRP register Read
#define QSPI_IRPP_CMD                (0x2F) // IRP register program
#define QSPI_CLSR_CMD                (0x30)
#define QSPI_QPP_CMD                 (0x32)
#define QSPI_RDCR3_CMD               (0x33) // Read Configuration Register-3
#define QSPI_QPP_4B_CMD              (0x34)
#define QSPI_RCR_CMD                 (0x35)
#define QSPI_IBL_CMD                 (0x36) // IBL lock
#define QSPI_QPIEN_CMD               (0x38) // Enter QPI
#define QSPI_IBUL_CMD                (0x39) // IBL unlock
#define QSPI_DOR_CMD                 (0x3B) // Dual output read
#define QSPI_4DOR_CMD                (0x3C) // Dual output read 4 Byte address 
#define QSPI_IBLRD_CMD               (0x3D) // IBL read
#define QSPI_DLPRD_CMD               (0x41) // Read Data Learning Pattern
#define QSPI_SECRP_CMD               (0x42) // security region program
#define QSPI_PNVDLR_CMD              (0x43) // Program NV Data Learning Register
#define QSPI_SECRE_CMD               (0x44) // security region erase
#define QSPI_SECRR_CMD               (0x48) // security region read
#define QSPI_WVDLR_CMD               (0x4A) // Write Volatile Data Learning Register
#define QSPI_RUID_CMD                (0x4B)
#define QSPI_WRENV_CMD               (0x50) // Write enable for Volatile status and configuration registers
#define QSPI_HBE_CMD                 (0x52) // Half Block Erase
#define QSPI_4HBE_CMD                (0x53) // Half Block Erase 4B address
#define QSPI_READ_SFDP_CMD           (0x5A) // Read Serial Flash Discoverable Parameter Register
#define QSPI_CE_CMD                  (0x60) // chip erase
#define QSPI_RDAR_CMD                (0x65) // Read Any Register
#define QSPI_RSTEN_CMD               (0x66) // Software Reset Enable
#define QSPI_QOR_CMD                 (0x6B) // Quad Output Read
#define QSPI_4QOR_CMD                (0x6C) // Quad Output Read 4B address
#define QSPI_WRAR_CMD                (0x71) // Write Any Register
#define QSPI_SETBURSTWRAP_CMD        (0x77) // Set Burst with Wrap
#define QSPI_GBL_CMD                 (0x7E) // Global IBL lock
//#define QSPI_CLSR2_CMD               (0x82)  //Clear Status Register 1 (alternate instruction) - Erase/Prog. Fail Reset
#define QSPI_RDID_90_CMD             (0x90)
#define QSPI_GBUL_CMD                (0x98) // Global IBL unlock 
#define QSPI_RESET_CMD               (0x99) // Software Reset
#define QSPI_RDID_9F_CMD             (0x9F)
#define QSPI_PRL_CMD                 (0xA6) // Protection Register lock
#define QSPI_PRRD_CMD                (0xA7) // Protection Register read
#define QSPI_RDID_AB_CMD             (0xAB)
#define QSPI_RES_CMD                 (0xAB)
#define QSPI_RDQID_CMD               (0xAF) //Read Quad ID
#define QSPI_EPS_CMD                 (0x75) // for FLL
#define QSPI_EPR_CMD                 (0x7A) // for FLL
#define QSPI_4BEN_CMD                (0xB7) // Enter 4-bytes Address Mode
#define QSPI_DP_CMD                  (0xB9)
#define QSPI_DIOR_CMD                (0xBB) // Dual IO read
#define QSPI_4DIOR_CMD               (0xBC) // Dual IO read 4B address
#define QSPI_CE1_CMD                 (0xC7) // Chip erase
#define QSPI_SE_CMD                  (0x20) // Sector Erase
#define QSPI_SE_4B_CMD               (0x21)
#define QSPI_BLOCK_ERASE_CMD         (0xD8) // Block erase
#define QSPI_4BE_CMD                 (0xDC) // Block erase 4B address
#define QSPI_4IBLRD_CMD              (0xE0) // IBL read 4B address
#define QSPI_4IBL_CMD                (0xE1) // IBL lock 4B address
#define QSPI_4IBUL_CMD               (0xE2) // IBL unlock 4B address
#define QSPI_4SPRP_CMD               (0xE3) // Set Point Region Protection 4B address
#define QSPI_PASSRD_CMD              (0xE7) // Password Read
#define QSPI_PASSP_CMD               (0xE8) // Password Program
#define QSPI_4BEX_CMD                (0xE9) // Exit 4 Byte Address mode
#define QSPI_PASSU_CMD               (0xEA) // Password Unlock
#define QSPI_QIOR_CMD                (0xEB) // Quad IO read
#define QSPI_4QIOR_CMD               (0xEC) // Quad IO read 4B address
#define QSPI_DDRQIOR_CMD             (0xED) // DDR Quad IO read
#define QSPI_4DDRQIOR_CMD            (0xEE) // DDR Quad IO read 4B address
#define QSPI_QPIEX_CMD               (0xF5) // Exit QPI
#define QSPI_SPRP_CMD                (0xFB) // Set Pointer Region Protection
#define QSPI_MBR_CMD                 (0xFF) // Mode Bit Reset

#define QSPI_SR_WIP                  (0x01) // Write In Progress
#define QSPI_SR_WEL                  (0x02) // Write Enable Latch
#define QSPI_SR_BLOCKPR              (0x5C) // Block protected against program and erase operations
#define QSPI_SR_PRBOTTOM             (0x20) // Protected memory area defined by BLOCKPR starts from top or bottom
#define QSPI_SR_QUADEN               (0x40) // Quad IO mode enabled if =1
#define QSPI_SR_SRWREN               (0x80) // Status register write enable/disable
  
#define QSPI_READ_MODE               -122
#define QSPI_TIMEOUT                 HAL_QPSI_TIMEOUT_DEFAULT_VALUE

#define QSPI_ERASE_BLOCK_SIZE        (64*1024) // 64k block sector
#define QSPI_FLASH_PAGE_SIZE         256 // Program page size 256 bytes

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

static int flash_read_block(int mode, uint32_t address, uint8_t* data, size_t size);
static int flash_write_block(uint32_t address, const uint8_t* data, size_t size);
static int qspi_erase_block(uint32_t address, size_t size);
static int qspi_write_enable();
static int qspi_quad_mode(uint8_t cr1, uint8_t cr2);
static int qspi_qpi_mode(bool qpi);

static volatile int qspi_mapped_mode = 0;
static QSPI_HandleTypeDef* qspi_handle;

/**
 * Write In Progress (WIP) SR1V[0]: Indicates whether the device is performing a program, write, erase operation, or any other operation, during which a new operation command will be ignored. 
 * When the bit is set to a “1” the device is busy performing an operation. While WIP is “1”, only Read Status (RDSR1 or RDSR2), Read Any Register (RDAR), Erase / Program Suspend (EPS), Clear Status Register (CLSR), and Software Reset (RSTEN 66h followed by RST 99h) commands are accepted. 
 * EPS command will only be accepted if memory array erase or program operations are in progress. 
 * The status register E_ERR and P_ERR bits are updated while WIP =1. 
 * When P_ERR or E_ERR bits are set to one, the WIP bit will remain set to one indicating the device remains busy and unable to receive new operation commands. 
 * A Clear Status Register (CLSR) command must be received to return the device to standby mode. 
 * When the WIP bit is cleared to 0 no operation is in progress. This is a read-only bit. 
 */
static int qspi_wait(uint32_t match, uint32_t mask){
  QSPI_CommandTypeDef     cmd;
  QSPI_AutoPollingTypeDef cfg;
  cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction       = QSPI_RDSR_CMD; // Read Status Register
  cmd.AddressMode       = QSPI_ADDRESS_NONE;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode          = QSPI_DATA_1_LINE;
  cmd.DummyCycles       = 0;
  cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
  cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  cfg.Match             = match;
  cfg.Mask              = mask;
  cfg.MatchMode         = QSPI_MATCH_MODE_AND;
  cfg.StatusBytesSize   = 1;
  cfg.Interval          = 0x10;
  cfg.AutomaticStop     = QSPI_AUTOMATIC_STOP_ENABLE;
  if(HAL_QSPI_AutoPolling(qspi_handle, &cmd, &cfg, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  return 0;
}

/**
 * Write Enable Latch (WEL) SR1V[1]: The WEL bit must be set to 1 to enable program, write, or erase operations as a means to provide protection against inadvertent changes to memory or register values. 
 * The Write Enable (WREN) command execution sets the Write Enable Latch to a “1” to allow any program, erase, or write commands to execute afterwards. 
 * The Write Disable (WRDI) command can be used to set the Write Enable Latch to a “0” to prevent all program, erase, and write commands from execution. 
 * The WEL bit is cleared to 0 at the end of any successful program, write, or erase operation. 
 * Following a failed operation the WEL bit may remain set and should be cleared with a CLSR command. 
 * After a power down / power up sequence, hardware reset, or software reset, the Write Enable Latch is set to a WEL_D. 
 * The WRR or WRAR command does not affect this bit.
*/
static int qspi_write_enable() {    
  QSPI_CommandTypeDef cmd;
  /* Enable write operations */
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction = QSPI_WREN_CMD; // Write Enable
  cmd.AddressMode = QSPI_ADDRESS_NONE;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode = QSPI_DATA_NONE;
  cmd.DummyCycles = 0;
  cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
  cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
  if(HAL_QSPI_Command(qspi_handle, &cmd, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  return qspi_wait(QSPI_SR_WEL, QSPI_SR_WEL);
}

static void qspi_abort() {
  // prevents timeout due to previous operations in some cases
  // https://community.st.com/s/question/0D50X00009XkXMHSA3/qspi-flag-qspiflagbusy-sometimes-stays-set
  if((FlagStatus)(__HAL_QSPI_GET_FLAG(qspi_handle, QSPI_FLAG_BUSY)) == SET){
    qspi_handle->State = HAL_QSPI_STATE_BUSY;
    HAL_QSPI_Abort(qspi_handle);
  }
}

static int qspi_write_page(uint32_t address, void* data, size_t size){
  // write in 1-1-1 mode
  QSPI_CommandTypeDef cmd;
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction = QSPI_PP_CMD; // Page Program
  cmd.AddressMode = QSPI_ADDRESS_1_LINE;
  cmd.AddressSize = QSPI_ADDRESS_24_BITS;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode = QSPI_DATA_1_LINE;
  cmd.DummyCycles = 0;
  cmd.NbData = size;
  cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
  cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
  cmd.Address = address;
  if(qspi_write_enable() != 0)
    return -1;
  if(HAL_QSPI_Command(qspi_handle, &cmd, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  if(HAL_QSPI_Transmit(qspi_handle, (uint8_t *)data, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  return qspi_wait(0, QSPI_SR_WIP);
}

int qspi_erase_block(uint32_t address, size_t size){
  QSPI_CommandTypeDef cmd;
  cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction       = QSPI_BLOCK_ERASE_CMD;
  cmd.AddressMode       = QSPI_ADDRESS_1_LINE;
  cmd.AddressSize       = QSPI_ADDRESS_24_BITS;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode          = QSPI_DATA_NONE;
  cmd.DummyCycles       = 0;
  cmd.NbData            = 1;
  cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
  cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  int remain = size;
  while(remain > 0){
    cmd.Address         = address;
    if(qspi_write_enable() != 0)
      return -1;
    if(HAL_QSPI_Command(qspi_handle, &cmd, QSPI_TIMEOUT) != HAL_OK)
      return -1;
    address += QSPI_ERASE_BLOCK_SIZE;
    remain -= QSPI_ERASE_BLOCK_SIZE;
    qspi_wait(0, QSPI_SR_WIP);
  }
  return 0;
}

/*
  Single bit wide commands start with an instruction and may provide an address 
  or data, all sent only on the SI signal. Data may be sent back to the host 
  serially on the SO signal. This is referenced as a 1-1-1 command protocol for 
  single bit width instruction, single bit width address and modifier, single bit 
  data.
 */
void qspi_enter_indirect_mode(){
  if(qspi_mapped_mode == 1){
    qspi_mapped_mode = 0;
#if 1
    // clear busy bit to deactivate memory mapped mode
    /* The memory-mapped mode can be deactivated by changing the FMODE bits in the QUADSPI_CCR register when BUSY is cleared. In Memory-mapped mode, BUSY goes high as soon as the first memory-mapped access occurs. Because of the prefetch operations, BUSY does not fall until there is a timeout, there is an abort, or the peripheral is disabled.
       https://community.st.com/s/question/0D50X00009XkaJuSAJ/stm32f7-qspi-exit-memory-mapped-mode
     */
    qspi_abort();
#else
    // reinitialise driver
    HAL_QSPI_DeInit(qspi_handle);
    qspi_abort();
    if(HAL_QSPI_Init(qspi_handle) != HAL_OK)
      Error_Handler();
#endif
  }
}

void qspi_enter_mapped_mode(){
  if(qspi_mapped_mode == 0){
    flash_status();
    flash_memory_map(-122);
    qspi_mapped_mode = 1;
  }
}

int flash_write_block(uint32_t address, const uint8_t *data, size_t size){
  int ret = 0;
  const uint32_t end = address + size;
  uint8_t* src = (uint8_t*)data;
  while(ret == 0 && address < end){
    size_t len = min(size, QSPI_FLASH_PAGE_SIZE);
    ret = qspi_write_page(address, src, len);
    address += len;
    src += len;
    size -= len;
  }
  return ret;
}

#if 1
int flash_write(uint32_t address, const uint8_t* data, size_t size){
  qspi_enter_indirect_mode();
  flash_write_block(address, data, size);  
  return 0;
}
int flash_erase(uint32_t address, size_t size){
  qspi_enter_indirect_mode();
  qspi_erase_block(address, size);
  return 0;
}

#include <string.h>
int flash_read(uint32_t address, uint8_t* data, size_t size){
  qspi_enter_mapped_mode();
  memcpy(data, (void*)(QSPI_FLASH_BASE+address), size);
  return 0;
}
#else
int flash_read(uint32_t address, uint8_t* data, size_t size){
  return flash_read_block(QSPI_READ_MODE, address, data, size);
}

int flash_write(uint32_t address, const uint8_t* data, size_t size){
  return flash_write_block(address, data, size);
}

int flash_erase(uint32_t address, size_t size){
  return qspi_erase_block(address, size);  
}
#endif

void flash_init(void* handle){
  qspi_handle = (QSPI_HandleTypeDef*)handle;
}

int flash_read_register(int instruction){
  QSPI_CommandTypeDef cmd;
  /* Initialize the read flag status register command */
  cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction       = instruction;
  cmd.AddressMode       = QSPI_ADDRESS_NONE;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode          = QSPI_DATA_1_LINE;
  cmd.DummyCycles       = 0;
  cmd.NbData            = 1;
  cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
  cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(qspi_handle, &cmd, QSPI_TIMEOUT) != HAL_OK)
    return -1;

  uint8_t value;
  /* Reception of the data */
  if (HAL_QSPI_Receive(qspi_handle, &value, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  
  return value;
}

int flash_status(){
  // Read Status Register 1 (RDSR1 05h)
  // Read Status Register 2 (RDSR2 07h)
  // Read Configuration Register 1 (RDCR1 35h)
  // Read Configuration Register 2 (RDCR2 15h)
  // Read Configuration Register 3 (RDCR3 33h)
  /* It is possible to read CR1V, CR2V and CR3V continuously by providing multiples of eight clock cycles. */
  return flash_read_register(0x35);
}

int qspi_quad_mode(uint8_t cr1, uint8_t cr2){
  uint8_t data[3] = {0x00, cr1, cr2};
  QSPI_CommandTypeDef cmd;

  // WRENV
  cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction       = QSPI_WRENV_CMD;
  cmd.AddressMode       = QSPI_ADDRESS_NONE;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode          = QSPI_DATA_NONE;
  cmd.DummyCycles       = 0;
  cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
  cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  if(HAL_QSPI_Command(qspi_handle, &cmd, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  if(qspi_wait(QSPI_SR_WEL, QSPI_SR_WEL) != 0)
    return -1;

  // WRR
  cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction       = QSPI_WRR_CMD;
  cmd.AddressMode       = QSPI_ADDRESS_NONE;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode          = QSPI_DATA_1_LINE;
  cmd.DummyCycles       = 0;
  cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
  cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  cmd.NbData            = sizeof(data);    
  if(HAL_QSPI_Command(qspi_handle, &cmd, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  if(HAL_QSPI_Transmit(qspi_handle, data, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  return qspi_wait(0, QSPI_SR_WIP);
  /*
    WRENV 0x50
    WRR 0x01
    WRAR 0x71
    The Write Enable Volatile (WRENV) command must be written prior to Write Register (WRR) command that modifies volatile registers data.
  */ 
}

int qspi_qpi_mode(bool quad){
  /* The recommended procedure for moving to QPI mode is to first use the QPIEN (38h) command, the WRR or WRAR command can also set CR2V[3] = 1, QPI mode
     QPI CR2V[3]: This bit controls the expected instruction width for all commands. This volatile QPI configuration bit enables the
     device to enter and exit QPI mode during normal operation. When this bit is set to QPI mode, the QUAD mode is active, independent
     of the setting of QIO mode (CR1V[1]). When this bit is cleared to legacy SPI mode, the QUAD bit is not affected. The QPI CR2V[3]
     bit can also be set to “1” by the QPIEN (38h) command and set to “0” by the QPIEX (F5h) command.

     The enter QPI Mode (QPIEN) command enables the QPI mode by setting the volatile QPI bit (CR2V[3] = 1). The time required to enter QPI Mode is tQEN, 1.5uS, no other commands are allowed during the tQEN transition time to QPI mode.
  */

  QSPI_CommandTypeDef cmd;  
  cmd.AddressSize       = QSPI_ADDRESS_24_BITS;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
  cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  cmd.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
  cmd.AddressMode = QSPI_ADDRESS_NONE;
  cmd.DataMode    = QSPI_DATA_NONE;
  cmd.DummyCycles = 0;
  cmd.NbData      = 0;
  if(quad){
    cmd.Instruction = QSPI_QPIEN_CMD; // Enter Quad Mode QPIEN
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    if(HAL_QSPI_Command(qspi_handle, &cmd, QSPI_TIMEOUT) != HAL_OK)
      return -1;
  }else{
    // exit quad mode QPIEX
    cmd.Instruction = QSPI_QPIEX_CMD; // Exit Quad Mode QPIEX
    cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    if(HAL_QSPI_Command(qspi_handle, &cmd, QSPI_TIMEOUT) != HAL_OK)
      return -1;
  }
  /* Automatic polling to wait for QUADEN bit=1 and WIP bit=0 */
  qspi_wait(QSPI_SR_QUADEN, QSPI_SR_QUADEN|QSPI_SR_WIP);

  /* volatile size_t counter = 10000; */
  /* while(counter--) */
  /*   __NOP(); */
  return 0;
}

int flash_read_mode(QSPI_CommandTypeDef* cmd, int mode){
/* Configure Volatile Configuration register (with new dummy cycles) */
  /* qspi_dummy_cycles_cfg(qspi_handle); */
  // defaults for all modes
  cmd->InstructionMode    = QSPI_INSTRUCTION_1_LINE;
  cmd->AddressMode        = QSPI_ADDRESS_1_LINE;
  cmd->DataMode           = QSPI_DATA_1_LINE;
  cmd->AddressSize        = QSPI_ADDRESS_32_BITS;
  cmd->DummyCycles        = 8;
  cmd->DdrMode            = QSPI_DDR_MODE_DISABLE;
  cmd->DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
  cmd->SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;
  cmd->AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
  cmd->AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
  cmd->AlternateBytes     = 0xA0; // Continuous read feature is enabled if the mode bits value is Axh.
  switch(mode){
  case -111: // SPI read 1-1-1
    cmd->Instruction       = QSPI_FAST_READ_CMD;
    cmd->AddressSize       = QSPI_ADDRESS_24_BITS;
    break;
  case -112: // Dual out fast read 1-1-2
    cmd->Instruction       = QSPI_DOR_CMD;
    cmd->DataMode          = QSPI_DATA_2_LINES;
    cmd->AddressSize       = QSPI_ADDRESS_24_BITS;
    break;
  case -114: // Quad out read 1-1-4
    cmd->Instruction       = QSPI_QOR_CMD;
    cmd->DataMode          = QSPI_DATA_4_LINES;
    cmd->AddressSize       = QSPI_ADDRESS_24_BITS;
    break;
  case -122: // Dual I/O 1-2-2, mode cycles 4
    cmd->Instruction       = QSPI_DIOR_CMD;
    cmd->AddressMode       = QSPI_ADDRESS_2_LINES;
    cmd->DataMode          = QSPI_DATA_2_LINES;
    cmd->AddressSize       = QSPI_ADDRESS_24_BITS;
    cmd->AlternateByteMode = QSPI_ALTERNATE_BYTES_2_LINES;
    cmd->SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    break;
  case -144: // Quad I/O read 1-4-4, mode cycles 2
    cmd->Instruction       = QSPI_QIOR_CMD;
    cmd->AddressMode       = QSPI_ADDRESS_4_LINES;
    cmd->DataMode          = QSPI_DATA_4_LINES;
    cmd->AddressSize       = QSPI_ADDRESS_24_BITS;
    cmd->AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    cmd->SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    break;
  case -444: // Quad I/O 4-4-4, mode cycles 2
    // QSPI read
    cmd->Instruction       = QSPI_QIOR_CMD;
    cmd->InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    cmd->AddressMode       = QSPI_ADDRESS_4_LINES;
    cmd->DataMode          = QSPI_DATA_4_LINES;
    cmd->AddressSize       = QSPI_ADDRESS_24_BITS;
    cmd->AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    cmd->SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    break;
  case 111: // SPI read 1-1-1 
    cmd->Instruction       = QSPI_FAST_READ_4B_CMD;
    break;
  case 112: // Dual out fast read 1-1-2
    cmd->Instruction       = QSPI_4DOR_CMD;
    cmd->DataMode          = QSPI_DATA_2_LINES;
    break;
  case 122: // Dual I/O 1-2-2, mode cycles 4
    cmd->Instruction       = QSPI_4DIOR_CMD;
    cmd->AddressMode       = QSPI_ADDRESS_2_LINES;
    cmd->DataMode          = QSPI_DATA_2_LINES;
    cmd->AlternateByteMode = QSPI_ALTERNATE_BYTES_2_LINES;
    cmd->SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    break;
  case 114: // Quad out 1-1-4
    cmd->Instruction       = QSPI_4QOR_CMD;
    cmd->DataMode          = QSPI_DATA_4_LINES;
    break;
  case 144: // Quad I/O 1-4-4, mode cycles 2
    // QSPI read
    cmd->Instruction       = QSPI_4QIOR_CMD;
    cmd->AddressMode       = QSPI_ADDRESS_4_LINES;
    cmd->DataMode          = QSPI_DATA_4_LINES;
    cmd->AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    cmd->SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    break;
  case 444: // Quad I/O 4-4-4, mode cycles 2
    // QSPI read
    cmd->Instruction       = QSPI_4QIOR_CMD;
    cmd->InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    cmd->AddressMode       = QSPI_ADDRESS_4_LINES;
    cmd->DataMode          = QSPI_DATA_4_LINES;
    cmd->AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    cmd->SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    break;
  default:
    return -1;
  }
  return 0;
}

int flash_read_block(int mode, uint32_t address, uint8_t* data, size_t size){
  QSPI_CommandTypeDef cmd;
  flash_read_mode(&cmd, mode);
  cmd.Address           = address;
  cmd.NbData            = size;  
  if(HAL_QSPI_Command(qspi_handle, &cmd, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  if(HAL_QSPI_Receive(qspi_handle, data, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  return 0;
}

int flash_memory_map(int mode){
#if 0 // uncomment this to enable quad modes
  if(mode == 114 || mode == 144 || 
     mode == -114 || mode == -144){
    qspi_quad_mode(0x02, 0x60);
    qspi_qpi_mode(false);
  }else if(mode == 444 || mode == -444){
    qspi_quad_mode(0x02, 0x68);
    qspi_qpi_mode(true);
  }else{
    qspi_quad_mode(0x00, 0x60);
    qspi_qpi_mode(false);
  }
#endif
  QSPI_CommandTypeDef cmd;
  flash_read_mode(&cmd, mode);
  QSPI_MemoryMappedTypeDef cfg;
  cfg.TimeOutPeriod = 0;
  cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
  if(HAL_QSPI_MemoryMapped(qspi_handle, &cmd, &cfg) != HAL_OK)
    return -1;
  return 0;
}

#endif
