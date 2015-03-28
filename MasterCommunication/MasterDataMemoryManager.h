#ifndef _MASTER_DATA_MEMORY_MANAGER_H_

#define _MASTER_DATA_MEMORY_MANAGER_H_

#include "Defines/CommonDefines.h"
#include "SharedDefines/EMessageId.h"

void* MasterDataMemoryManager_allocate(EMessageId messageId);
void MasterDataMemoryManager_free(EMessageId messageId, void* allocatedMemory);
u8 MasterDataMemoryManager_getLength(EMessageId messageId);

#endif
