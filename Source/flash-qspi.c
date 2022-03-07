#include "device.h"
#include "flash.h"
#include "main.h"
#include "qspi.h"

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

/* Buffer used for transmission */

static QSPI_HandleTypeDef* qspi_handle;
volatile uint8_t CmdCplt, RxCplt, TxCplt, StatusMatch;
uint8_t qspi_status = 0;

// used only in flash_memory_map ? and flash_read_block
static QSPI_CommandTypeDef      sCommand;
static QSPI_MemoryMappedTypeDef sMemMappedCfg;

/*
  Single bit wide commands start with an instruction and may provide an address 
  or data, all sent only on the SI signal. Data may be sent back to the host 
  serially on the SO signal. This is referenced as a 1-1-1 command protocol for 
  single bit width instruction, single bit width address and modifier, single bit 
  data.
 */

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
  cmd.Instruction       = 0x50;
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
    sCommand.Address     = address;
    if (HAL_QSPI_Command(qspi_handle, &sCommand, QSPI_TIMEOUT) != HAL_OK)
      return -1;
    address += QSPI_ERASE_BLOCK_SIZE;
    remain -= QSPI_ERASE_BLOCK_SIZE;
  }
  return 0;
}

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

int flash_write_page(uint32_t address, void* data, size_t size){
  /* Writing Sequence ------------------------------------------------ */
  sCommand.Instruction = QUAD_IN_FAST_PROG_CMD;
  sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
  sCommand.DataMode    = QSPI_DATA_4_LINES;
  sCommand.DummyCycles = 0;
  sCommand.Address     = address;
  sCommand.NbData      = size;

    // To use Quad Page Program the Quad Enable Bit in the Configuration Register must be set (QUAD=1). A Write Enable command must be executed before the device will accept the QPP command (Status Register 1, WEL=1).
 
  // SPI alternative
  sCommand.Instruction     = PAGE_PROG_CMD;
  sCommand.AddressMode     = QSPI_ADDRESS_1_LINE;
  sCommand.DataMode        = QSPI_DATA_1_LINE;

  /* The Page Program command accepts from 1-byte up to 256 consecutive bytes of data (page) to be programmed in one operation. */

  if (HAL_QSPI_Command(qspi_handle, &sCommand, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  if (HAL_QSPI_Transmit(qspi_handle, data, QSPI_TIMEOUT) != HAL_OK)
    return -1;
  return 0;
}

int flash_write_block(uint32_t address, const uint8_t *data, size_t size){
  /* Enable write operations ----------------------------------------- */
  qspi_write_enable(qspi_handle, true);

  size_t pages = (size+QSPI_FLASH_PAGE_SIZE-1)/QSPI_FLASH_PAGE_SIZE;
  uint8_t* source = (uint8_t*)data;
  int ret = 0;
  while(ret == 0 && pages--){
    flash_wait();
    ret = flash_write_page(address, source, min(QSPI_FLASH_PAGE_SIZE, size));
    address += QSPI_FLASH_PAGE_SIZE;
    source += QSPI_FLASH_PAGE_SIZE;
    size -= QSPI_FLASH_PAGE_SIZE;
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
  sCommand.Instruction       = nv ? 0x06 : 0x50; // WRITE_ENABLE_CMD;
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
  sConfig.Match           = 0x02;
  sConfig.Mask            = 0x02;
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
