#ifndef _S_FAULT_INDICATION_H_

#define _S_FAULT_INDICATION_H_

#include "SharedDefines/EFaultId.h"
#include "SharedDefines/EFaultIndicationState.h"
#include "SharedDefines/EUnitId.h"

typedef struct _SFaultIndication
{
    EFaultId faultId;
    EUnitId faultyUnitId;
    EUnitId faultySubUnitId;
    EFaultIndicationState state;
} SFaultIndication;

#endif
