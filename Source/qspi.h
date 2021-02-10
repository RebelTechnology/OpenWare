/* MT25TL01GHBA8ESF Micron memory */
/* Size of the flash */

#define QSPI_PAGE_SIZE                       256 // Page Programming allows up to 256bytes to be programmed in one operation

#define QSPI_ERASE_BLOCK_SIZE                (64*1024) // 64k block sector
#define QSPI_FLASH_PAGE_SIZE                 256 // Program page size 256 bytes

/* Reset Operations */
#define RESET_ENABLE_CMD                     0x66
#define RESET_MEMORY_CMD                     0x99

/* Identification Operations */
#define READ_ID_CMD                          0x9E
#define READ_ID_CMD2                         0x9F
#define MULTIPLE_IO_READ_ID_CMD              0xAF

/* Read Operations */
#define READ_CMD                             0x03 // Read READ
#define READ_4_BYTE_ADDR_CMD                 0x13 // Read 4READ

#define QSPI_FLASH_CMD_FAST_READ             0x0b // Fast Read FAST_READ (1-1-1)
#define QSPI_FLASH_CMD_4FAST_READ            0x0c // Fast Read 4FAST_READ

#define QSPI_FLASH_CMD_DOR                   0x3b // Dual Output Read DOR (1-1-2)
#define QSPI_FLASH_CMD_4DOR                  0x3c // Dual Output Read 4DOR

#define QSPI_FLASH_CMD_QOR                   0x6b // Quad Output Read QOR (1-1-4)
#define QSPI_FLASH_CMD_4QOR                  0x6c // Quad Output Read 4QOR

#define QSPI_FLASH_CMD_DIOR                  0xbb // Dual I/O Read DIOR (1-2-2)
#define QSPI_FLASH_CMD_4DIOR                 0xbc // Dual I/O Read 4DIOR

#define QSPI_FLASH_CMD_QIOR                  0xeb // Quad I/O Read QIOR (1-4-4 or 4-4-4)
#define QSPI_FLASH_CMD_4QIOR                 0xec // Quad I/O Read 4QIOR

/* Write Operations */
#define WRITE_ENABLE_CMD                     0x06 // Write Enable for Nonvolatile data change WREN
#define WRITE_DISABLE_CMD                    0x04 // Write Disable WRDI 

/* Register Operations */
#define READ_STATUS_REG_CMD                  0x05 // Read Status Register 1 RDSR1
#define WRITE_STATUS_REG_CMD                 0x01 // Write Register (Status-1 and Configuration-1,2,3) WRR

/* Write Registers (WRR 01h), Read Any */
/* Register (RDAR 65h), Write Any Register (WRAR 71h) */
/* Read Status Register 1 (RDSR1 05h), Write Enable for Volatile (WRENV 50h), Write Registers (WRR 01h), */
/* Clear Status Register (CLSR 30h) */

#define READ_LOCK_REG_CMD                    0xE8
#define WRITE_LOCK_REG_CMD                   0xE5

#define READ_FLAG_STATUS_REG_CMD             0x70
#define CLEAR_FLAG_STATUS_REG_CMD            0x50

#define READ_NONVOL_CFG_REG_CMD              0xB5
#define WRITE_NONVOL_CFG_REG_CMD             0xB1

#define READ_VOL_CFG_REG_CMD                 0x85
#define WRITE_VOL_CFG_REG_CMD                0x81

#define READ_ENHANCED_VOL_CFG_REG_CMD        0x65
#define WRITE_ENHANCED_VOL_CFG_REG_CMD       0x61

#define READ_EXT_ADDR_REG_CMD                0xC8
#define WRITE_EXT_ADDR_REG_CMD               0xC5

/* Program Operations */
#define PAGE_PROG_CMD                        0x02 // Page Program PP
#define PAGE_PROG_4_BYTE_ADDR_CMD            0x12 // Page Program 4PP

/* #define DUAL_IN_FAST_PROG_CMD                0xA2 */
/* #define EXT_DUAL_IN_FAST_PROG_CMD            0xD2 */

#define QUAD_IN_FAST_PROG_CMD                0x32 // Quad Page Program QPP
#define QUAD_IN_FAST_PROG_4_BYTE_ADDR_CMD    0x34 // Quad Page Program 4QPP

/* Erase Operations */
#define SUBSECTOR_ERASE_CMD                  0x20 // Sector Erase SE (4KB)
#define SUBSECTOR_ERASE_4_BYTE_ADDR_CMD      0x21 // Sector Erase 4SE

#define SECTOR_ERASE_CMD                     0xD8 // Block Erase BE (64KB)
#define SECTOR_ERASE_4_BYTE_ADDR_CMD         0xDC // Block Erase 4BE

#define BULK_ERASE_CMD                       0xC7

#define PROG_ERASE_RESUME_CMD                0x7A
#define PROG_ERASE_SUSPEND_CMD               0x75

/* One-Time Programmable Operations */
#define READ_OTP_ARRAY_CMD                   0x4B
#define PROG_OTP_ARRAY_CMD                   0x42

/* 4-byte Address Mode Operations */
#define ENTER_4_BYTE_ADDR_MODE_CMD           0xB7
#define EXIT_4_BYTE_ADDR_MODE_CMD            0xE9

/* Quad Operations */
#define ENTER_QUAD_CMD                       0x38 // was 0x35 Enter QPI QPIEN (S25FL064L: 38h)
#define EXIT_QUAD_CMD                        0xF5 // Exit QPI QPIEX 

/* Default dummy clocks cycles */
#define DUMMY_CLOCK_CYCLES_READ              8
#define DUMMY_CLOCK_CYCLES_READ_QUAD         8

#define DUMMY_CLOCK_CYCLES_READ_DTR          8
#define DUMMY_CLOCK_CYCLES_READ_QUAD_DTR     8

/* End address of the QSPI memory */
#define QSPI_END_ADDR              (1 << QSPI_FLASH_SIZE)
