#include "Utilities/CopyObject.h"

void CopyObject_TMessage(TMessage* source, TMessage* dest)
{
    dest->data = source->data;
    dest->id = source->id;
    dest->length = source->length;
    dest->transactionId = source->transactionId;
}

void CopyObject_SFaultIndication(SFaultIndication* source, SFaultIndication* dest)
{
    dest->faultId = source->faultId;
    dest->faultySubUnitId = source->faultySubUnitId;
    dest->faultyUnitId = source->faultyUnitId;
    dest->state = source->state;
}

void CopyObject_SSampleCarrierData(SSampleCarrierData* source, SSampleCarrierData* dest)
{
    for (u8 iter = 0; THERMOCOUPLES_COUNT > iter; ++iter)
    {
        dest->data[iter].nanoVoltVoltage = source->data[iter].nanoVoltVoltage;
        dest->data[iter].thermocouple = source->data[iter].thermocouple;
    }
    dest->rtdTemperatureValue = source->rtdTemperatureValue;
}
