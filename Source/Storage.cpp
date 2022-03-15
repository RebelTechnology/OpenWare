#include <cstring>
#include <algorithm>
#include "device.h"
#include "message.h"
#include "Storage.h"
#include "eepromcontrol.h"
#include "callbacks.h"
#include "crc32.h"
#ifndef USE_BOOTLOADER_MODE
#include "cmsis_os.h"
#endif

#ifdef USE_NOR_FLASH
#include "flash.h"

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
  while(p > begin && *p == RESOURCE_FREE_MAGIC){
    setProgress(((uint32_t)end-(uint32_t)p)*4095LL/((uint32_t)end-(uint32_t)begin), "Index");
    p--;
  }
  p += align/sizeof(uint32_t);
  p = (uint32_t*)((uint32_t)p & ~(align-1));
  if(*p == RESOURCE_FREE_MAGIC)
    return p;
  return NULL;
}

#ifdef USE_NOR_FLASH
uint32_t findFirstFreePage(uint32_t start, uint32_t end, size_t align){
  uint32_t quad[4]; // read 16 bytes at a time (slow but memory efficient)
  uint32_t address = end-align; // start at the end
  uint16_t progress = 0;
  while(address > start){
    setProgress(progress, "Index");
    flash_read(address, (uint8_t*)quad, sizeof(quad));
    if(RESOURCE_FREE_MAGIC != (quad[0] & quad[1] & quad[2] & quad[3]))
      break;
    address -= sizeof(quad);
    progress = (end-address)*4095LL/(end-start);
    device_watchdog();
  }
  if(address > start)
    return (address+sizeof(quad) + (align-1)) & ~(align-1) ;
  return end;
}
#endif
  
void Storage::init(){
#if defined USE_SPI_FLASH
  extern SPI_HandleTypeDef SPI_FLASH_HANDLE;
  flash_init(&SPI_FLASH_HANDLE);
  memset(nor_index, 0, sizeof(nor_index));
#elif defined USE_QSPI_FLASH
  extern QSPI_HandleTypeDef QSPI_FLASH_HANDLE;
  flash_init(&QSPI_FLASH_HANDLE);
  memset(nor_index, 0, sizeof(nor_index));
#endif
  index();
}

void Storage::index(){
  memset(resources, 0, sizeof(resources));
  size_t i = 0;
  size_t progress = 0;
#ifdef USE_FLASH
  resources[i].setHeader((ResourceHeader*)(INTERNAL_STORAGE_BEGIN));
  while(i<MAX_RESOURCE_HEADERS && resources[i].isUsed()){
    ResourceHeader* next = resources[i].getNextHeader();
    if(resources[i].isValid())
      i++;
    resources[i].setHeader(next);
    setProgress(progress += 4095/MAX_RESOURCE_HEADERS, "Indexing");
  }
  if(resources[i].isFree()){
    i++;
  }else{
    error(FLASH_ERROR, "Invalid flash resource");
    // set resource to point to first free block, or NULL
    void* p = findFirstFreeBlock(resources[i].getHeader(), (void*)INTERNAL_STORAGE_END, 32);
    resources[i++].setHeader((ResourceHeader*)p);
  }
#endif
#ifdef USE_NOR_FLASH
  uint32_t address = 0;
  NorHeader* header = &nor_index[0];
  header->address = address;
  resources[i].setHeader(header);
  flash_read(address, (uint8_t*)header, sizeof(ResourceHeader));
  while(i<MAX_RESOURCE_HEADERS && resources[i].isUsed() &&
	address < EXTERNAL_STORAGE_SIZE &&
	header < &nor_index[MAX_SPI_FLASH_HEADERS]){
    address += resources[i].getTotalSize();
    if(resources[i].isValid())
      resources[++i].setHeader(++header);
    header->address = address;
    flash_read(address, (uint8_t*)header, sizeof(ResourceHeader));
    setProgress(progress += 4095/MAX_RESOURCE_HEADERS, "Indexing");
  }
  if(!resources[i].isFree()){
    error(FLASH_ERROR, "Invalid SPI resource");
    // set resource to point to first free block, or NULL
    address = findFirstFreePage(resources[i].getAddress(), EXTERNAL_STORAGE_SIZE, 256);
    if(address < EXTERNAL_STORAGE_SIZE){
      header = (NorHeader*)resources[i].getHeader();
      header->address = address;
      flash_read(address, (uint8_t*)header, sizeof(ResourceHeader));
    }else{
      resources[i].setHeader(NULL);
    }
  }
#endif
  setProgress(4095, "Indexing");
  resource_count = i+1;
}

uint32_t Storage::getChecksum(ResourceHeader* header){
  Resource resource(header);
  return getChecksum(&resource);
}

uint32_t Storage::getChecksum(Resource* resource){
  uint32_t crc = 0;
  if(resource->isMemoryMapped()){
    crc = crc32(resource->getData(), resource->getDataSize(), 0);
#ifdef USE_NOR_FLASH
  }else{
    uint8_t data[32]; // read chunk of bytes at a time
    uint32_t address = resource->getAddress() + sizeof(ResourceHeader);
    uint32_t end = address + resource->getDataSize();
    while(address < end){
      size_t len = std::min(sizeof(data), (size_t)(end-address));
      flash_read(address, data, len);
      crc = crc32(data, len, crc);
      address += len;
    }
#endif
  }
  return crc;
}

size_t Storage::readResource(ResourceHeader* header, void* data, size_t offset, size_t length){
  Resource resource(header);
  size_t ret = 0;
  if(header){
    size_t len = std::min(resource.getDataSize()-offset, length);
    if(resource.isInMemory()){
      memmove(data, resource.getData()+offset, len);
      ret = len;
    }else if(resource.isMemoryMapped()){
      memcpy(data, resource.getData()+offset, len);
      ret = len;
    }else{
#ifdef USE_NOR_FLASH
      uint32_t address = resource.getAddress();
      memset(data, 0, len);
      flash_read(address+sizeof(ResourceHeader)+offset, (uint8_t*)data, len);
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

bool Storage::eraseResource(ResourceHeader* header){
  Resource resource(header);
  return eraseResource(&resource);
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
    copy.magic = RESOURCE_ERASED_MAGIC;
    status = eeprom_write_block((uint32_t)header, &copy, sizeof(ResourceHeader));
#else
    status = eeprom_write_word((uint32_t)header, RESOURCE_ERASED_MAGIC);
#endif
    eeprom_lock();
#endif
  }else{
#ifdef USE_NOR_FLASH
    uint32_t address = resource->getAddress();
    resource->getHeader()->magic = RESOURCE_ERASED_MAGIC;
    flash_write(address, (uint8_t*)resource->getHeader(), 4); // write new magic
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
    eeprom_erase(INTERNAL_STORAGE_BEGIN, INTERNAL_STORAGE_SIZE);
    eeprom_lock();
  }
#endif
#ifdef USE_NOR_FLASH
  if(flags & RESOURCE_PORT_MAPPED){
    constexpr size_t blocksize = 64*1024;
    uint32_t endaddress = findFirstFreePage(0, EXTERNAL_STORAGE_SIZE, blocksize);
    for(uint32_t address=0; address < endaddress; address += blocksize){
      setProgress(address*4095LL/endaddress, "Erasing");
      flash_erase(address, blocksize); // 450 to 1150 mS each
      device_watchdog();
#ifndef USE_BOOTLOADER_MODE
      vTaskDelay(2);
#endif
    }
    setProgress(4095, "Erasing");
  }
#endif
  index();
}

void Storage::defrag(void* buffer, size_t size, uint32_t flags){
  uint8_t* ptr = (uint8_t*)buffer;
  if(getUsedSize(flags) > size)
    debugMessage("Not enough RAM to defrag");
  uint32_t offset = 0;
  for(uint8_t i=0; i<resource_count; ++i){
    if(resources[i].isValid() && resources[i].flagsContain(flags)){
      if(offset+resources[i].getTotalSize() < size){
	readResource(resources[i].getHeader(), ptr+offset,  0, resources[i].getTotalSize());
	offset += resources[i].getTotalSize();
      }
    }
  }
  erase(flags);
  if(flags & RESOURCE_MEMORY_MAPPED){
#ifdef USE_FLASH
    eeprom_unlock();
    eeprom_write_block(INTERNAL_STORAGE_BEGIN, buffer, offset);
    eeprom_lock();
#endif
  }else{
#ifdef USE_NOR_FLASH
    flash_write(0, (uint8_t*)buffer, offset);
#endif
  }
  index();
}

size_t Storage::writeResourceHeader(void* dest, const char* name, size_t datasize, uint32_t crc, uint32_t flags){
  ResourceHeader header;
  header.magic = RESOURCE_VALID_MAGIC;
  header.size = datasize;
  strlcpy(header.name, name, sizeof(header.name));
  // header.name = name;
  header.checksum = crc;
  header.flags = flags;
  memcpy(dest, &header, sizeof(header));
  return sizeof(header);
}

// data must already include space for ResourceHeader
size_t Storage::writeResource(const char* name, uint8_t* data, size_t datasize, uint32_t flags){
  uint32_t crc = crc32(data+sizeof(ResourceHeader), datasize, 0);
  writeResourceHeader(data, name, datasize, crc, flags);  
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
  if(header->flags & RESOURCE_IN_MEMORY){
    header->flags &= ~(RESOURCE_IN_MEMORY);
    header->flags |= FLASH_DEFAULT_FLAGS;
  }
#ifndef USE_NOR_FLASH
  header->flags |= RESOURCE_MEMORY_MAPPED; // save everything mem mapped
#endif
  size_t length = header->size+sizeof(ResourceHeader);
  uint32_t flags = header->flags;
  uint8_t slot = flags & 0xff;
  uint8_t* data = (uint8_t*)header;
  Resource* old = NULL;
  if(slot){ // if there is a slot number
    old = getResourceBySlot(slot);
  }else{
    old = getResourceByName(header->name);
  }      
  if(old){
    // compare CRC
    uint32_t crc = crc32(data+sizeof(ResourceHeader), header->size, 0);
    if(old->getDataSize() == header->size){
      if(crc == getChecksum(old))
	debugMessage("Resource checksum match");
      if(verifyData(old, data, length)){
	debugMessage("Resource identical");
	return 0;
      }
    }
    eraseResource(old); // mark as deleted if it exists but is non-identical
  }
  size_t capacity = getFreeSize(flags);
  if(length > capacity){ // assuming 'header' is located in extram
#ifdef USE_EXTERNAL_RAM
    extern char _EXTRAM_SIZE;
    defrag(data + length, _EXTRAM_SIZE - length, flags);
#else
    // required by devices with no ext mem
    extern char _PATCHRAM_SIZE;
    defrag(data + length, _PATCHRAM_SIZE - length, flags);
#endif
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
#ifdef USE_NOR_FLASH
    uint32_t address = dest->getAddress();
    flash_write(address, data, length);
    flash_read(address, (uint8_t*)dest->getHeader(), sizeof(ResourceHeader)); // read back resource header
    status = 0;
#endif
  }
  if(status){
    error(FLASH_ERROR, "Flash write failed");
  }else if(dest->getTotalSize() < length){ // allow for storage-specific alignment
    error(FLASH_ERROR, "Size verification failed");
  }else if(!verifyData(dest, data, length)){
    error(FLASH_ERROR, "Data verification failed");
  }
  index(); // rebuild index
  return length;
}

bool Storage::verifyData(Resource* resource, void* data, size_t length){
  if(resource->isMemoryMapped()){
    return memcmp(data, resource->getHeader(), length) == 0;
#ifdef USE_NOR_FLASH
  }else{
    uint32_t quad[4]; // read 16 bytes at a time (slow but memory efficient)
    uint32_t address = resource->getAddress();
    size_t blocks = length/sizeof(quad);
    uint8_t* src = (uint8_t*)data;
    while(blocks--){
      flash_read(address, (uint8_t*)quad, sizeof(quad));
      if(memcmp(src, quad, sizeof(quad)) != 0)
	return false;
      address += sizeof(quad);
      src += sizeof(quad);
    }
    return true;
#endif
  }
  return false;
}

size_t Storage::getFreeSize(uint32_t flags){
  size_t total = 0;
#ifdef USE_FLASH
  if(flags & RESOURCE_MEMORY_MAPPED){
    Resource* resource = getFreeResource(RESOURCE_MEMORY_MAPPED);
    if(resource)
      total += INTERNAL_STORAGE_END - (uint32_t)resource->getHeader();
  }
#endif
#ifdef USE_NOR_FLASH
  if(flags & RESOURCE_PORT_MAPPED){
    Resource* resource = getFreeResource(RESOURCE_PORT_MAPPED);
    if(resource)
      total += EXTERNAL_STORAGE_SIZE - resource->getAddress();
  }
#endif
  return total;
}

size_t Storage::getTotalCapacity(uint32_t flags){
  size_t total = 0;
#ifdef USE_FLASH
  if(flags & RESOURCE_MEMORY_MAPPED)
    total += INTERNAL_STORAGE_SIZE;
#endif
#ifdef USE_NOR_FLASH
  if(flags & RESOURCE_PORT_MAPPED)
    total += EXTERNAL_STORAGE_SIZE;
#endif
  return total;
}
