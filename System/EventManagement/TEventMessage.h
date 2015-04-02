#ifndef _T_EVENT_MESSAGE_H_

#define _T_EVENT_MESSAGE_H_

#include "SharedDefines/EUnitId.h"
#include "SharedDefines/TMessage.h"

typedef struct _TEventMessageNewRTDValueInd
{
    float value;
} TEventMessageNewRTDValueInd;

typedef struct _TEventMessageNewThermocoupleVoltageValueInd
{
    EUnitId thermocouple;
    double value;
} TEventMessageNewThermocoupleVoltageValueInd;

typedef struct _TEventMessageDataFromMasterReceivedInd
{
    TMessage message;
} TEventMessageDataFromMasterReceivedInd;

#endif
