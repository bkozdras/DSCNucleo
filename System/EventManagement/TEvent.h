#ifndef _T_EVENT_H_

#define _T_EVENT_H_

#include "System/EventManagement/EEventId.h"
#include "System/EThreadId.h"

typedef struct _TEvent
{
    EThreadId sender;
    EEventId id;
    void* data;
} TEvent;

#endif
