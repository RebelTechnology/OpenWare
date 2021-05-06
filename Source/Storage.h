#ifndef __Storage_h__
#define __Storage_h__

#include <stdint.h>
#include <cstddef>
#include "device.h"
#include "Resource.h"

class Storage {
private:
  Resource resources[MAX_RESOURCE_HEADERS];
  size_t resource_count = 0;
  Resource* getFreeResource(uint32_t flags);
  bool verifyData(Resource* resource, void* data, size_t length);
public:
  Storage(){}
  void init();
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
  Resource* getResourceByName(const char* name);  
  Resource* getResourceBySlot(uint8_t slot);  
  size_t readResource(Resource* resource, void* data, size_t length);
  size_t writeResource(ResourceHeader* header);
  size_t writeResource(const char* name, uint8_t* data, size_t datasize, uint32_t flags);
  size_t writeResourceHeader(uint8_t* dest, const char* name, size_t datasize, uint32_t flags);
  bool eraseResource(uint8_t slot);
  bool eraseResource(const char* name);
  bool eraseResource(Resource* resource);
  void erase(uint32_t flags);

  size_t getTotalAllocatedSize(uint32_t flags = RESOURCE_MEMORY_MAPPED | RESOURCE_PORT_MAPPED);    
  /** 
   * returns bytes used by written and deleted blocks
   */
  size_t getTotalUsedSize(uint32_t flags = RESOURCE_MEMORY_MAPPED | RESOURCE_PORT_MAPPED){
    size_t size = 0;
    for(size_t i=0; i<resource_count; ++i)
      if(!resources[i].isFree() && resources[i].flagsContain(flags))
	size += resources[i].getTotalSize();
    return size;
  }
  size_t getDeletedSize(uint32_t flags = RESOURCE_MEMORY_MAPPED | RESOURCE_PORT_MAPPED){
    // returns bytes used by deleted blocks
    size_t size = 0;
    for(size_t i=0; i<resource_count; ++i)
      if(resources[i].isErased() && resources[i].flagsContain(flags))
	size += resources[i].getTotalSize();
    return size;
  }
  size_t getWrittenSize(uint32_t flags = RESOURCE_MEMORY_MAPPED | RESOURCE_PORT_MAPPED){
    return getTotalUsedSize(flags) - getDeletedSize(flags);
  }
  size_t getFreeSize(uint32_t flags = RESOURCE_MEMORY_MAPPED | RESOURCE_PORT_MAPPED){
    return getTotalAllocatedSize(flags) - getTotalUsedSize(flags);
  }
};

extern Storage storage;

#endif // __Storage_h__
