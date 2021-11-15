
#include "flash.h"

static int cmp(void* a, void* b, size_t len){
  uint8_t* lhs = (uint8_t*)a;
  uint8_t* rhs = (uint8_t*)b;
  while(len--){
    if(*lhs++ != *rhs++)
      return -1;
  }
  return 0;
}

int setError(const char* msg){
  error(CONFIG_ERROR, msg);
  return -1;
}
int flashExternal(uint8_t* source, size_t size, int sector, uint32_t address){
  flash_init();
  int memloc = sector*128*1024;
  // size_t size = getSize();
  // uint8_t* source = getData();
  // uint32_t address = QSPI_FLASH_BASE+memloc;
  if(address < QSPI_FLASH_BASE || address+size > QSPI_FLASH_BASE+QSPI_FLASH_SIZE)
    return setError("Invalid QSPI FLASH sector or size");
  if(flash_erase(memloc, size) != 0)
    return setError("QSPI Flash erase failed");
  flash_wait();
  if(flash_write_block(memloc, source, size) != 0)
    return setError("QSPI Flash write failed");
  flash_wait();
  flash_memory_map(122);
  if(cmp((void*)address, source, size) != 0)
    return setError("QSPI FLASH verify failed");
  return 0;
}

extern "C" {
  void HAL_QSPI_ErrorCallback(QSPI_HandleTypeDef *hqspi){
    debugMessage("QSPI Error", (int)hqspi->ErrorCode);
  }
  void HAL_QSPI_AbortCpltCallback(QSPI_HandleTypeDef *hqspi){
    debugMessage("QSPI Abort", (int)hqspi->ErrorCode);
  }
}


const char flashdata[] = "This is the new data.";
 
#ifdef DEBUG
#include <cstdio>
#endif

void setup(){
  printf("QSPI flash test: %s.\n", flashdata);

  flashExternal((uint8_t*)flashdata, sizeof(flashdata), 0, QSPI_FLASH_BASE);

  // flash_reset();
  flash_init();
  int fs1 = flash_status();
  flash_memory_map(-122);
  int fs2 = flash_status();
  // flash_memory_map(-444);
#ifdef DEBUG
  printf("FLASH %s %d %d.\n", (const char*)QSPI_FLASH_BASE, fs1, fs2);
#endif
}
