#include <cstring>
#include <algorithm>
#include "device.h"
#include "message.h"
#include "Storage.h"
#include "eepromcontrol.h"

#ifdef USE_SPI_FLASH
#include "Flash_S25FL.h"
#define MAX_SPI_FLASH_HEADERS 32

struct NorHeader : ResourceHeader {
  uint32_t address;
};

static NorHeader nor_index[MAX_SPI_FLASH_HEADERS];
#endif

void Storage::index(){
  size_t i=0;
#ifdef USE_FLASH
  resources[i].setHeader((ResourceHeader*)(EEPROM_PAGE_BEGIN));
  while(i<MAX_RESOURCE_HEADERS && !resources[i].isFree()){
    ResourceHeader* next = resources[i++].getNextHeader();
    resources[i].setHeader(next);
    // if(resources[i].isErased())
    //   resources[i].setHeader(next); // reuse current resource
    // else if(i+1<MAX_RESOURCE_HEADERS)
    //   resources[++i].setHeader(next);
  }
#endif
#ifdef USE_SPI_FLASH
  uint32_t address = 0;
  NorHeader* header = &nor_index[0];
  do{
    Flash_read(address, (uint8_t*)header, sizeof(ResourceHeader));
    header->address = address;
    resources[i].setHeader(header++);
    address += resources[i++].getTotalSize();
  }while(i<MAX_RESOURCE_HEADERS && !resources[i].isFree()
	 && header < &nor_index[MAX_SPI_FLASH_HEADERS]);
  // todo: also check address < sizeof flash
#endif
  resource_count = i;
}

size_t Storage::readResource(Resource* resource, uint8_t* data, size_t length){
  size_t ret = 0;
  if(resource){
    if(resource->isMemoryMapped()){
      size_t len = std::min(resource->getDataSize(), length);
      memcpy(data, resource->getData(), len);
      ret = len;
    }else{
#ifdef USE_SPI_FLASH
      uint32_t address = resource->getAddress();
      size_t len = std::min(resource->getDataSize(), length);
      Flash_read(address+sizeof(ResourceHeader), data, len);
      ret = len;
#endif
    }
  }
  return ret;    
}

// mark as deleted
bool Storage::erase(Resource* resource){
  bool status = false;  
  if(resource->isMemoryMapped()){
#ifdef USE_FLASH
    ResourceHeader* header = resource->getHeader();
    eeprom_unlock();
    eeprom_wait(); // clear flash flags
#ifdef STM32H743xx
    ResourceHeader copy = *header;
    copy->magic = 0x00000000;
    status = eeprom_write_block((uint32_t)header, copy, sizeof(ResourceHeader));
#else
    status = eeprom_write_word((uint32_t)header, 0x00000000);
#endif
    eeprom_lock();
#endif
  }else{
#ifdef USE_SPI_FLASH
    uint32_t address = resource->getAddress();
    resource->getHeader()->magic = 0x00000000;
    Flash_write(address, (uint8_t*)resource->getHeader(), 4); // write new magic
#endif
  }
  return status;
}

/**
 * Get patch or resource with the given name
 */
Resource* Storage::getResource(const char* name){
  for(size_t i=0; i<resource_count; ++i){
    if((resources[i].isValid())
       && strcmp(name, resources[i].getName()) == 0)
      return &resources[i];
  }
  return NULL;
}

Resource* Storage::getFreeResource(uint32_t flags){
  bool mapped = flags & STORAGE_MEMORY_MAPPED;
  for(size_t i=0; i<MAX_RESOURCE_HEADERS; ++i)
    if(resources[i].isFree() && mapped == resources[i].isMemoryMapped())
      return &resources[i]; // todo: check there's enough space
  return NULL;
}

// erase entire allocated FLASH memory
void Storage::erase(uint32_t flags){
  if(flags & STORAGE_MEMORY_MAPPED){  
#ifdef USE_FLASH
    uint32_t page = EEPROM_PAGE_BEGIN;
    eeprom_unlock();
    while(page < EEPROM_PAGE_END){
      eeprom_erase(page);
      page += EEPROM_PAGE_SIZE;
    }
    eeprom_lock();
#endif
  }else{
#ifdef USE_SPI_FLASH
    Flash_BulkErase();
#endif
  }
}

void Storage::defrag(void* buffer, size_t size, uint32_t flags){
  uint8_t* ptr = (uint8_t*)buffer;
  if(getDeletedSize() > 0 && getWrittenSize() > 0){
    uint32_t offset = 0;
    for(uint8_t i=0; i<resource_count && offset<size; ++i){
      if(resources[i].isValid()){
	readResource(&resources[i], ptr+offset,  resources[i].getTotalSize());
	offset += resources[i].getTotalSize();
      }
    }
    if(flags & STORAGE_MEMORY_MAPPED){
      erase(flags);
      eeprom_unlock();
      eeprom_write_block(EEPROM_PAGE_BEGIN, buffer, offset);
      eeprom_lock();
    }else{
      erase(flags);
  // Flash_erase(address, ERASE_64KB);
      Flash_write(0, (uint8_t*)buffer, offset);
    }
    index();
  }
}

// assume 'data' and 'length' already include ResourceHeader
size_t Storage::writeResource(const char* name, uint8_t* data, size_t length, uint32_t flags){
// size_t Storage::writeResource(Resource* resource, uint32_t flags){
  Resource* resource = getResource(name);
  size_t ret = 0;
  if(resource)
    erase(resource); // mark as deleted
  resource = getFreeResource(flags);

  size_t capacity = 0;
  if(resource && resource->isMemoryMapped())
    capacity = EEPROM_PAGE_END - (uint32_t)resource->getData();
  else if(resource)
    capacity = NOR_FLASH_SIZE - resource->getAddress() - sizeof(ResourceHeader);

  extern char _EXTRAM, _EXTRAM_SIZE;
  if(length > capacity){
    defrag(data + length, _EXTRAM_SIZE - length, flags);
  }
  
  // 1. if exists, erase
  // 2. if no space, defrag
  // 3. write
  // 4. verify
  
#ifdef USE_SPI_FLASH
  if(!(flags & STORAGE_MEMORY_MAPPED)){
    uint32_t address = resource->getAddress();
    Flash_write(address, data, length);
    // todo: read back and verify
    return length;
  }
#endif
  eeprom_unlock();
  eeprom_wait();
  int status = eeprom_write_block((uint32_t)resource->getData(), data, length);
  eeprom_lock();
  if(status){
    error(FLASH_ERROR, "Flash write failed");
    return 0;
  }
  if(length != resource->getTotalSize()){
    error(FLASH_ERROR, "Size verification failed");
    return 0;
  }
  if(memcmp(data, resource->getData(), length) != 0){
    error(FLASH_ERROR, "Data verification failed");
    return 0;
  }
  return length;
}
