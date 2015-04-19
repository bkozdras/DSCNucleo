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
        dest->data[iter].milliVoltVoltage = source->data[iter].milliVoltVoltage;
        dest->data[iter].thermocouple = source->data[iter].thermocouple;
    }
    dest->rtdTemperatureValue = source->rtdTemperatureValue;
}

void CopyObject_SPidTunes(SPidTunes* source, SPidTunes* dest)
{
    dest->kp = source->kp;
    dest->td = source->td;
    dest->ti = source->ti;
}

void CopyObject_SControllerData(SControllerData* source, SControllerData* dest)
{
    dest->CV = source->CV;
    dest->ERR = source->ERR;
    dest->PV = source->PV;
    dest->SP = source->SP;
}

void CopyObject_SSegmentData(SSegmentData* source, SSegmentData* dest)
{
    dest->number = source->number;
    dest->settingTimeInterval = source->settingTimeInterval;
    dest->startTemperature = source->startTemperature;
    dest->stopTemperature = source->stopTemperature;
    dest->type = source->type;
}
