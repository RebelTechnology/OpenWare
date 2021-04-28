#include <cstring>
#include <algorithm>
#include "device.h"
#include "message.h"
#include "Storage.h"
#include "eepromcontrol.h"

extern char _FLASH_STORAGE_BEGIN;
extern char _FLASH_STORAGE_END;
#define INTERNAL_FLASH_BEGIN ((uint32_t)&_FLASH_STORAGE_BEGIN)
#define INTERNAL_FLASH_END   ((uint32_t)&_FLASH_STORAGE_END)
#define INTERNAL_FLASH_SIZE  (INTERNAL_FLASH_END - INTERNAL_FLASH_BEGIN)

#ifdef USE_SPI_FLASH
#include "Flash_S25FL.h"
#define MAX_SPI_FLASH_HEADERS 32

struct NorHeader : ResourceHeader {
  uint32_t address;
};

static NorHeader nor_index[MAX_SPI_FLASH_HEADERS];
#endif

Storage storage;

void Storage::index(){
  memset(resources, 0, sizeof(resources));
  size_t i = 0;
#ifdef USE_FLASH
  resources[i].setHeader((ResourceHeader*)(INTERNAL_FLASH_BEGIN));
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

size_t Storage::readResource(Resource* resource, void* data, size_t length){
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
      Flash_read(address+sizeof(ResourceHeader), (uint8_t*)data, len);
      ret = len;
#endif
    }
  }
  return ret;    
}

// mark as deleted
bool Storage::eraseResource(Resource* resource){
  bool status = false;  
  if(resource->isMemoryMapped()){
#ifdef USE_FLASH
    ResourceHeader* header = resource->getHeader();
    eeprom_unlock();
    eeprom_wait(); // clear flash flags
#ifdef STM32H743xx
    ResourceHeader copy = *header;
    copy->magic = RESOURCE_ERASED_MAGIC;
    status = eeprom_write_block((uint32_t)header, copy, sizeof(ResourceHeader));
#else
    status = eeprom_write_word((uint32_t)header, RESOURCE_ERASED_MAGIC);
#endif
    eeprom_lock();
#endif
  }else{
#ifdef USE_SPI_FLASH
    uint32_t address = resource->getAddress();
    resource->getHeader()->magic = RESOURCE_ERASED_MAGIC;
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
  for(size_t i=0; i<MAX_RESOURCE_HEADERS; ++i)
    if(resources[i].isFree() && resources[i].flagsContain(flags))
      return &resources[i]; // todo: check there's enough space
  return NULL;
}

// erase entire FLASH memory
void Storage::erase(uint32_t flags){
#ifdef USE_FLASH
  // tbd: keep system resources
  if(flags & RESOURCE_MEMORY_MAPPED){
    eeprom_unlock();
    eeprom_erase(INTERNAL_FLASH_BEGIN, INTERNAL_FLASH_SIZE);
    eeprom_lock();
  }
#endif
#ifdef USE_SPI_FLASH
  if(flags & RESOURCE_PORT_MAPPED){
    Flash_BulkErase();
  }
#endif
}

void Storage::defrag(void* buffer, size_t size, uint32_t flags){
  uint8_t* ptr = (uint8_t*)buffer;
  if(getWrittenSize(flags) > size)
    debugMessage("Not enough RAM to defrag");

  uint32_t offset = 0;
  for(uint8_t i=0; i<resource_count; ++i){
    if(resources[i].isValid() && resources[i].flagsContain(flags)){
      if(offset+resources[i].getTotalSize() < size){
	readResource(&resources[i], ptr+offset,  resources[i].getTotalSize());
	offset += resources[i].getTotalSize();
      }
    }
  }
  if(flags & RESOURCE_MEMORY_MAPPED){
    erase(flags);
    eeprom_unlock();
    eeprom_write_block(INTERNAL_FLASH_BEGIN, buffer, offset);
    eeprom_lock();
  }else{
    erase(flags);
    // Flash_erase(address, ERASE_64KB);
    Flash_write(0, (uint8_t*)buffer, offset);
  }
  index();
}

// Resource* Storage::createResource(const char* name, size_t length, uint32_t flags){
//   Resource resource = getFreeResource(flags);
//   if(resource){
//     resource->setName(name);
//     resource->getHeader()->length = length;
//     resource->getHeader()->flags = flags;
//   }
//   return resource;
// }

size_t Storage::writeResourceHeader(uint8_t* dest, const char* name, size_t size, uint32_t flags){
  ResourceHeader header;
  header.magic = RESOURCE_VALID_MAGIC;
  header.size = size;
  strncpy(header.name, name, sizeof(header.name));
  header.flags = flags;
  memcpy(dest, &header, sizeof(header));
  return sizeof(header);
}

// assume 'data' and 'length' already include ResourceHeader
size_t Storage::writeResource(const char* name, uint8_t* data, size_t length, uint32_t flags){
  Resource* dest = getResource(name);
  if(dest)
    eraseResource(dest); // mark as deleted
  dest = getFreeResource(flags);
  writeResourceHeader(data, name, length, flags);  
  size_t capacity = getFreeSize(flags);
  if(length > capacity){ // assuming 'data' is located in extram
    extern char _EXTRAM, _EXTRAM_SIZE;
    defrag(data + length, _EXTRAM_SIZE - length, flags);
  }
  
  // 1. if exists, erase
  // 2. if no space, defrag
  // 3. write
  // 4. verify
  // 5. rebuild index
  
#ifdef USE_SPI_FLASH
  if(!(flags & RESOURCE_MEMORY_MAPPED)){
    uint32_t address = dest->getAddress();
    Flash_write(address, data, length);
    // todo: read back and verify
    return length;
  }
#endif
  eeprom_unlock();
  eeprom_wait();
  int status = eeprom_write_block((uint32_t)dest->getData(), data, length);
  eeprom_lock();
  if(status){
    error(FLASH_ERROR, "Flash write failed");
    return 0;
  }
  if(length != dest->getTotalSize()){
    error(FLASH_ERROR, "Size verification failed");
    return 0;
  }
  if(memcmp(data, dest->getData(), length) != 0){
    error(FLASH_ERROR, "Data verification failed");
    return 0;
  }
  // rebuild index
  index();
  return length;
}

size_t Storage::getTotalAllocatedSize(uint32_t flags){
  size_t total = 0;
#ifdef USE_FLASH
  if(flags & RESOURCE_MEMORY_MAPPED)
    total += INTERNAL_FLASH_SIZE;
#endif
#ifdef USE_SPI_FLASH
  if(flags & RESOURCE_PORT_MAPPED)
    total += EXTERNAL_FLASH_SIZE;
#endif
  return total;
}
