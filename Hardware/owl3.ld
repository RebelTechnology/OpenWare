/* Entry Point */
ENTRY(Reset_Handler)

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size  = 0x200; /* required amount of heap  */
_Min_Stack_Size = 0x200; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
/* DTCMRAM  (rwx)   : ORIGIN = 0x20000000, LENGTH = 128K */
/* RAM_D1   (rwx)   : ORIGIN = 0x24000000, LENGTH = 512K */
/* RAM_D2   (rwx)   : ORIGIN = 0x30000000, LENGTH = 288K */
/* RAM_D3   (rwx)   : ORIGIN = 0x38000000, LENGTH = 64K */
/* ITCMRAM  (rwx)   : ORIGIN = 0x00000000, LENGTH = 64K */
/* FLASH    (rx)    : ORIGIN = 0x08000000, LENGTH = 2048K */
DTCMRAM  (rwx)   : ORIGIN = 0x20000000, LENGTH = 48K
PATCHRAM (rwx)   : ORIGIN = 0x2000c000, LENGTH = 80K
PLUSRAM  (rwx)   : ORIGIN = 0x24000000, LENGTH = 512K /* D1 RAM */
DMA_D2   (rwx)   : ORIGIN = 0x30000000, LENGTH = 64K
HEAP_D2  (rwx)   : ORIGIN = 0x30010000, LENGTH = 224K
RAM_D3   (rwx)   : ORIGIN = 0x38000000, LENGTH = 64K
ITCMRAM  (rwx)   : ORIGIN = 0x00000000, LENGTH = 4K
CCMRAM   (rwx)   : ORIGIN = 0x00001000, LENGTH = 60K
EXTRAM   (rwx)   : ORIGIN = 0xD0000000, LENGTH = 8M
/* BOOTLOADER (rx)  : ORIGIN = 0x08000000, LENGTH = 0K */
/* FLASH   (rx)     : ORIGIN = 0x08000000, LENGTH = 512K */
BOOTLOADER (rx)  : ORIGIN = 0x08000000, LENGTH = 128K
FLASH   (rx)     : ORIGIN = 0x08020000, LENGTH = 384K
STORAGE (rx)     : ORIGIN = 0x08080000, LENGTH = 512K  /* Assuming 1Mb FLASH */
/* STORAGE (rx)     : ORIGIN = 0x08080000, LENGTH = 1536K /\* Assuming 2Mb FLASH *\/ */
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    PROVIDE ( _ISR_VECTOR = . );
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
    PROVIDE ( _ISR_VECTOR_END = . );
  } >FLASH
  _ISR_VECTOR_SIZE = _ISR_VECTOR_END - _ISR_VECTOR;

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : 
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >DTCMRAM AT> FLASH

  
  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;

    end = .;           /* system heap, used e.g. by printf in DEBUG builds */
    . = . + _Min_Heap_Size;
  } >DTCMRAM

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >ITCMRAM
  _estack = ORIGIN(ITCMRAM)+LENGTH(ITCMRAM);

  /* CCM section, vars must be located here explicitly */
  /* Example: int foo __attribute__ ((section (".ccmdata"))); */
  .ccmdata (NOLOAD) :
  {
    *(.ccmdata)
    . = ALIGN(8);
    PROVIDE (_CCMRAM = .);
  } >CCMRAM
  _CCMRAM_END = ORIGIN(CCMRAM) + LENGTH(CCMRAM);
  _CCMRAM_SIZE = _CCMRAM_END - _CCMRAM;

  /* Bootloader end will be used for finding bootloader token */
  .bootloader (NOLOAD) :
  {
    PROVIDE (_BOOTLOADER = .);
  } >BOOTLOADER
  _BOOTLOADER_END = ORIGIN(BOOTLOADER) + LENGTH(BOOTLOADER);

  /* External memory, vars must be located here explicitly */
  /* Example: int foo __attribute__ ((section (".extdata"))); */
  .extdata (NOLOAD) :
  {
    /* *(.extdata) */
    /* . = ALIGN(8); */
    PROVIDE ( _EXTRAM = . );
  /* } >RAM_D1 */
  /* _EXTRAM_END = ORIGIN(RAM_D1) + LENGTH(RAM_D1); */
  /* _EXTRAM_SIZE = LENGTH(RAM_D1); */
  } >EXTRAM
  _EXTRAM_END = ORIGIN(EXTRAM) + LENGTH(EXTRAM);
  _EXTRAM_SIZE = LENGTH(EXTRAM);

  .patchram (NOLOAD) :
  {
    PROVIDE (_PATCHRAM = .);
  } >PATCHRAM
  _PATCHRAM_END = ORIGIN(PATCHRAM) + LENGTH(PATCHRAM);
  _PATCHRAM_SIZE = LENGTH(PATCHRAM);

  .plusram (NOLOAD) :
  {
    PROVIDE (_PLUSRAM = .);
  } >PLUSRAM
  _PLUSRAM_END = ORIGIN(PLUSRAM) + LENGTH(PLUSRAM);
  _PLUSRAM_SIZE = LENGTH(PLUSRAM);

  .heapd2 (NOLOAD) :
  {
    PROVIDE (_HEAP_D2 = .);
  } >HEAP_D2
  _HEAP_D2_END = ORIGIN(HEAP_D2) + LENGTH(HEAP_D2);
  _HEAP_D2_SIZE = LENGTH(HEAP_D2);

  .dmadata (NOLOAD) :
  {
    PROVIDE (_DMA_DATA = .);
    *(.dmadata)
  } >DMA_D2

  .flash_storage (NOLOAD) :
  {
    PROVIDE (_FLASH_STORAGE_BEGIN = .);
  } >STORAGE
  _FLASH_STORAGE_END = ORIGIN(STORAGE) + LENGTH(STORAGE);  
  _FLASH_STORAGE_SIZE = LENGTH(STORAGE);

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}


