#include "System/EventManagement/Event.h"
#include "System/EventManagement/TEvent.h"
#include "System/EventManagement/EEventId.h"
#include "System/EventManagement/TEventMessage.h"

#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"

#include "FaultManagement/FaultIndication.h"

#include "cmsis_os.h"
#include "string.h"

/************************************MACROS***************************************************************************************/

#define waitForEvent(threadId, timeout)      osMessageGet(Event_getId(threadId), timeout)

#define ThreadId(name) EThreadId_##name

#define IsThread(threadName) ( ThreadId(threadName) == threadId )

#define EventQueueDef(name) osMessageQDef(name, 1, TEvent)
#define EventQueueSizedDef(name, size) osMessageQDef(name, size, TEvent)
#define EventQueue(name) osMessageQ(name)
#define EventQueueCreate(name) osMessageCreate(EventQueue(name), NULL)
#define EventQueueSend(eventQueueId, data, timeout) osMessagePut(eventQueueId, (uint32_t)data, timeout)
#define EventQueueReceive(eventQueueId, timeout) osMessageGet(eventQueueId, timeout)
#define EventQueueId osMessageQId

#define HeapDef(name, type) osPoolDef(name, 1, type)
#define HeapSizedDef(name, size, type) osPoolDef(name, size, type)
#define Heap(name) osPool(name)
#define HeapCreate(name) osPoolCreate(Heap(name))
#define HeapAlloc(heapId) osPoolAlloc(heapId)
#define HeapCalloc(heapId) osPoolCAlloc(heapId)
#define HeapFree(heapId, event) osPoolFree(heapId, event)
#define HeapId osPoolId

#define GetEventQueueName(thread) EventQueueEThreadId_##thread
#define GetEventQueueId(thread) EventQueueEThreadId_##thread
#define DefineEventQueue(thread) EventQueueDef(GetEventQueueName(thread))
#define DefineEventQueueSized(thread, size) EventQueueSizedDef(GetEventQueueName(thread), size)
#define DefineEventQueueId(thread) EventQueueId GetEventQueueId(thread)

#define GetHeapName(thread) HeapEThreadId_##thread
#define GetHeapId(thread) HeapEThreadId_##thread##ID
#define DefineHeap(thread) HeapDef(GetHeapName(thread), TEvent)
#define DefineHeapSized(thread, size) HeapSizedDef(GetHeapName(thread), size, TEvent)
#define DefineHeapId(thread) HeapId GetHeapId(thread)

#define GetEventMessageHeapName(event) HeapEEventMessageId_##event
#define GetEventMessageHeapId(event) HeapEventMessageId_##event##ID
#define DefineEventMessageHeap(event) HeapDef(GetEventMessageHeapName(event), TEventMessage##event)
#define DefineEventMessageHeapSized(event, size) HeapSizedDef(GetEventMessageHeapName(event), size, TEventMessage##event)
#define DefineEventMessageHeapId(event) HeapId GetEventMessageHeapId(event)

#define DEFINE_EVENT_QUEUE(thread)                          DefineEventQueue(thread);                                                               \
                                                            DefineEventQueueId(thread);                                                             \
                                                            DefineHeap(thread);                                                                     \
                                                            DefineHeapId(thread)
                                            
#define DEFINE_EVENT_QUEUE_SIZED(thread, size)              DefineEventQueueSized(thread, size);                                                    \
                                                            DefineEventQueueId(thread);                                                             \
                                                            DefineHeapSized(thread, size);                                                          \
                                                            DefineHeapId(thread)

#define CREATE_EVENT_QUEUE(thread, event)                   GetEventQueueId(thread) = EventQueueCreate(GetEventQueueName(thread));                  \
                                                            GetHeapId(thread) = HeapCreate(GetHeapName(thread))

#define DEFINE_EVENT_HEAP(event, size)                      DefineEventMessageHeapSized(event, size);                                               \
                                                            DefineEventMessageHeapId(event)
                                                            
#define CREATE_EVENT_HEAP(event)                            GetEventMessageHeapId(event) = HeapCreate(GetEventMessageHeapName(event))

#define EVENT_GET_ID_HANDLER(thread)                        if ( IsThread(thread) )                                                                 \
                                                                return GetEventQueueId(thread);

#define SEND_EVENT_TO_THREAD_HANDLER(thread)                if ( IsThread(thread) )                                                                 \
                                                            {                                                                                       \
                                                                status = EventQueueSend(GetEventQueueId(thread), allocatedEvent, timeout);          \
                                                                return status;                                                                      \
                                                            }
                                                            
#define ALLOCATE_MALLOC_HANDLER(thread)                     if ( IsThread(thread) )                                                                 \
                                                            {                                                                                       \
                                                                allocatedEvent = HeapAlloc(GetHeapId(thread));                                      \
                                                                return allocatedEvent;                                                              \
                                                            }
                                                            
#define ALLOCATE_CALLOC_HANDLER(thread)                     if ( IsThread(thread) )                                                                 \
                                                            {                                                                                       \
                                                                allocatedEvent = HeapCalloc(GetHeapId(thread));                                     \
                                                                return allocatedEvent;                                                              \
                                                            }

#define ALLOCATE_MALLOC_EVENT_MESSAGE_HANDLER(eventName)    case EEventId_##eventName :                                                             \
                                                            {                                                                                       \
                                                                event->data = HeapAlloc(GetEventMessageHeapId(eventName));                          \
                                                                break;                                                                              \
                                                            }
                                                            
#define ALLOCATE_CALLOC_EVENT_MESSAGE_HANDLER(eventName)    case EEventId_##eventName :                                                             \
                                                            {                                                                                       \
                                                                event->data = HeapCalloc(GetEventMessageHeapId(eventName));                         \
                                                                break;                                                                              \
                                                            }

#define FREE_ALLOCATED_EVENT_MESSAGE_HANDLER(eventName)     case EEventId_##eventName :                                                             \
                                                            {                                                                                       \
                                                                HeapFree(GetEventMessageHeapId(eventName), event->data);                            \
                                                                break;                                                                              \
                                                            }

#define FREE_ALLOCATED_EVENT_HANDLER(thread)                if ( IsThread(thread) )                                                                 \
                                                            {                                                                                       \
                                                                HeapFree(GetHeapId(thread), event);                                                 \
                                                                return;                                                                             \
                                                            }
                                                            
/***********************************************STATIC ATTRIBUTES************************************************************/

static osMutexDef(mMutexEvent);
static osMutexId mMutexEventId = NULL;

DEFINE_EVENT_QUEUE_SIZED(ADS1248Controller, 2);
DEFINE_EVENT_QUEUE_SIZED(LMP90100ControlSystemController, 2);
DEFINE_EVENT_QUEUE_SIZED(LMP90100SignalsMeasurementController, 2);
DEFINE_EVENT_QUEUE_SIZED(HeaterTemperatureReader, 2);
DEFINE_EVENT_QUEUE_SIZED(SampleCarrierDataManager, 2);
DEFINE_EVENT_QUEUE_SIZED(ReferenceTemperatureReader, 2);
DEFINE_EVENT_QUEUE_SIZED(SampleThread, 2);
DEFINE_EVENT_QUEUE_SIZED(SystemManager, 2);
DEFINE_EVENT_QUEUE_SIZED(MasterDataTransmitter, 10);
DEFINE_EVENT_QUEUE_SIZED(MasterDataReceiver, 10);
DEFINE_EVENT_QUEUE_SIZED(MasterDataManager, 10);
DEFINE_EVENT_QUEUE_SIZED(StaticSegmentProgramExecutor, 2);

DEFINE_EVENT_HEAP(NewRTDValueInd, 10);
DEFINE_EVENT_HEAP(NewThermocoupleVoltageValueInd, 10);
DEFINE_EVENT_HEAP(DataFromMasterReceivedInd, 20);

/***************************************INTERNAL FUNCTION DECLARATIONS*******************************************************/

static osStatus sendEventToThread(EThreadId threadId, TEvent* allocatedEvent);

static TEvent* allocateMallocEvent(EThreadId threadId);
static void allocateMallocEventMessage(TEvent* event, EEventId eventId);
static TEvent* allocateCallocEvent(EThreadId threadId);
static void allocateCallocEventMessage(TEvent* event, EEventId eventId);

static void freeEventMessage(TEvent* event);
static void freeEvent(EThreadId threadId, TEvent* event);

/******************************************FUNCTION IMPLEMENTATIONS**********************************************************/

void Event_setup(void)
{
    if ( !mMutexEventId )
    {
        mMutexEventId = osMutexCreate( osMutex(mMutexEvent) );
    }
    
    CREATE_EVENT_QUEUE(ADS1248Controller, Signal);
    CREATE_EVENT_QUEUE(LMP90100ControlSystemController, Signal);
    CREATE_EVENT_QUEUE(LMP90100SignalsMeasurementController, Signal);
    CREATE_EVENT_QUEUE(SampleCarrierDataManager, Signal);
    CREATE_EVENT_QUEUE(HeaterTemperatureReader, Signal);
    CREATE_EVENT_QUEUE(ReferenceTemperatureReader, Signal);
    CREATE_EVENT_QUEUE(SampleThread, Signal);
    CREATE_EVENT_QUEUE(SystemManager, Signal);
    CREATE_EVENT_QUEUE(MasterDataTransmitter, Signal);
    CREATE_EVENT_QUEUE(MasterDataReceiver, Signal);
    CREATE_EVENT_QUEUE(MasterDataManager, Signal);
    CREATE_EVENT_QUEUE(StaticSegmentProgramExecutor, Signal);
    
    CREATE_EVENT_HEAP(NewRTDValueInd);
    CREATE_EVENT_HEAP(NewThermocoupleVoltageValueInd);
    CREATE_EVENT_HEAP(DataFromMasterReceivedInd);
}

OsEventId Event_getId(EThreadId threadId)
{
    EVENT_GET_ID_HANDLER(ADS1248Controller)
    EVENT_GET_ID_HANDLER(LMP90100ControlSystemController)
    EVENT_GET_ID_HANDLER(LMP90100SignalsMeasurementController)
    EVENT_GET_ID_HANDLER(HeaterTemperatureReader)
    EVENT_GET_ID_HANDLER(SampleCarrierDataManager)
    EVENT_GET_ID_HANDLER(ReferenceTemperatureReader)
    EVENT_GET_ID_HANDLER(SampleThread)
    EVENT_GET_ID_HANDLER(SystemManager)
    EVENT_GET_ID_HANDLER(MasterDataTransmitter)
    EVENT_GET_ID_HANDLER(MasterDataReceiver)
    EVENT_GET_ID_HANDLER(MasterDataManager)
    EVENT_GET_ID_HANDLER(StaticSegmentProgramExecutor)
    
    return NULL;
}
osStatus Event_send(EThreadId threadId, TEvent* event)
{
    return sendEventToThread(threadId, event);
}

TEvent* Event_malloc(EThreadId threadId, EEventId eventId)
{
    void* allocatedEvent = allocateMallocEvent(threadId);
    
    if ( allocatedEvent )
    {
        allocateMallocEventMessage(allocatedEvent, eventId);
    }
    
    return allocatedEvent;
}

TEvent* Event_calloc(EThreadId threadId, EEventId eventId)
{
    void* allocatedEvent = allocateCallocEvent(threadId);
    
    if ( allocatedEvent )
    {
        allocateCallocEventMessage(allocatedEvent, eventId);
    }
    
    return allocatedEvent;
}

void Event_free(EThreadId threadId, TEvent* event)
{
    if (event)
    {
        freeEventMessage(event);
        freeEvent(threadId, event);
    }
}

osStatus sendEventToThread(EThreadId threadId, TEvent* allocatedEvent)
{
    osStatus status = osErrorValue;
    
    if (allocatedEvent)
    {
        u32 timeout = 1000;

        SEND_EVENT_TO_THREAD_HANDLER(ADS1248Controller)
        SEND_EVENT_TO_THREAD_HANDLER(LMP90100ControlSystemController)
        SEND_EVENT_TO_THREAD_HANDLER(LMP90100SignalsMeasurementController)
        SEND_EVENT_TO_THREAD_HANDLER(HeaterTemperatureReader)
        SEND_EVENT_TO_THREAD_HANDLER(SampleCarrierDataManager)
        SEND_EVENT_TO_THREAD_HANDLER(ReferenceTemperatureReader)
        SEND_EVENT_TO_THREAD_HANDLER(SampleThread)
        SEND_EVENT_TO_THREAD_HANDLER(SystemManager)
        SEND_EVENT_TO_THREAD_HANDLER(MasterDataTransmitter)
        SEND_EVENT_TO_THREAD_HANDLER(MasterDataReceiver)
        SEND_EVENT_TO_THREAD_HANDLER(MasterDataManager)
        SEND_EVENT_TO_THREAD_HANDLER(StaticSegmentProgramExecutor)
    }
    
    return status;
}

TEvent* Event_wait(EThreadId threadId, u32 timeout)
{
    osEvent event = waitForEvent(threadId, timeout);
    if (osEventMessage == event.status)
    {
        return event.value.p;
    }
    else
    {
        Logger_error("Event: Failure during waiting for event (Thread: %s). Reason: %s.", CStringConverter_EThreadId(threadId), CStringConverter_osStatus(event.status));
        FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
        return NULL;
    }
}

TEvent* allocateMallocEvent(EThreadId threadId)
{
    void* allocatedEvent = NULL;

    ALLOCATE_MALLOC_HANDLER(ADS1248Controller)
    ALLOCATE_MALLOC_HANDLER(LMP90100ControlSystemController)
    ALLOCATE_MALLOC_HANDLER(LMP90100SignalsMeasurementController)
    ALLOCATE_MALLOC_HANDLER(HeaterTemperatureReader)
    ALLOCATE_MALLOC_HANDLER(SampleCarrierDataManager)
    ALLOCATE_MALLOC_HANDLER(ReferenceTemperatureReader)
    ALLOCATE_MALLOC_HANDLER(SampleThread)
    ALLOCATE_MALLOC_HANDLER(SystemManager)
    ALLOCATE_MALLOC_HANDLER(MasterDataTransmitter)
    ALLOCATE_MALLOC_HANDLER(MasterDataReceiver)
    ALLOCATE_MALLOC_HANDLER(MasterDataManager)
    ALLOCATE_MALLOC_HANDLER(StaticSegmentProgramExecutor)
    
    return allocatedEvent;
}

void allocateMallocEventMessage(TEvent* event, EEventId eventId)
{
    switch (eventId)
    {
        ALLOCATE_MALLOC_EVENT_MESSAGE_HANDLER(NewRTDValueInd)
        ALLOCATE_MALLOC_EVENT_MESSAGE_HANDLER(NewThermocoupleVoltageValueInd)
        ALLOCATE_MALLOC_EVENT_MESSAGE_HANDLER(DataFromMasterReceivedInd)
        
        default :
            break;
    }
    
    event->id = eventId;
}

TEvent* allocateCallocEvent(EThreadId threadId)
{
    TEvent* allocatedEvent = NULL;

    ALLOCATE_CALLOC_HANDLER(ADS1248Controller)
    ALLOCATE_CALLOC_HANDLER(LMP90100ControlSystemController)
    ALLOCATE_CALLOC_HANDLER(LMP90100SignalsMeasurementController)
    ALLOCATE_CALLOC_HANDLER(HeaterTemperatureReader)
    ALLOCATE_CALLOC_HANDLER(SampleCarrierDataManager)
    ALLOCATE_CALLOC_HANDLER(ReferenceTemperatureReader)
    ALLOCATE_CALLOC_HANDLER(SampleThread)
    ALLOCATE_CALLOC_HANDLER(SystemManager)
    ALLOCATE_CALLOC_HANDLER(MasterDataTransmitter)
    ALLOCATE_CALLOC_HANDLER(MasterDataReceiver)
    ALLOCATE_CALLOC_HANDLER(MasterDataManager)
    ALLOCATE_CALLOC_HANDLER(StaticSegmentProgramExecutor)
    
    return allocatedEvent;
}

void allocateCallocEventMessage(TEvent* event, EEventId eventId)
{
    switch (eventId)
    {
        ALLOCATE_CALLOC_EVENT_MESSAGE_HANDLER(NewRTDValueInd)
        ALLOCATE_CALLOC_EVENT_MESSAGE_HANDLER(NewThermocoupleVoltageValueInd)
        ALLOCATE_CALLOC_EVENT_MESSAGE_HANDLER(DataFromMasterReceivedInd)
        
        default :
            break;
    }
    
    event->id = eventId;
}

void freeEventMessage(TEvent* event)
{
    switch (event->id)
    {
        FREE_ALLOCATED_EVENT_MESSAGE_HANDLER(NewRTDValueInd)
        FREE_ALLOCATED_EVENT_MESSAGE_HANDLER(NewThermocoupleVoltageValueInd)
        FREE_ALLOCATED_EVENT_MESSAGE_HANDLER(DataFromMasterReceivedInd)
        
        default :
            break;
    }
}

void freeEvent(EThreadId threadId, TEvent* event)
{
    FREE_ALLOCATED_EVENT_HANDLER(ADS1248Controller)
    FREE_ALLOCATED_EVENT_HANDLER(LMP90100ControlSystemController)
    FREE_ALLOCATED_EVENT_HANDLER(LMP90100SignalsMeasurementController)
    FREE_ALLOCATED_EVENT_HANDLER(HeaterTemperatureReader)
    FREE_ALLOCATED_EVENT_HANDLER(SampleCarrierDataManager)
    FREE_ALLOCATED_EVENT_HANDLER(ReferenceTemperatureReader)
    FREE_ALLOCATED_EVENT_HANDLER(SampleThread)
    FREE_ALLOCATED_EVENT_HANDLER(SystemManager)
    FREE_ALLOCATED_EVENT_HANDLER(MasterDataTransmitter)
    FREE_ALLOCATED_EVENT_HANDLER(MasterDataReceiver)
    FREE_ALLOCATED_EVENT_HANDLER(MasterDataManager)
    FREE_ALLOCATED_EVENT_HANDLER(StaticSegmentProgramExecutor)
}

#undef waitForEvent
#undef ThreadId
#undef IsThread
#undef EventQueueDef
#undef EventQueueSizedDef
#undef EventQueue
#undef EventQueueCreate
#undef EventQueueSend
#undef EventQueueReceive
#undef EventQueueId
#undef HeapDef
#undef HeapSizedDef
#undef Heap
#undef HeapCreate
#undef HeapAlloc
#undef HeapCalloc
#undef HeapFree
#undef HeapId
#undef GetEventQueueName
#undef GetEventQueueId
#undef DefineEventQueue
#undef DefineEventQueueSized
#undef DefineEventQueueId
#undef GetHeapName
#undef GetHeapId
#undef DefineHeap
#undef DefineHeapSized
#undef DefineHeapId
#undef GetEventMessageHeapName
#undef GetEventMessageHeapId
#undef DefineEventMessageHeap
#undef DefineEventMessageHeapSized
#undef DefineEventMessageHeapId
#undef DEFINE_EVENT_QUEUE
#undef DEFINE_EVENT_QUEUE_SIZED
#undef CREATE_EVENT_QUEUE
#undef DEFINE_EVENT_HEAP
#undef CREATE_EVENT_HEAP
#undef EVENT_GET_ID_HANDLER
#undef SEND_EVENT_TO_THREAD_HANDLER
#undef ALLOCATE_MALLOC_HANDLER
#undef ALLOCATE_CALLOC_HANDLER
#undef ALLOCATE_MALLOC_EVENT_MESSAGE_HANDLER
#undef ALLOCATE_CALLOC_EVENT_MESSAGE_HANDLER
#undef FREE_ALLOCATED_EVENT_MESSAGE_HANDLER
#undef FREE_ALLOCATED_EVENT_HANDLER
