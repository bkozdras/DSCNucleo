#ifndef _MASTER_DATA_MANAGER_H_

#define _MASTER_DATA_MANAGER_H_

#include "Defines/CommonDefines.h"
#include "System/ThreadMacros.h"

THREAD_PROTOTYPE(MasterDataManager)

void MasterDataManager_setup(void);
void MasterDataManager_initialize(void);

#endif
