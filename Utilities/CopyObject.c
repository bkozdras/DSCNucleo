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
    /*
    dest->refThermocoupleValue = source->refThermocoupleValue;
    dest->thermocouple1Value = source->thermocouple1Value;
    dest->thermocouple2Value = source->thermocouple2Value;
    dest->thermocouple3Value = source->thermocouple3Value;
    dest->thermocouple4Value = source->thermocouple4Value;
    dest->rtdTemperatureValue = source->rtdTemperatureValue;*/
    dest->unitId = source->unitId;
    dest->value = source->value;
}

void CopyObject_SPidTunes(SPidTunes* source, SPidTunes* dest)
{
    dest->kp = source->kp;
    dest->kd = source->kd;
    dest->ki = source->ki;
    dest->n = source->n;
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
    dest->temperatureStep = source->temperatureStep;
    dest->type = source->type;
}
