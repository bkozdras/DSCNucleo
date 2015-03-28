#include "FaultManagement/FaultIndication.h"
#include "SharedDefines/SFaultIndication.h"

#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"

static void sendFaultIndication(EFaultId faultId, EUnitId faultyUnitId, EUnitId faultySubUnitId, EFaultIndicationState state);

void FaultIndication_start(EFaultId faultId, EUnitId faultyUnitId, EUnitId faultySubUnitId)
{
    Logger_error
    (
        "Fault Indication (Start): ID: %s, Unit: %s, SubUnit: %s.",
        CStringConverter_EFaultId(faultId),
        CStringConverter_EUnitId(faultyUnitId),
        CStringConverter_EUnitId(faultySubUnitId)
    );
    
    sendFaultIndication(faultId, faultyUnitId, faultySubUnitId, EFaultIndicationState_Start);
}

void FaultIndication_cancel(EFaultId faultId, EUnitId faultyUnitId, EUnitId faultySubUnitId)
{
    Logger_warning
    (
        "Fault Indication (Cancel): ID: %s, Unit: %s, SubUnit: %s.",
        CStringConverter_EFaultId(faultId),
        CStringConverter_EUnitId(faultyUnitId),
        CStringConverter_EUnitId(faultySubUnitId)
    );
    
    sendFaultIndication(faultId, faultyUnitId, faultySubUnitId, EFaultIndicationState_Cancel);
}

void sendFaultIndication(EFaultId faultId, EUnitId faultyUnitId, EUnitId faultySubUnitId, EFaultIndicationState state)
{
    //SFaultIndication faultIndication;
    
    //faultIndication.faultId = faultId;
    //faultIndication.state = state;
    //faultIndication.faultyUnitId = faultyUnitId;
    //faultIndication.faultySubUnitId = faultySubUnitId;
    // sendMessage(EMessageId_FaultIndication, &faultIndication);
}
