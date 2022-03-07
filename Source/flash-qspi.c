#include "device.h"
#include "flash.h"
#include "main.h"
#include "qspi.h"


// =======================
// SPI Flash Commands info
// =======================
#define SPI_WRR_CMD                 (0x01)
#define SPI_PP_CMD                  (0x02)
#define SPI_READ_CMD                (0x03)
#define SPI_WRDI_CMD                (0x04)
#define SPI_RDSR_CMD                (0x05)
#define SPI_WREN_CMD                (0x06)
#define SPI_RDSR2_CMD               (0x07)  // Read Status Register-2
#define SPI_FAST_READ_CMD           (0x0B)
#define SPI_FAST_READ_4B_CMD        (0x0C)
#define SPI_PP_4B_CMD               (0x12)
#define SPI_READ_4B_CMD             (0x13)
#define SPI_RDCR2_CMD               (0x15) // Read configuration register-2
#define SPI_IRPRD_CMD               (0x2B) // IRP register Read
#define SPI_IRPP_CMD                  (0x2F) // IRP register program
#define SPI_CLSR_CMD                (0x30)
#define SPI_QPP_CMD                 (0x32)
#define SPI_RDCR3_CMD           (0x33) // Read Configuration Register-3
#define SPI_QPP_4B_CMD              (0x34)
#define SPI_RCR_CMD                 (0x35)
#define SPI_IBL_CMD                   (0x36) // IBL lock
#define SPI_QPIEN_CMD               (0x38) // Enter QPI
#define SPI_IBUL_CMD                 (0x39) // IBL unlock
#define SPI_DOR_CMD             (0x3B) // Dual output read
#define SPI_4DOR_CMD              (0x3C) // Dual output read 4 Byte address 
#define SPI_IBLRD_CMD           (0x3D) // IBL read
#define SPI_DLPRD_CMD               (0x41)  // Read Data Learning Pattern
#define SPI_SECRP_CMD           (0x42) // security region program
#define SPI_PNVDLR_CMD              (0x43)  // Program NV Data Learning Register
#define SPI_SECRE_CMD             (0x44) // security region erase
#define SPI_SECRR_CMD             (0x48) // security region read
#define SPI_WVDLR_CMD               (0x4A)  // Write Volatile Data Learning Register
#define SPI_RUID_CMD                (0x4B)
#define SPI_WRENV_CMD            (0x50) // Write enable for Volatile status and configureation registers
#define SPI_HBE_CMD             (0x52) // Half Block Erase
#define SPI_4HBE_CMD            (0x53) // Half Block Erase 4B address
#define SPI_READ_SFDP_CMD           (0x5A)  // Read Serial Flash Discoverable Parameter Register
#define SPI_CE_CMD              (0x60) // chip erase
#define SPI_RDAR_CMD                (0x65)  // Read Any Register
#define SPI_RSTEN_CMD               (0x66)  // Software Reset Enable
#define SPI_QOR_CMD                 (0x6B) // Quad Output Read
#define SPI_4QOR_CMD                  (0x6C) // Quad Output Read 4B address
#define SPI_WRAR_CMD                (0x71)  // Write Any Register
#define SPI_SETBURSTWRAP_CMD        (0x77)  // Set Burst with Wrap
#define SPI_GBL_CMD               (0x7E) // Global IBL lock
//#define SPI_CLSR2_CMD               (0x82)  //Clear Status Register 1 (alternate instruction) - Erase/Prog. Fail Reset
#define SPI_RDID_90_CMD  (0x90)
#define SPI_GBUL_CMD                (0x98) // Global IBL unlock 
#define SPI_RESET_CMD               (0x99)  // Software Reset
#define SPI_RDID_9F_CMD          (0x9F)
#define SPI_PRL_CMD                (0xA6) // Protection Register lock
#define SPI_PRRD_CMD                  (0xA7) // Protection Register read
#define SPI_RDID_AB_CMD             (0xAB)
#define SPI_RES_CMD                 (0xAB)
#define SPI_RDQID_CMD               (0xAF)  //Read Quad ID
#define SPI_EPS_CMD                 (0x75)  // for FLL
#define SPI_EPR_CMD                 (0x7A) // for FLL
#define SPI_4BEN_CMD                (0xB7) // Enter 4-bytes Address Mode
#define SPI_DP_CMD                  (0xB9)
#define SPI_DIOR_CMD            (0xBB) // Dual IO read
#define SPI_4DIOR_CMD          (0xBC) // Dual IO read 4B address
#define SPI_CE1_CMD              (0xC7) // chip erase
#define SPI_SE_CMD          (0x20)  // Sector Erase
#define SPI_SE_4B_CMD               (0x21)
#define SPI_BLOCK_ERASE_CMD         (0xD8)  // BLOCK ERASE
#define SPI_4BE_CMD             (0xDC) // Block erase 4B address
#define SPI_4IBLRD_CMD              (0xE0) // IBL read 4B address
#define SPI_4IBL_CMD                   (0xE1) // IBL lock 4B address
#define SPI_4IBUL_CMD                 (0xE2) // IBL unlock 4B address
#define SPI_4SPRP_CMD               (0xE3) // Set Point Region Protection 4B address
#define SPI_PASSRD_CMD              (0xE7)  // Password Read
#define SPI_PASSP_CMD               (0xE8)  // Password Program
#define SPI_4BEX_CMD                (0xE9) // Exit 4 Byte Address mode
#define SPI_PASSU_CMD            (0xEA) // Password Unlock
#define SPI_QIOR_CMD            (0xEB) // Quad IO read
#define SPI_4QIOR_CMD          (0xEC) // Quad IO read 4B address
#define SPI_DDRQIOR_CMD         (0xED) // DDR Quad IO read
#define SPI_4DDRQIOR_CMD       (0xEE) // DDR Quad IO read 4B address
#define SPI_QPIEX_CMD               (0xF5) // Exit QPI
#define SPI_SPRP_CMD                (0xFB) // Set Pointer Region Protection
#define SPI_MBR_CMD                 (0xFF) // Mode Bit Reset

#define QSPI_SR_WEL                      ((uint8_t)0x02)    /* Write Enable Latch */
#define QSPI_SR_WIP                      ((uint8_t)0x01)    /* Write In Progress */

#define QSPI_READ_MODE               -122
#define QSPI_TIMEOUT                 50000
/* #define QSPI_TIMEOUT HAL_QPSI_TIMEOUT_DEFAULT_VALUE */

int flash_quad_mode(uint8_t cr1, uint8_t cr2);
int flash_qpi_mode(bool qpi);
static int flash_read_block(int mode, uint32_t address, uint8_t* data, size_t size);
static int flash_write_block(uint32_t address, const uint8_t* data, size_t size);
static int flash_erase_block(uint32_t address, size_t size);
static void qspi_write_enable(QSPI_HandleTypeDef *hqspi, bool nv);
static int qspi_autopolling_ready(QSPI_HandleTypeDef *hqspi);
static void qspi_dummy_cycles_cfg(QSPI_HandleTypeDef *hqspi);

static int qspi_mapped_mode = 0;

/*
  Single bit wide commands start with an instruction and may provide an address 
  or data, all sent only on the SI signal. Data may be sent back to the host 
  serially on the SO signal. This is referenced as a 1-1-1 command protocol for 
  single bit width instruction, single bit width address and modifier, single bit 
  data.
 */

void qspi_enter_indirect_mode(){
  if(qspi_mapped_mode == 1){
    extern QSPI_HandleTypeDef QSPI_FLASH_HANDLE;
    flash_init(&QSPI_FLASH_HANDLE);
    qspi_mapped_mode = 0;
  }
}

void qspi_enter_mapped_mode(){
  if(qspi_mapped_mode == 0){
    extern QSPI_HandleTypeDef QSPI_FLASH_HANDLE;
    flash_init(&QSPI_FLASH_HANDLE);
    flash_status();
    flash_memory_map(-122);
    qspi_mapped_mode = 1;
  }
}

#if 1
int flash_write(uint32_t address, const uint8_t* data, size_t size){
  qspi_enter_indirect_mode();
  flash_wait();
  flash_write_block(address, data, size);  
  flash_wait();
  return 0;
}
int flash_erase(uint32_t address, size_t size){
  qspi_enter_indirect_mode();
  flash_wait();
  flash_erase_block(address, size);
  flash_wait();
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
  flash_wait();
  flash_read_block(QSPI_READ_MODE, address, data, size);
  return flash_wait();
}

int flash_write(uint32_t address, const uint8_t* data, size_t size){
  flash_wait();
  flash_write_block(address, data, size);
  return flash_wait();
}

int flash_erase(uint32_t address, size_t size){
  flash_wait();
  flash_erase_block(address, size);
  return flash_wait();
}
#endif
/* Buffer used for transmission */

static QSPI_HandleTypeDef* qspi_handle;
volatile uint8_t CmdCplt, RxCplt, TxCplt, StatusMatch;
uint8_t qspi_status = 0;

// used only in flash_memory_map ? and flash_read_block
static QSPI_CommandTypeDef      sCommand;
static QSPI_MemoryMappedTypeDef sMemMappedCfg;

static int flash_reset(QSPI_HandleTypeDef *hqspi) {
  QSPI_CommandTypeDef cmd;
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction = RESET_ENABLE_CMD;
  cmd.AddressMode = QSPI_ADDRESS_NONE;
  cmd.DataMode = QSPI_DATA_NONE;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DummyCycles = 0;
  cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
  cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
  if(HAL_QSPI_Command(hqspi, &cmd, QSPI_TIMEOUT) != HAL_OK) 
    return -1;
  cmd.Instruction = RESET_MEMORY_CMD;
  if(HAL_QSPI_Command(hqspi, &cmd, QSPI_TIMEOUT) != HAL_OK) 
    return -1;
  return qspi_autopolling_ready(hqspi);
}

#if 1
void flash_init(void* handle){
  qspi_handle = (QSPI_HandleTypeDef*)handle;
  qspi_handle->Instance = QUADSPI;
  /* QSPI clock = 480MHz / (1+9) = 48MHz */
  /* QSPI clock = 480MHz / (1+4) = 96MHz */
  qspi_handle->Init.ClockPrescaler     = 9;
  /* qspi_handle->Init.ClockPrescaler     = 4; // 4 and 5 don't work? 8 works  */
/* #define IS_QSPI_FIFO_THRESHOLD(THR)        (((THR) > 0U) && ((THR) <= 32U)) */
  qspi_handle->Init.FifoThreshold      = 4;
  qspi_handle->Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_NONE;
  qspi_handle->Init.FlashSize          = 22; // 2^(22+1) = 8M / 64Mbit
  qspi_handle->Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
  qspi_handle->Init.ClockMode          = QSPI_CLOCK_MODE_0;

  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; // QSPI_ALTERNATE_BYTES_8_BITS
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
  /* Initialize QuadSPI ------------------------------------------------ */
  HAL_QSPI_DeInit(qspi_handle);

  if ((FlagStatus)(__HAL_QSPI_GET_FLAG(qspi_handle, QSPI_FLAG_BUSY)) == SET){
    qspi_handle->State = HAL_QSPI_STATE_BUSY;
    HAL_QSPI_Abort(qspi_handle);
  }

  if (HAL_QSPI_Init(qspi_handle) != HAL_OK)
    {
      Error_Handler();
    }
  if( HAL_QSPI_SetFifoThreshold(qspi_handle, 16) != HAL_OK)
    {
      Error_Handler();
    }

  qspi_status = flash_status();
}
#else
void flash_init(void* handle){
  qspi_handle = (QSPI_HandleTypeDef*)handle;

  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; // QSPI_ALTERNATE_BYTES_8_BITS
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;

   // Important - this prevents timeout due to previous operations in some cases
  // https://community.st.com/s/question/0D50X00009XkXMHSA3/qspi-flag-qspiflagbusy-sometimes-stays-set
  /* if ((FlagStatus)(__HAL_QSPI_GET_FLAG(qspi_handle, QSPI_FLAG_BUSY)) == SET){ */
  /*   qspi_handle->State = HAL_QSPI_STATE_BUSY; */
  /*   HAL_QSPI_Abort(qspi_handle); */
  /* } */
  /* qspi_status = flash_status(); */

  /* flash_reset(qspi_handle); */

  /* qspi_dummy_cycles_cfg(qspi_handle); */
  /* flash_quad_mode(0x00, 0x60); */
  /* flash_qpi_mode(false); */  
}
#endif

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

int flash_quad_mode(uint8_t cr1, uint8_t cr2){ //, uint8_t cr3){
  uint8_t data[3] = {0x00, cr1, cr2};
  QSPI_CommandTypeDef cmd;
  /* qspi_write_enable(qspi_handle, false); // WRENV */

  // WRENV
  cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction       = SPI_WRENV_CMD;
  cmd.AddressMode       = QSPI_ADDRESS_NONE;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode          = QSPI_DATA_NONE;
  cmd.DummyCycles       = 0;
  cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
  cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  if (HAL_QSPI_Command(qspi_handle, &cmd, QSPI_TIMEOUT) != HAL_OK)
    return -1;

  // WRR
  cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction       = 0x01; // WRR
  cmd.AddressMode       = QSPI_ADDRESS_NONE;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode          = QSPI_DATA_1_LINE;
  cmd.DummyCycles       = 0;
  cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
  cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  cmd.NbData            = sizeof(data);    
  if (HAL_QSPI_Command(qspi_handle, &cmd, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  if (HAL_QSPI_Transmit(qspi_handle, data, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  return 0;
  /*
    WRENV 0x50
    WRR 0x01
    WRAR 0x71
    The Write Enable Volatile (WRENV) command must be written prior to Write Register (WRR) command that modifies volatile registers data.
  */ 
}

int flash_erase_block(uint32_t address, size_t size){
  /* Enable write operations ------------------------------------------- */
  qspi_write_enable(qspi_handle, true);

  /* Erasing Sequence -------------------------------------------------- */
  sCommand.Instruction = SECTOR_ERASE_CMD;
  sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
  sCommand.DataMode    = QSPI_DATA_NONE;
  sCommand.DummyCycles = 0;
  sCommand.NbData      = 0;
  int remain = size;
  while(remain > 0){
    flash_wait();
    sCommand.Address     = address;
    if(HAL_QSPI_Command(qspi_handle, &sCommand, QSPI_TIMEOUT) != HAL_OK)
      return -1;
    address += QSPI_ERASE_BLOCK_SIZE;
    remain -= QSPI_ERASE_BLOCK_SIZE;
  }
  return 0;
}

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

static uint32_t write_enable(QSPI_HandleTypeDef *hqspi) {
  QSPI_CommandTypeDef s_command;
  QSPI_AutoPollingTypeDef s_config;

  /* Enable write operations */
  s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction = WRITE_ENABLE_CMD;
  s_command.AddressMode = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode = QSPI_DATA_NONE;
  s_command.DummyCycles = 0;
  s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return -1;
  }

  /* Configure automatic polling mode to wait for write enabling */
  //		s_config.Match           = IS25LP080D_SR_WREN |
  //(IS25LP080D_SR_WREN
  //<< 8); 		s_config.Mask            = IS25LP080D_SR_WREN |
  //(IS25LP080D_SR_WREN
  //<< 8);
  s_config.MatchMode = QSPI_MATCH_MODE_AND;
  s_config.Match = QSPI_SR_WEL;
  s_config.Mask = QSPI_SR_WEL;
  s_config.Interval = 0x10;
  s_config.StatusBytesSize = 1;
  s_config.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

  s_command.Instruction = READ_STATUS_REG_CMD;
  s_command.DataMode = QSPI_DATA_1_LINE;

  if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config,
                           HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return -1;
  }
  return 0;
}

static uint32_t autopolling_mem_ready(QSPI_HandleTypeDef *hqspi,
                                      uint32_t timeout) {
  QSPI_CommandTypeDef s_command;
  QSPI_AutoPollingTypeDef s_config;

  /* Configure automatic polling mode to wait for memory ready */
  s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction = READ_STATUS_REG_CMD;
  s_command.AddressMode = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode = QSPI_DATA_1_LINE;
  s_command.DummyCycles = 0;
  s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

  s_config.Match = 0;
  s_config.MatchMode = QSPI_MATCH_MODE_AND;
  s_config.Mask = QSPI_SR_WIP;
  s_config.StatusBytesSize = 1;
  s_config.Interval = 0x10;
  s_config.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, timeout) != HAL_OK) {
    return -1;
  }
  return 0;
}

int flash_write_page(uint32_t address, void* data, size_t size){
  QSPI_CommandTypeDef s_command;
  s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction = PAGE_PROG_CMD;
  s_command.AddressMode = QSPI_ADDRESS_1_LINE;
  s_command.AddressSize = QSPI_ADDRESS_24_BITS;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode = QSPI_DATA_1_LINE;
  s_command.DummyCycles = 0;
  s_command.NbData = size <= 256 ? size : 256;
  s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
  s_command.Address = address;
  if (write_enable(qspi_handle) != 0) {
    return -1;
  }
  if (HAL_QSPI_Command(qspi_handle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) !=
      0) {
    return -1;
  }
  if (HAL_QSPI_Transmit(qspi_handle, (uint8_t *)data,
                        HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != 0) {
    return -1;
  }
  if (autopolling_mem_ready(qspi_handle, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) !=
      0) {
    return -1;
  }
  return 0;
}

/* int flash_write_page(uint32_t address, void* data, size_t size){ */
  
/*   /\* Writing Sequence ------------------------------------------------ *\/ */
/*   sCommand.Instruction = QUAD_IN_FAST_PROG_CMD; */
/*   sCommand.AddressMode = QSPI_ADDRESS_1_LINE; */
/*   sCommand.DataMode    = QSPI_DATA_4_LINES; */
/*   sCommand.DummyCycles = 0; */
/*   sCommand.Address     = address; */
/*   sCommand.NbData      = size; */

/*     // To use Quad Page Program the Quad Enable Bit in the Configuration Register must be set (QUAD=1). A Write Enable command must be executed before the device will accept the QPP command (Status Register 1, WEL=1). */
 
/*   // SPI alternative */
/*   sCommand.Instruction     = PAGE_PROG_CMD; */
/*   sCommand.AddressMode     = QSPI_ADDRESS_1_LINE; */
/*   sCommand.DataMode        = QSPI_DATA_1_LINE; */

/*   /\* The Page Program command accepts from 1-byte up to 256 consecutive bytes of data (page) to be programmed in one operation. *\/ */

/*   if (HAL_QSPI_Command(qspi_handle, &sCommand, QSPI_TIMEOUT) != HAL_OK) */
/*     return -1; */
/*   flash_wait(); */
/*   if (HAL_QSPI_Transmit(qspi_handle, data, QSPI_TIMEOUT) != HAL_OK) */
/*     return -1; */
/*   return 0; */
/* } */

int flash_write_block(uint32_t address, const uint8_t *data, size_t size){
  int ret = 0;
  const uint32_t end = address + size;
  uint8_t* src = (uint8_t*)data;
  while(ret == 0 && address < end){
    qspi_write_enable(qspi_handle, true);
    flash_wait();
    size_t len = min(QSPI_FLASH_PAGE_SIZE, size);
    ret = flash_write_page(address, src, len);
    address += len;
    src += len;
    size -= len;
  }
  return ret;
}

int flash_qpi_mode(bool quad){
  /* The recommended procedure for moving to QPI mode is to first use the QPIEN (38h) command, the WRR or WRAR command can also set CR2V[3] = 1, QPI mode
     QPI CR2V[3]: This bit controls the expected instruction width for all commands. This volatile QPI configuration bit enables the
     device to enter and exit QPI mode during normal operation. When this bit is set to QPI mode, the QUAD mode is active, independent
     of the setting of QIO mode (CR1V[1]). When this bit is cleared to legacy SPI mode, the QUAD bit is not affected. The QPI CR2V[3]
     bit can also be set to “1” by the QPIEN (38h) command and set to “0” by the QPIEX (F5h) command.

     The enter QPI Mode (QPIEN) command enables the QPI mode by setting the volatile QPI bit (CR2V[3] = 1). The time required to enter QPI Mode is tQEN, 1.5uS, no other commands are allowed during the tQEN transition time to QPI mode.
  */
  sCommand.AddressMode = QSPI_ADDRESS_NONE;
  sCommand.DataMode    = QSPI_DATA_NONE;
  sCommand.DummyCycles = 0;
  sCommand.NbData      = 0;
  if(quad){
    // enable quad mode QPIEN
    sCommand.Instruction = ENTER_QUAD_CMD;
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    if (HAL_QSPI_Command(qspi_handle, &sCommand, QSPI_TIMEOUT) != HAL_OK)
      return -1;
    sCommand.InstructionMode = QSPI_INSTRUCTION_4_LINES;
  }else{
    // exit quad mode QPIEX
    sCommand.Instruction = EXIT_QUAD_CMD;
    sCommand.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    if (HAL_QSPI_Command(qspi_handle, &sCommand, QSPI_TIMEOUT) != HAL_OK)
      return -1;
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  }
  volatile size_t counter = 10000;
  while(counter--)
    __NOP();
  return 0;
}

int flash_read_mode(int mode){
/* Configure Volatile Configuration register (with new dummy cycles) */
  /* qspi_dummy_cycles_cfg(qspi_handle); */

  // defaults for all modes
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.DummyCycles       = 8;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.AlternateBytes     = 0xA0; // Continuous read feature is enabled if the mode bits value is Axh.

  switch(mode){
  case -111: // SPI read 1-1-1
    sCommand.Instruction       = QSPI_FLASH_CMD_FAST_READ;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    break;
  case -112: // Dual out fast read 1-1-2
    sCommand.Instruction       = QSPI_FLASH_CMD_DOR;
    sCommand.DataMode          = QSPI_DATA_2_LINES;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    break;
  case -114: // Quad out read 1-1-4
    sCommand.Instruction       = QSPI_FLASH_CMD_QOR;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    break;
  case -122: // Dual I/O 1-2-2, mode cycles 4
    sCommand.Instruction       = QSPI_FLASH_CMD_DIOR;
    sCommand.AddressMode       = QSPI_ADDRESS_2_LINES;
    sCommand.DataMode          = QSPI_DATA_2_LINES;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_2_LINES;
    sCommand.SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    break;
  case -144: // Quad I/O read 1-4-4, mode cycles 2
    sCommand.Instruction       = QSPI_FLASH_CMD_QIOR;
    sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    sCommand.SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    break;
  case -444: // Quad I/O 4-4-4, mode cycles 2
    // QSPI read
    sCommand.Instruction       = QSPI_FLASH_CMD_QIOR;
    sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    sCommand.SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    break;
  case 111: // SPI read 1-1-1 
    sCommand.Instruction       = QSPI_FLASH_CMD_4FAST_READ;
    break;
  case 112: // Dual out fast read 1-1-2
    sCommand.Instruction       = QSPI_FLASH_CMD_4DOR;
    sCommand.DataMode          = QSPI_DATA_2_LINES;
    break;
  case 122: // Dual I/O 1-2-2, mode cycles 4
    sCommand.Instruction       = QSPI_FLASH_CMD_4DIOR;
    sCommand.AddressMode       = QSPI_ADDRESS_2_LINES;
    sCommand.DataMode          = QSPI_DATA_2_LINES;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_2_LINES;
    sCommand.SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    break;
  case 114: // Quad out 1-1-4
    sCommand.Instruction       = QSPI_FLASH_CMD_4QOR;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    break;
  case 144: // Quad I/O 1-4-4, mode cycles 2
    // QSPI read
    sCommand.Instruction       = QSPI_FLASH_CMD_4QIOR;
    sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    sCommand.SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    break;
  case 444: // Quad I/O 4-4-4, mode cycles 2
    // QSPI read
    sCommand.Instruction       = QSPI_FLASH_CMD_4QIOR;
    sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    sCommand.SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    break;
  default:
    return -1;
  }
  return 0;
}

int flash_read_block(int mode, uint32_t address, uint8_t* data, size_t size){
  flash_read_mode(mode);
  sCommand.Address           = address;
  sCommand.NbData            = size;
  if (HAL_QSPI_Command(qspi_handle, &sCommand, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  if (HAL_QSPI_Receive(qspi_handle, data, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  return 0;
}

int flash_wait(){
  while(qspi_handle->State == HAL_QSPI_STATE_BUSY)
    HAL_Delay(1);
  /* return qspi_handle->State == HAL_QSPI_STATE; */
  /* while(CmdCplt != 0) */
  /*   HAL_Delay(10);   */
  /* CmdCplt = 0; */
  /* StatusMatch = 0; */
  /* /\* Configure automatic polling mode to wait for end of erase or write *\/  */
  qspi_autopolling_ready(qspi_handle);
  /* while(StatusMatch != 0) */
  /*   HAL_Delay(10); */
  /* return 0; */
  return HAL_QSPI_GetError(qspi_handle);
}

int flash_memory_map(int mode){
  if(mode == 114 || mode == 144 || 
     mode == -114 || mode == -144){
    flash_quad_mode(0x02, 0x60);
    flash_qpi_mode(false);
  }else if(mode == 444 || mode == -444){
    flash_quad_mode(0x02, 0x68);
    flash_qpi_mode(true);
  }else{
    flash_quad_mode(0x00, 0x60);
    flash_qpi_mode(false);
  }

  flash_read_mode(mode);
  sMemMappedCfg.TimeOutPeriod = 0;
  sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
  if (HAL_QSPI_MemoryMapped(qspi_handle, &sCommand, &sMemMappedCfg) != HAL_OK)
    return -1;
  return 0;
}

/**
  * @brief  Command completed callbacks.
  * @param  hqspi: QSPI handle
  * @retval None
  */
void HAL_QSPI_CmdCpltCallback(QSPI_HandleTypeDef *hqspi)
{
  CmdCplt++;
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  hqspi: QSPI handle
  * @retval None
  */
void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
  RxCplt++;
}

/**
  * @brief  Tx Transfer completed callbacks.
  * @param  hqspi: QSPI handle
  * @retval None
  */
void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
  CmdCplt++;
  TxCplt++; 
}

/**
  * @brief  Status Match callbacks
  * @param  hqspi: QSPI handle
  * @retval None
  */
void HAL_QSPI_StatusMatchCallback(QSPI_HandleTypeDef *hqspi)
{
  StatusMatch++;
}

/**
  * @brief  This function send a Write Enable and wait it is effective.
  * @param  hqspi: QSPI handle
  * @retval None
  */
void qspi_write_enable(QSPI_HandleTypeDef *hqspi, bool nv)
{
  QSPI_CommandTypeDef     sCommand;
  QSPI_AutoPollingTypeDef sConfig;

  /* Enable write operations ------------------------------------------ */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = nv ? SPI_WREN_CMD : SPI_WRENV_CMD; // WRite ENable (Volatile)
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(hqspi, &sCommand, QSPI_TIMEOUT) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* Configure automatic polling mode to wait for write enabling ---- */  
  sConfig.Match           = QSPI_SR_WEL;
  sConfig.Mask            = QSPI_SR_WEL;
  sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
  sConfig.StatusBytesSize = 1;
  sConfig.Interval        = 0x10;
  sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  sCommand.Instruction    = READ_STATUS_REG_CMD;
  sCommand.DataMode       = QSPI_DATA_1_LINE;

  if (HAL_QSPI_AutoPolling(hqspi, &sCommand, &sConfig, QSPI_TIMEOUT) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function read the SR of the memory and wait the EOP.
  * @param  hqspi: QSPI handle
  * @retval None
  */
int qspi_autopolling_ready(QSPI_HandleTypeDef *hqspi){
  QSPI_CommandTypeDef     sCommand;
  QSPI_AutoPollingTypeDef sConfig;

  /* Configure automatic polling mode to wait for memory ready ------ */  
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = READ_STATUS_REG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  sConfig.Match           = 0x00;
  sConfig.Mask            = 0x01;
  sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
  sConfig.StatusBytesSize = 1;
  sConfig.Interval        = 0x10;
  sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_QSPI_AutoPolling(hqspi, &sCommand, &sConfig, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  return 0;
}

/**
  * @brief  This function configure the dummy cycles on memory side.
  * @param  hqspi: QSPI handle
  * @retval None
  */
void qspi_dummy_cycles_cfg(QSPI_HandleTypeDef *hqspi){
  QSPI_CommandTypeDef sCommand;
  uint8_t reg;

  /* Read Volatile Configuration register --------------------------- */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = READ_VOL_CFG_REG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  sCommand.NbData            = 1;

  if (HAL_QSPI_Command(hqspi, &sCommand, QSPI_TIMEOUT) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_QSPI_Receive(hqspi, &reg, QSPI_TIMEOUT) != HAL_OK)
  {
    Error_Handler();
  }

  /* Enable write operations ---------------------------------------- */
  qspi_write_enable(hqspi, false);

  /* Write Volatile Configuration register (with new dummy cycles) -- */  
  sCommand.Instruction = WRITE_VOL_CFG_REG_CMD;
  MODIFY_REG(reg, 0xF0, (DUMMY_CLOCK_CYCLES_READ_QUAD << POSITION_VAL(0xF0)));
      
  if (HAL_QSPI_Command(hqspi, &sCommand, QSPI_TIMEOUT) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_QSPI_Transmit(hqspi, &reg, QSPI_TIMEOUT) != HAL_OK)
  {
    Error_Handler();
  }
}
