#include <cstring>
#include <algorithm>
#include "device.h"
#include "message.h"
#include "Storage.h"
#include "eepromcontrol.h"

#ifdef USE_SPI_FLASH
#include "Flash_S25FL.h"

extern char _FLASH_STORAGE_BEGIN;
extern char _FLASH_STORAGE_END;

struct NorHeader : ResourceHeader {
  uint32_t address;
};

static NorHeader nor_index[MAX_SPI_FLASH_HEADERS];
#endif

Storage storage;

void* findFirstFreeBlock(void* begin, void* end, uint32_t align){
  // search backwards for last block that is not written
  uint32_t* p = (uint32_t*)end;
  p -= 2*align/sizeof(uint32_t); // start at two alignment units from end
  while(p > begin && *p == RESOURCE_FREE_MAGIC)
    p -= align/sizeof(uint32_t);
  p += align/sizeof(uint32_t);
  p = (uint32_t*)((uint32_t)p & ~(align-1));
  if(*p == RESOURCE_FREE_MAGIC)
    return p;
  return NULL;
}

#ifdef USE_SPI_FLASH
uint32_t findFirstFreePage(uint32_t begin, uint32_t end, size_t pagesize){
  uint32_t page[pagesize/sizeof(uint32_t)];
  uint32_t address = end-pagesize;
  while(address > begin){
    Flash_read(address, (uint8_t*)page, pagesize);
    if(page[0] != RESOURCE_FREE_MAGIC ||
       memcmp(&page[0], &page[1], pagesize-sizeof(uint32_t)) != 0)
      break;
  }
  if(address > begin)
    return address+pagesize;
  return end;
}
#endif
  
void Storage::init(){
#ifdef USE_SPI_FLASH
  extern SPI_HandleTypeDef SPI_FLASH_HSPI;
  Flash_S25FL_init(&SPI_FLASH_HSPI);
  memset(nor_index, 0, sizeof(nor_index));
#endif
  index();
}

void Storage::index(){
  memset(resources, 0, sizeof(resources));
  size_t i = 0;
#ifdef USE_FLASH
  resources[i].setHeader((ResourceHeader*)(INTERNAL_FLASH_BEGIN));
  while(i<MAX_RESOURCE_HEADERS && resources[i].isUsed()){
    ResourceHeader* next = resources[i].getNextHeader();
    if(resources[i].isValid())
      i++;
    resources[i].setHeader(next);
  }
  if(!resources[i].isFree()){
    error(FLASH_ERROR, "Invalid flash resource");
    // set resource to point to first free block, or NULL
    void* p = findFirstFreeBlock(resources[i].getHeader(), (void*)INTERNAL_FLASH_END, 32);
    resources[i].setHeader((ResourceHeader*)p);
  }
#endif
#ifdef USE_SPI_FLASH
  uint32_t address = 0;
  NorHeader* header = &nor_index[0];
  header->address = address;
  resources[++i].setHeader(header);
  Flash_read(address, (uint8_t*)header, sizeof(ResourceHeader));
  while(i<MAX_RESOURCE_HEADERS && resources[i].isUsed() &&
	address < EXTERNAL_FLASH_SIZE &&
	header < &nor_index[MAX_SPI_FLASH_HEADERS]){
    address += resources[i].getTotalSize();
    if(resources[i].isValid())
      resources[++i].setHeader(++header);
    header->address = address;
    Flash_read(address, (uint8_t*)header, sizeof(ResourceHeader));
  }
  if(!resources[i].isFree()){
    error(FLASH_ERROR, "Invalid SPI resource");
    // set resource to point to first free block, or NULL
    address = findFirstFreePage(resources[i].getAddress(), EXTERNAL_FLASH_SIZE, 256);
    if(address < EXTERNAL_FLASH_SIZE)
      ((NorHeader*)resources[i].getHeader())->address = address;
    else
      resources[i].setHeader(NULL);
  }
#endif
  resource_count = i+1;
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

bool Storage::eraseResource(uint8_t slot){
  Resource* resource = getResourceBySlot(slot);
  if(resource)
    return eraseResource(resource);
  return false;
}

bool Storage::eraseResource(const char* name){
  Resource* resource = getResourceByName(name);
  if(resource)
    return eraseResource(resource);
  return false;
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
Resource* Storage::getResourceByName(const char* name){
  for(size_t i=0; i<resource_count; ++i){
    if(resources[i].isValid()
       && strcmp(name, resources[i].getName()) == 0)
      return &resources[i];
  }
  return NULL;
}

/**
 * Get patch or resource with the given slot number
 */
Resource* Storage::getResourceBySlot(uint8_t slot){
  for(size_t i=0; i<resource_count; ++i){
    if(resources[i].isValid() &&
       (resources[i].getFlags() & 0xff) == slot)
      return &resources[i];
  }
  return NULL;
}

/**
 * Returns first free resource in either memory-mapped or port-mapped storage,
 * depending on @param flags.
 * @return NULL if no free resource found.
 */
Resource* Storage::getFreeResource(uint32_t flags){
  bool mapped = flags & RESOURCE_MEMORY_MAPPED;
  for(size_t i=0; i<MAX_RESOURCE_HEADERS; ++i)
    if(resources[i].isFree() && resources[i].isMemoryMapped() == mapped)
      return &resources[i];
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
  index();
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
  erase(flags);
  if(flags & RESOURCE_MEMORY_MAPPED){
#ifdef USE_FLASH
    eeprom_unlock();
    eeprom_write_block(INTERNAL_FLASH_BEGIN, buffer, offset);
    eeprom_lock();
#endif
  }else{
#ifdef USE_SPI_FLASH
    // Flash_erase(address, ERASE_64KB);
    Flash_write(0, (uint8_t*)buffer, offset);
#endif
  }
  index();
}

size_t Storage::writeResourceHeader(uint8_t* dest, const char* name, size_t datasize, uint32_t flags){
  ResourceHeader header;
  header.magic = RESOURCE_VALID_MAGIC;
  header.size = datasize;
  strncpy(header.name, name, sizeof(header.name));
  header.flags = flags;
  memcpy(dest, &header, sizeof(header));
  return sizeof(header);
}

// data length must already include space for ResourceHeader
size_t Storage::writeResource(const char* name, uint8_t* data, size_t datasize, uint32_t flags){
  writeResourceHeader(data, name, datasize, flags);  
  return writeResource((ResourceHeader*)data);
}

/**
 * Write new or overwrite existing resource. Steps:
 * 1. if it exists, erase (indexed by slot number, if present, or name)
 * 2. if there is not enough remaining storage space, defrag
 * 3. flash header and data
 * 4. verify written data
 * 5. rebuild index
 */
size_t Storage::writeResource(ResourceHeader* header){
  size_t length = header->size+sizeof(ResourceHeader);
  uint32_t flags = header->flags;
#ifndef USE_SPI_FLASH
  flags |= RESOURCE_MEMORY_MAPPED; // save everything mem mapped
#endif
  if(flags & 0xff){ // there's a slot number
    eraseResource(flags & 0xff); // mark as deleted if it exists
  }else{
    eraseResource(header->name); // mark as deleted if it exists
  }
  uint8_t* data = (uint8_t*)header;
  size_t capacity = getFreeSize(flags);
  if(length > capacity){ // assuming 'header' is located in extram
    extern char _EXTRAM_SIZE;
    defrag(data + length, _EXTRAM_SIZE - length, flags);
  }
  Resource* dest = getFreeResource(flags);
  if(!dest){
    error(FLASH_ERROR, "No free resources");
    return 0;
  }

  int status = -1;
  if(dest->isMemoryMapped()){
#ifdef USE_FLASH
    eeprom_unlock();
    eeprom_wait();
    status = eeprom_write_block((uint32_t)dest->getHeader(), data, length);
    eeprom_lock();
#endif
  }else{
#ifdef USE_SPI_FLASH
    uint32_t address = dest->getAddress();
    Flash_write(address, data, length);
    status = 0;
#endif
  }
  index(); // rebuild index
  if(status){
    error(FLASH_ERROR, "Flash write failed");
    return 0;
  }
  if(length != dest->getTotalSize()){
    error(FLASH_ERROR, "Size verification failed");
    return 0;
  }
  if(dest->isMemoryMapped() && memcmp(data, dest->getData(), length) != 0){
    // todo: verify port mapped data
    error(FLASH_ERROR, "Data verification failed");
    return 0;
  }
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
