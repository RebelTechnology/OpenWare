#include "PatchRegistry.h"
#include "ProgramManager.h"
#include "ResourceHeader.h"
#include "ProgramHeader.h"
#include "DynamicPatchDefinition.hpp"
#include "Storage.h"
#include "message.h"

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

static PatchDefinition emptyPatch("---", 0, 0);

PatchRegistry::PatchRegistry() {}

void PatchRegistry::init() {
  patchCount = 0;
  resourceCount = 0;

  memset(patches, 0, sizeof(patches));
  memset(resources, 0, sizeof(resources));

  for(size_t i=0; i<storage.getNumberOfResources(); ++i){
    Resource* resource = storage.getResource(i);
    if(resource->isValid()){
      uint8_t id = resource->getSlot();
      if(resource->isPatch() && id <= MAX_NUMBER_OF_PATCHES){
        patches[id-1] = resource;
        patchCount = max(patchCount, id);
      }else if(!resource->isSystemResource() && resourceCount < MAX_NUMBER_OF_RESOURCES){
        resources[resourceCount++] = resource;
      }
    }
  }
}

Resource* PatchRegistry::getResource(uint8_t index){
  if(index < MAX_NUMBER_OF_RESOURCES)
    return resources[index];
  return NULL;
}

Resource* PatchRegistry::getPatch(uint8_t index){
  if(index < MAX_NUMBER_OF_PATCHES)
    return patches[index];
  return NULL;
}

const char* PatchRegistry::getResourceName(unsigned int index){
  Resource* hdr = getResource(index);
  if(hdr)
    return hdr->getName();
  return emptyPatch.getName();
}

const char* PatchRegistry::getPatchName(unsigned int index){
  if(index == 0){
    PatchDefinition *def = dynamicPatchDefinition;
    if(def)
      return def->getName();    
  }else{
    Resource* resource = getPatch(index-1);
    if(resource)
      return resource->getName();
  }
  return emptyPatch.getName();
}

unsigned int PatchRegistry::getNumberOfPatches(){
  // +1 for the current / dynamic patch in slot 0
  return patchCount+1;
}

unsigned int PatchRegistry::getNumberOfResources(){
  return resourceCount;
}

bool PatchRegistry::hasPatches(){
  return patchCount > 0 || dynamicPatchDefinition != NULL;
}

PatchDefinition* PatchRegistry::getPatchDefinition(unsigned int index){
  PatchDefinition *def = NULL;
  if(index == 0)
    def = dynamicPatchDefinition;
  else if(--index < MAX_NUMBER_OF_PATCHES){
    static DynamicPatchDefinition flashPatch;
    Resource* resource = patches[index];
    if(resource && resource->isValid()){
      if(flashPatch.load(resource))
        def = &flashPatch;
    }
  }
  if(def == &emptyPatch)
    def = NULL;
  return def;
}

// void PatchRegistry::registerPatch(PatchDefinition* def){
//   if(patchCount < MAX_NUMBER_OF_PATCHES)
//     defs[patchCount++] = def;
// }

// void delete_resource(uint8_t index){
//   registry.setDeleted(index + MAX_NUMBER_OF_PATCHES + 1);
// }
