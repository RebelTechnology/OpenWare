#ifndef __Resource_h__
#define __Resource_h__

#include <stdint.h>
#include <cstddef>
#include <cstring>
#include "ResourceHeader.h"

#define RESOURCE_SYSTEM_RESOURCE 0x0100 // System settings
#define RESOURCE_USER_PATCH      0x0200 // User patch
#define RESOURCE_MEMORY_MAPPED   0x0400 // Internal MCU flash memory
#define RESOURCE_PORT_MAPPED     0x0800 // External NOR flash

#define RESOURCE_VALID_MAGIC     0xDADADEED
#define RESOURCE_ERASED_MAGIC    0xDADA0000
#define RESOURCE_FREE_MAGIC      0xffffffff

class Resource {
private:
  ResourceHeader* header;
public:
  Resource() : header(NULL){}
  Resource(ResourceHeader* header) : header(header){}
  bool isValid(){
    return header && header->magic == RESOURCE_VALID_MAGIC;
  }
  bool isErased(){
    return header && header->magic == RESOURCE_ERASED_MAGIC;
  }
  bool isFree(){
    return header && header->magic == RESOURCE_FREE_MAGIC;
  }
  bool isPatch(){
    return header->flags & RESOURCE_USER_PATCH;
  }
  bool isMemoryMapped(){
    return header->flags & RESOURCE_MEMORY_MAPPED;
  }
  bool isSystemResource(){
    return header->flags & RESOURCE_SYSTEM_RESOURCE;
  }
  /*
   * Returns true if resource only has flags that are set in @param mask
   */
  bool flagsMatch(uint32_t mask){
    return (header->flags & mask) == header->flags;
  }
  /*
   * Returns true if resource has at least those flags that are set in @param mask
   */
  bool flagsContain(uint32_t mask){
    return (header->flags & mask) == mask;
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
  void setName(const char* name){
    strncpy(header->name, name, sizeof(header->name));
  }
  void setHeader(ResourceHeader* header){
    this->header = header;
  }
  uint32_t getFlags(){
    return header->flags;
  }
  ResourceHeader* getHeader(){
    return header;
  }
  ResourceHeader* getNextHeader(){
    return (ResourceHeader*)(((uint8_t*)header)+getTotalSize());
  }
};

#endif // __Resource_h__
