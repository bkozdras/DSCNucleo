#ifndef _FAULT_INDICATION_H_

#define _FAULT_INDICATION_H_

#include "SharedDefines/EFaultId.h"
#include "SharedDefines/EUnitId.h"
#include "SharedDefines/SFaultIndication.h"

void FaultIndication_start(EFaultId faultId, EUnitId faultyUnitId, EUnitId faultySubUnitId);
void FaultIndication_cancel(EFaultId faultId, EUnitId faultyUnitId, EUnitId faultySubUnitId);

void FaultIndication_registerNewFaultCallback(void (*callback)(SFaultIndication*));
void FaultIndication_deregisterNewFaultCallback(void);

#endif
