/*******************************************************************************
* BlueNRG-1 generic linker file for ATOLLIC
* Main linker variables to control it are:
*
* MEMORY_FLASH_APP_SIZE: define the size of the application in case not all the flash is needed.
* Default value is: 156KB when no OTA firmware upgrade is supported
*
* MEMORY_FLASH_APP_OFFSET: define the offset of the application.
* Default value is: 0 offset
*
* MEMORY_RAM_APP_OFFSET: define the offset in RAM from which sections can be
* allocated.
* Default value is: 0x02CC
* 
* ST_OTA_HIGHER_APPLICATION: When defined application is built for OTA support
* in the higher part of memory
*
* ST_OTA_LOWER_APPLICATION: When defined application is built for OTA support
* in the lower part of memory
*
* ST_USE_OTA_SERVICE_MANAGER_APPLICATION: When defined application is built for
* OTA firmware upgrade support with separated application for firmware upgrade
*
*******************************************************************************/

/*******************************************************************************
* Memory Definitions
*******************************************************************************/
/*
BlueNRG-1 memory map
+-----------------------+ 0x20005FFF
|  RAM (24K)            |
+-----------------------+ 0x20000000
|                       |
|                       |
+-----------------------+ 0x10067FFF
|                       |
|  FLASH (160K)         |
+-----------------------+ 0x10040000
|                       |
+-----------------------| 0x100007FF
|   ROM (2K)            |
+-----------------------+ 0x10000000
*/


_MEMORY_RAM_BEGIN_   =  0x20000000; 
_MEMORY_RAM_SIZE_    =  0x6000;           /* 24KB  */
_MEMORY_RAM_END_     =  0x20005FFF; 

_MEMORY_FLASH_BEGIN_ = 0x10040000;  
_MEMORY_FLASH_SIZE_  = 0x28000;          /* 160KB */ 
_MEMORY_FLASH_END_   = 0x10067FFF;  

_MEMORY_ROM_BEGIN_   = 0x10000000;    
_MEMORY_ROM_SIZE_    = 0x800;             /* 2KB */ 
_MEMORY_ROM_END_     = 0x100007FF;  


/* Reserved for BTLE stack non volatile memory */
FLASH_NVM_DATASIZE   = (4*1024);


  /* This configuration is intended for application not supporting OTA firmware upgrade */
  /*
  BlueNRG-1 standard application memory map
  +-----------------------+ 0x20005FFF
  |  RAM (24K)            |
  +-----------------------+ 0x20000000
  |                       |
  |                       |
  +-----------------------+ 0x10068000
  |                       |
  |  NVM(4K)              |
  +-----------------------+ 0x10067000
  |                       |
  |  User app (156K)      |
  +-----------------------+ 0x10040000
  |                       |
  +-----------------------| 0x100007FF
  |   ROM (2K)            |
  +-----------------------+ 0x10000000
*/

MEMORY_FLASH_APP_OFFSET = DEFINED(MEMORY_FLASH_APP_OFFSET) ? (MEMORY_FLASH_APP_OFFSET) : (0) ;
MEMORY_FLASH_APP_SIZE = DEFINED(MEMORY_FLASH_APP_SIZE) ? (MEMORY_FLASH_APP_SIZE) : ( _MEMORY_FLASH_SIZE_ - FLASH_NVM_DATASIZE - MEMORY_FLASH_APP_OFFSET);
MEMORY_RAM_APP_OFFSET = DEFINED(MEMORY_RAM_APP_OFFSET) ? (MEMORY_RAM_APP_OFFSET) : (0x2CC) ;
RESET_SM_MANAGER_SIZE = 0;
RESET_MANAGER_SIZE = 0;



/* 
   *****************************
   * ST_OTA_HIGHER_APPLICATION *
   *****************************
*/
   /* This configuration is intended for application supporting OTA firmware upgrade with 2-app scheme (app in the upper part of memory map) */
  /*
     BlueNRG-1 OTA firmware upgrade support for higher application memory map
     +-----------------------+ 0x20005FFF
     |  RAM (24K-4)          |
     +-----------------------+ 0x20000004
     |                       |
     |                       |
     +-----------------------+ 0x10068000
     |                       |
     |  NVM(4K)              |
     +-----------------------+ 0x10066800
     |                       |
     |  Higher app (76K)     |
     +-----------------------+ 0x10053800
     |                       |
     |  Lower app (76K)      |
     +-----------------------| 0x10040800
     |   Reset Manager (2K)  |
     +-----------------------+ 0x10040000
     |                       |
     +-----------------------| 0x100007FF
     |   ROM (2K)            |
     +-----------------------+ 0x10000000
  */


RESET_MANAGER_SIZE = DEFINED(ST_OTA_HIGHER_APPLICATION) ? 0x800 : RESET_MANAGER_SIZE ;
MEMORY_FLASH_APP_SIZE = DEFINED(ST_OTA_HIGHER_APPLICATION) ? ((_MEMORY_FLASH_SIZE_ - RESET_MANAGER_SIZE - FLASH_NVM_DATASIZE - 2048) / 2) : MEMORY_FLASH_APP_SIZE ;
MEMORY_FLASH_APP_OFFSET = DEFINED(ST_OTA_HIGHER_APPLICATION) ? (RESET_MANAGER_SIZE + MEMORY_FLASH_APP_SIZE) : MEMORY_FLASH_APP_OFFSET ;


/* 
   *****************************
   * ST_OTA_LOWER_APPLICATION *
   *****************************
*/
  /* This configuration is intended for application supporting OTA firmware upgrade with 2-app scheme (app in the lower part of memory map) */
  /*
     BlueNRG-1 OTA firmware upgrade support for lower application memory map
     +-----------------------+ 0x20005FFF
     |  RAM (24K-4)          |
     +-----------------------+ 0x20000004
     |                       |
     |                       |
     +-----------------------+ 0x10068000
     |                       |
     |  NVM(4K)              |
     +-----------------------+ 0x10066800
     |                       |
     |  Higher app (76K)     |
     +-----------------------+ 0x10053800
     |                       |
     |  Lower app (76K)      |
     +-----------------------| 0x10040800
     |   Reset Manager (2K)  |
     +-----------------------+ 0x10040000
     |                       |
     +-----------------------| 0x100007FF
     |   ROM (2K)            |
     +-----------------------+ 0x10000000
  */

RESET_MANAGER_SIZE = DEFINED(ST_OTA_LOWER_APPLICATION) ? 0x800 : RESET_MANAGER_SIZE ;
MEMORY_FLASH_APP_SIZE = DEFINED(ST_OTA_LOWER_APPLICATION) ? ((_MEMORY_FLASH_SIZE_ - RESET_MANAGER_SIZE - FLASH_NVM_DATASIZE - 2048) / 2) : MEMORY_FLASH_APP_SIZE ;
MEMORY_FLASH_APP_OFFSET = DEFINED(ST_OTA_LOWER_APPLICATION) ? (RESET_MANAGER_SIZE) : MEMORY_FLASH_APP_OFFSET ;


/* 
   ******************************************
   * ST_USE_OTA_SERVICE_MANAGER_APPLICATION *
   ******************************************
*/
  /* This configuration is intended for application supporting OTA firmware upgrade with independent OTA firmware upgrade service manager
  (app in the upper part of memory map) */
  /*
     BlueNRG-1 OTA firmware upgrade support for service manager application memory map
     +-----------------------+ 0x20005FFF
     |  RAM (24K-4)          |
     +-----------------------+ 0x20000004
     |                       |
     |                       |
     +-----------------------+ 0x10068000
     |                       |
     |  NVM(4K)              |
     +-----------------------+ 0x10067000
     |                       |
     |  User app (88K)       |
     +-----------------------+ 0x10051000
     |  OTA Service          |
     |  Manager (68K)        |
     +-----------------------+ 0x10040000
     |                       |
     +-----------------------| 0x100007FF
     |   ROM (2K)            |
     +-----------------------+ 0x10000000
  */

RESET_SM_MANAGER_SIZE = DEFINED(ST_USE_OTA_SERVICE_MANAGER_APPLICATION) ? 0x11000 : RESET_SM_MANAGER_SIZE ; /* Including OTA firmware upgrade service manager application */
MEMORY_FLASH_APP_SIZE = DEFINED(ST_USE_OTA_SERVICE_MANAGER_APPLICATION) ? (_MEMORY_FLASH_SIZE_ - RESET_SM_MANAGER_SIZE - FLASH_NVM_DATASIZE) : MEMORY_FLASH_APP_SIZE ;
MEMORY_FLASH_APP_OFFSET = DEFINED(ST_USE_OTA_SERVICE_MANAGER_APPLICATION) ? (RESET_SM_MANAGER_SIZE) : MEMORY_FLASH_APP_OFFSET ;


EXTERNAL_BLE_STACK_FLASH_OFFSET = DEFINED(EXTERNAL_BLE_STACK_FLASH_OFFSET) ? EXTERNAL_BLE_STACK_FLASH_OFFSET : 0;
cmd_call_table = _MEMORY_FLASH_BEGIN_ + 0xC0 + EXTERNAL_BLE_STACK_FLASH_OFFSET;
ev_call_table = 0x200002cc;


/* Entry Point */
ENTRY(RESET_HANDLER)


/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x0;      /* required amount of heap  */
_Min_Stack_Size = 0xC00; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
  REGION_RAM (xrw)         		: ORIGIN = _MEMORY_RAM_BEGIN_, LENGTH = _MEMORY_RAM_SIZE_
  REGION_FLASH_BOOTLOADER (rx)  : ORIGIN = _MEMORY_FLASH_BEGIN_, LENGTH = MEMORY_FLASH_APP_OFFSET
  REGION_FLASH (rx)        		: ORIGIN = _MEMORY_FLASH_BEGIN_ + MEMORY_FLASH_APP_OFFSET, LENGTH = MEMORY_FLASH_APP_SIZE
  REGION_NVM (rx)          		: ORIGIN = _MEMORY_FLASH_END_ + 1 - FLASH_NVM_DATASIZE, LENGTH = FLASH_NVM_DATASIZE
  REGION_ROM (rx)          		: ORIGIN = _MEMORY_ROM_BEGIN_, LENGTH = _MEMORY_ROM_SIZE_
}

/* Define output sections */
SECTIONS
{  
  /* The startup code goes first into FLASH */
  .bootloader (ORIGIN(REGION_FLASH_BOOTLOADER)) :
  {
    . = ALIGN(4);
    
    KEEP(*(.bootloader)) /* Startup code */
    
    . = ALIGN(4);
  } >REGION_FLASH_BOOTLOADER

  /* The startup code goes first into FLASH */
  .intvec (ORIGIN(REGION_FLASH)) :
  {
    . = ALIGN(4);
    
    KEEP(*(.intvec)) /* Startup code */
    
    . = ALIGN(4);
  } >REGION_FLASH


  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);

    KEEP(*(.cmd_call_table))
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(i.*)             /* i.* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    *(.constdata)
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */

    . = ALIGN(4);
     _etext = .;
  } >REGION_FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);
  /* used by the startup to initialize data */
  _sidata2 = LOADADDR(.ram_preamble);


  /* RAM preamble no init */
  .ram_preamble_noinit 0x20000000 (NOLOAD) :
  {
    . = ALIGN(4);
    . = 0x04 ; /* There is a waste of RAM here */
    KEEP(*(.ota_sw_activation))
  }  >REGION_RAM

  /* RAM preamble initialized */
  .ram_preamble 0x20000008 /*(NOLOAD)*/ :
  {
    _sdata2 = .;        /* create a global symbol at data start */
    /* This is used by the startup in order to initialize the .bss section */
    KEEP(*(.savedMSP))
    . = 0x04 ;
    KEEP(*(.wakeupFromSleepFlag))
    . = 0x08 ;
    KEEP(*(.app_base))
    . = 0x0C ;
    KEEP(*(.flash_sw_lock))
    . = 0x28 ;
    KEEP(*(.__blueflag_RAM))
      _edata2 = .;        /* create a global symbol at data end (.__crash_RAM is skipped since it must not be initialized) */
  }  >REGION_RAM AT> REGION_FLASH
    
  /* RAM preamble no init */
  .ram_preamble_noinit2 0x20000034 (NOLOAD) :
  {
    KEEP(*(.__crash_RAM))
  }  >REGION_RAM
  /* Uninitialized data section */
   
  .bss.blueRAM 0x200000C0 :
  {
    . = ALIGN(4);
    _sbssblue = .;         /* define a global symbol at .bss.blueRAM start */
    KEEP(*(.bss.__blue_RAM))
    . = ALIGN(4);
    _ebssblue = .;         /* define a global symbol at .bss.blueRAM end */
    
    . = 0x20c + MEMORY_RAM_APP_OFFSET - 0x2cc;
    
  } >REGION_RAM  
     
  .bss  :
  {
    . = ALIGN(4);
    _sbss = .;         /* define a global symbol at bss start */
    *(.bss.ev_call_table)
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
  } >REGION_RAM

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >REGION_RAM AT> REGION_FLASH
  
  /* Data section that will not be initialized to any value. */
  .noinit (NOLOAD):
  {
    . = ALIGN(4);
    *(.noinit)
    . = ALIGN(4);
  } >REGION_RAM

/**
* The last 2KB sector of FLASH is reserved for firmware to use. The BLE Host
* stores its security database in this area. The linker needs to make sure this
* 2KB sector is left empty.
*/
  BLOCK_STACKLIB_FLASH_DATA (_MEMORY_FLASH_END_ - FLASH_NVM_DATASIZE + 1) (NOLOAD) :
  {
    . = ALIGN(2048);
    
    KEEP(*(.noinit.stacklib_flash_data))
    KEEP(*(.noinit.stacklib_stored_device_id_data))
    
  } >REGION_NVM



  /* This is to emulate place at end of IAR linker */
  CSTACK (ORIGIN(REGION_RAM) + LENGTH(REGION_RAM) - _Min_Stack_Size) (NOLOAD) :
  {
    . = ALIGN(4);
    _estack = . + _Min_Stack_Size;         /* define a global symbol at bss end */
    . = ALIGN(4);
  } > REGION_RAM

  
  .rom_info (NOLOAD) :
  {
    . = ALIGN(4);
    KEEP(*(.rom_info)) 
    . = ALIGN(4);
  } >REGION_ROM
  
  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
