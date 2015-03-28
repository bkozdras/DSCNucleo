#ifndef _FAULT_INDICATION_H_

#define _FAULT_INDICATION_H_

#include "SharedDefines/EFaultId.h"
#include "SharedDefines/EUnitId.h"

void FaultIndication_start(EFaultId faultId, EUnitId faultyUnitId, EUnitId faultySubUnitId);
void FaultIndication_cancel(EFaultId faultId, EUnitId faultyUnitId, EUnitId faultySubUnitId);

#endif
