#ifndef _MASTER_DATA_TRANSMITTER_H_

#define _MASTER_DATA_TRANSMITTER_H_

#include "Defines/CommonDefines.h"
#include "System/ThreadMacros.h"
#include "SharedDefines/TMessage.h"

THREAD_PROTOTYPE(MasterDataTransmitter)

void MasterDataTransmitter_setup(void);
void MasterDataTransmitter_initialize(void);
void MasterDataTransmitter_transmitAsync(TMessage* message);

#endif
