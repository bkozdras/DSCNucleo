#ifndef _S_SAMPLE_CARRIER_DATA_H_

#define _S_SAMPLE_CARRIER_DATA_H_

#include "SharedDefines/EUnitId.h"
#include "Defines/CommonDefines.h"

#define THERMOCOUPLES_COUNT 5

typedef struct _SThermocoupleData
{
    EUnitId thermocouple;
    double nanoVoltVoltage;
} SThermocoupleData;

typedef struct _SSampleCarrierData
{
    SThermocoupleData data [THERMOCOUPLES_COUNT];
    double rtdTemperatureValue;
} SSampleCarrierData;

#endif
