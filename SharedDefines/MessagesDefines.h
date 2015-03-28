#ifndef _MESSAGES_DEFINES_H_

#define _MESSAGES_DEFINES_H_

#include "stdbool.h"
#include "SharedDefines/SFaultIndication.h"
#include "SharedDefines/ELogSeverity.h"

typedef struct _TLogInd
{
    ELogSeverity severity;
    u16 length;
    char data [220];
} TLogInd;

typedef struct _TFaultInd
{
    SFaultIndication indication;
} TFaultInd;
 
typedef struct _TPollingRequest
{
    bool dummy;
} TPollingRequest;

typedef struct _TPollingResponse
{
    bool success;
} TPollingResponse;

typedef struct _TUnexpectedMasterMessageInd
{
    EMessageId messageId;
} TUnexpectedMasterMessageInd;

#endif
