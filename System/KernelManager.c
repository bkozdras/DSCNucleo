#include "System/KernelManager.h"
#include "Defines/CommonDefines.h"
#include "Utilities/Logger/Logger.h"
#include "System/EventManagement/Event.h"
#include "System/EventManagement/TEvent.h"
#include "System/EventManagement/EEventId.h"
#include "FaultManagement/FaultIndication.h"

#include "cmsis_os.h"

static osMutexDef(mMutexCommunication);
static osMutexId mMutexCommunicationId;

static osMutexDef(mMutexData);
static osMutexId mMutexDataId;

typedef struct _SThreadData
{
    EThreadId threadId;
    osThreadId osSpecifiedThreadId;
    bool isRunning;
    bool isRegistered;
} SThreadData;

static SThreadData mThreadData [_THREAD_IDs_COUNT];

static void startThread(EThreadId client, EThreadId threadId);
static void stopThread(EThreadId client, EThreadId threadId);
static osThreadId getOsThreadId(EThreadId threadId);
static SThreadData* getThreadData(EThreadId threadId);
static const char* getLoggerPrefix(void);

void KernelManager_setup(void)
{    
    mMutexCommunicationId = osMutexCreate( osMutex(mMutexCommunication) );
    mMutexDataId = osMutexCreate( osMutex(mMutexData) );
    
    for (u8 iter = 0; _THREAD_IDs_COUNT > iter; ++iter)
    {
        mThreadData[iter].isRegistered = false;
        mThreadData[iter].isRunning = false;
        mThreadData[iter].osSpecifiedThreadId = 0;
        mThreadData[iter].threadId = (EThreadId) iter;
    }
}

void KernelManager_registerThread(EThreadId threadId, osThreadId osSpecifiedThreadId)
{
    SThreadData* threadData = getThreadData(threadId);
    if (threadData && !threadData->isRegistered)
    {
        threadData->osSpecifiedThreadId = osSpecifiedThreadId;
        threadData->isRegistered = true;
    }
}

void KernelManager_startThread(EThreadId client, EThreadId threadId)
{   
    bool isSuccess = false;
    
    Logger_debug("%s: Starting thread: %s (Client: %s).", getLoggerPrefix(), CStringConverter_EThreadId(threadId), CStringConverter_EThreadId(client));
    osMutexWait(mMutexDataId, osWaitForever);
    SThreadData* threadData = getThreadData(threadId);
    bool isAllowed = (threadData && !threadData->isRunning);
    osMutexRelease(mMutexDataId);
    
    osMutexWait(mMutexCommunicationId, osWaitForever);
    if ( isAllowed )
    {
        startThread(client, threadId);
        osMutexRelease(mMutexCommunicationId);
        TEvent* event = Event_wait(client, 1000);
        if (event && EEventId_StartAck == event->id)
        {
            Event_free(client, event);
            osMutexWait(mMutexDataId, osWaitForever);
            threadData->isRunning = true;
            osMutexRelease(mMutexDataId);
            isSuccess = true;
            Logger_info("%s: Starting thread: %s done.", getLoggerPrefix(), CStringConverter_EThreadId(threadId));
        }
        else
        {
            Logger_error("%s: Starting thread: %s failure.", getLoggerPrefix(), CStringConverter_EThreadId(threadId));
        }
    }
    else
    {
        Logger_error("%s: Cannot start thread with ID: %d.", getLoggerPrefix(), CStringConverter_EThreadId(threadId));
        osMutexRelease(mMutexCommunicationId);
    }
    
    if (!isSuccess)
    {
        FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
    }
}

void KernelManager_stopThread(EThreadId client, EThreadId threadId)
{
    bool isSuccess = false;
    
    Logger_debug("%s: Stopping thread: %s (Client: %s).", getLoggerPrefix(), CStringConverter_EThreadId(threadId), CStringConverter_EThreadId(client));
    osMutexWait(mMutexDataId, osWaitForever);
    SThreadData* threadData = getThreadData(threadId);
    bool isAllowed = (threadData && !threadData->isRunning);
    osMutexRelease(mMutexDataId);
    
    osMutexWait(mMutexCommunicationId, osWaitForever);
    if ( isAllowed )
    {
        stopThread(client, threadId);
        osMutexRelease(mMutexCommunicationId);
        TEvent* event = Event_wait(client, 1000);
        if (event && EEventId_StopAck == event->id)
        {
            Event_free(client, event);
            osMutexWait(mMutexDataId, osWaitForever);
            threadData->isRunning = true;
            osMutexRelease(mMutexDataId);
            isSuccess = true;
            Logger_info("%s: Stopping thread: %s done.", getLoggerPrefix(), CStringConverter_EThreadId(threadId));
        }
        else
        {
            Logger_error("%s: Stopping thread: %s failure.", getLoggerPrefix(), CStringConverter_EThreadId(threadId));
            
        }
    }
    else
    {
        Logger_error("%s: Cannot stop thread with ID: %d.", getLoggerPrefix(), CStringConverter_EThreadId(threadId));
        osMutexRelease(mMutexCommunicationId);
    }
    
    if (!isSuccess)
    {
        FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
    }
}

osThreadId KernelManager_getOsThreadId(EThreadId threadId)
{
    osMutexWait(mMutexDataId, osWaitForever);
    osThreadId osSpecifiedThreadId = getOsThreadId(threadId);
    osMutexRelease(mMutexDataId);
    if (!osSpecifiedThreadId)
    {
        Logger_error("%s: Cannot determine osThreadId for Thread: %s.", getLoggerPrefix(), CStringConverter_EThreadId(threadId));
    }
    return osSpecifiedThreadId;
}

bool KernelManager_isThreadRunning(EThreadId threadId)
{
    osMutexWait(mMutexDataId, osWaitForever);
    SThreadData* threadData = getThreadData(threadId);
    bool isRunning = false;
    if (threadData)
    {
        isRunning = threadData->isRunning;
    }
    else
    {
        Logger_error("%s: Wrong thread ID: %d.", getLoggerPrefix(), threadId);
    }
    osMutexRelease(mMutexDataId);
    return isRunning;
}

void startThread(EThreadId client, EThreadId threadId)
{
    TEvent* event = Event_calloc(threadId, EEventId_Start);
    event->sender = client;
    osStatus status = Event_send(threadId, event);
    if (osOK != status)
    {
        Logger_warning
        (
            "%s: Starting thread failed. Cannot send event from %s to %s. Reason: %s.",
            getLoggerPrefix(),
            CStringConverter_EThreadId(client),
            CStringConverter_EThreadId(threadId),
            CStringConverter_osStatus(status)
        );
    }
}

void stopThread(EThreadId client, EThreadId threadId)
{
    TEvent* event = Event_calloc(threadId, EEventId_Stop);
    event->sender = client;
    osStatus status = Event_send(threadId, event);
    if (osOK != status)
    {
        Logger_warning
        (
            "%s: Stopping thread failed. Cannot send event from %s to %s. Reason: %s.",
            getLoggerPrefix(),
            CStringConverter_EThreadId(client),
            CStringConverter_EThreadId(threadId),
            CStringConverter_osStatus(status)
        );
    }
}

osThreadId getOsThreadId(EThreadId threadId)
{
    SThreadData* threadData = getThreadData(threadId);
    if (threadData && threadData->isRegistered)
    {
        return threadData->osSpecifiedThreadId;
    }
    else
    {
        return NULL;
    }
}

bool isThreadRunning(SThreadData* threadData)
{
    return threadData->isRunning;    
}

SThreadData* getThreadData(EThreadId threadId)
{
    for (u8 iter = 0; _THREAD_IDs_COUNT > iter; ++iter)
    {
        SThreadData* threadData = &(mThreadData[iter]);
        if (threadData->threadId == threadId)
        {
            return threadData;
        }
    }
    
    return NULL;
}

const char* getLoggerPrefix(void)
{
    return "KernelManager";
}
