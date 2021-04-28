#include <stdint.h>
#include <cstddef>
#include "ResourceHeader.h"

extern char _FLASH_STORAGE_BEGIN;
extern char _FLASH_STORAGE_END;
#define EEPROM_PAGE_BEGIN ((uint32_t)&_FLASH_STORAGE_BEGIN)
#define EEPROM_PAGE_END   ((uint32_t)&_FLASH_STORAGE_END)
#define EEPROM_PAGE_SIZE  (128*1024)

#define MAX_RESOURCE_HEADERS 40
#define USE_SPI_FLASH
#define USE_FLASH
#define NOR_FLASH_SIZE (8*1024*1024)

class Resource {
private:
  ResourceHeader* header = NULL;
public:
  bool isPatch(){
    return header && header->magic == 0xDADAC0DE;
  }
  bool isResource(){
    return header && header->magic == 0xDADADEED;
  }
  bool isValid(){
    return isPatch() || isResource();
  }
  bool isErased(){
    return header && header->magic == 0xDADA0000;
  }
  bool isFree(){
    return header && header->magic == 0xffffffff;
  }
  bool isMemoryMapped(){
    return uint32_t(header) >= EEPROM_PAGE_BEGIN && uint32_t(header) < EEPROM_PAGE_END;
  }
  /**
   * Get data pointer to memory-mapped resource
   */
  uint8_t* getData(){
    return ((uint8_t*)header)+sizeof(ResourceHeader);
  }
  /**
   * Get address of non-memory-mapped resource
   */
  uint32_t getAddress(){
    return *(uint32_t*)getData();
  }
  size_t getDataSize(){
    return header->size;
  }
  // get size, including header, aligned to 32 or 256 bytes
  size_t getTotalSize(){
    if(isMemoryMapped())
      return (header->size+sizeof(ResourceHeader)+31) & ~31;
    else
      return (header->size+sizeof(ResourceHeader)+255) & ~255;
  }
  const char* getName(){
    return header->name;
  }
  void setHeader(ResourceHeader* header){
    this->header = header;
  }
  ResourceHeader* getHeader(){
    return header;
  }
  ResourceHeader* getNextHeader(){
    return (ResourceHeader*)(((uint8_t*)header)+getTotalSize());
  }
};

#define STORAGE_MEMORY_MAPPED 0x01

class Storage {
private:
  Resource resources[MAX_RESOURCE_HEADERS];
  size_t resource_count = 0;
  Resource* getFreeResource(uint32_t flags);
public:
  void index();
  void defrag(void* buffer, size_t size, uint32_t flags);
  size_t getNumberOfResources(){
    return resource_count;
  }
  Resource* getResource(size_t index){
    if(index < resource_count)
      return &resources[index];
    return NULL;
  }
  Resource* getResource(const char* name);  
  size_t readResource(Resource* resource, uint8_t* data, size_t length);
  size_t writeResource(const char* name, uint8_t* data, size_t length, uint32_t flags);
  bool erase(Resource* resource);
  void erase(uint32_t flags);

  size_t getTotalUsedSize(){
    // returns bytes used by written and deleted blocks
    size_t size = 0;
    for(size_t i=0; i<resource_count; ++i)
      // if(resources[i].isPatch() || resources[i].isResource())
      if(!resources[i].isFree())
	size += resources[i].getTotalSize();
    return size;
  }
  size_t getDeletedSize(){
    // returns bytes used by deleted blocks
    size_t size = 0;
    for(size_t i=0; i<resource_count; ++i)
      if(resources[i].isErased())
	size += resources[i].getTotalSize();
    return size;
  }
  size_t getWrittenSize(){
    return getTotalUsedSize() - getDeletedSize();
  }
};
