#ifndef __PatchRegistry_h__
#define __PatchRegistry_h__

#include "device.h"
#include "PatchDefinition.hpp"
#include "StorageBlock.h"
#include "ResourceHeader.h"

class PatchRegistry;
extern PatchRegistry registry;

class PatchRegistry {
public:
  PatchRegistry();
  void init();
  /* const char* getName(unsigned int index); */
  const char* getPatchName(unsigned int index);
  const char* getResourceName(unsigned int index);
  PatchDefinition* getPatchDefinition(unsigned int index);
  unsigned int getNumberOfPatches();
  unsigned int getNumberOfResources();
  void registerPatch(PatchDefinition* def);
  void setDynamicPatchDefinition(PatchDefinition* def){
    dynamicPatchDefinition = def;
  }
  ResourceHeader* getResource(uint8_t index);
  ResourceHeader* getResource(const char* name);
  uint8_t* getData(ResourceHeader* resource);
  void store(uint8_t index, uint8_t* data, size_t size);
  void setDeleted(uint8_t index);
private:
  bool isPresetBlock(StorageBlock block);
  StorageBlock patchblocks[MAX_NUMBER_OF_PATCHES];
  StorageBlock resourceblocks[MAX_NUMBER_OF_RESOURCES];
  PatchDefinition* defs[MAX_NUMBER_OF_PATCHES];
  uint8_t patchCount, resourceCount;
  PatchDefinition* dynamicPatchDefinition;
};

// Wrappers to be used as callbacks for service calls
void store_resource(uint8_t index, ResourceHeader* resource);
void delete_resource(uint8_t index);

#endif // __PatchRegistry_h__
