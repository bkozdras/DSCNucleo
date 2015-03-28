#ifndef _MASTER_DATA_RECEIVER_H_

#define _MASTER_DATA_RECEIVER_H_

#include "Defines/CommonDefines.h"
#include "System/ThreadMacros.h"
#include "System/EThreadId.h"

THREAD_PROTOTYPE(MasterDataReceiver)

void MasterDataReceiver_setup(void);
void MasterDataReceiver_initialize(void);

#endif
