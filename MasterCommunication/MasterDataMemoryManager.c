#include "MasterCommunication/MasterDataMemoryManager.h"

#include "SharedDefines/EMessageId.h"
#include "SharedDefines/MessagesDefines.h"
#include "SharedDefines/ELogSeverity.h"

#include "cmsis_os.h"

#define HeapSizedDef(name, size, type) osPoolDef(name, size, type)
#define Heap(name) osPool(name)
#define HeapCreate(name) osPoolCreate(Heap(name))
#define HeapAlloc(heapId) osPoolAlloc(heapId)
#define HeapCalloc(heapId) osPoolCAlloc(heapId)
#define HeapFree(heapId, event) osPoolFree(heapId, event)
#define HeapId osPoolId

#define GetHeapName(message) HeapEMessageId_##message
#define GetHeapId(message) HeapEMessageId_##message##ID
#define DefineHeapSized(message, size) HeapSizedDef(GetHeapName(message), size, T##message )
#define DefineHeapId(message) HeapId GetHeapId(message)

#define MessageId(message) EMessageId_##message
#define IsMessage(message) ( MessageId(message) == messageId )

#define ALLOCATE_MESSAGE_CALLOC_HANDLER(message)            if ( IsMessage(message) )                                                                   \
                                                            {                                                                                           \
                                                                allocatedMessage = HeapCalloc(GetHeapId(message));                                      \
                                                                return allocatedMessage;                                                                \
                                                            }

#define FREE_ALLOCATED_MESSAGE_HANDLER(message)             if ( IsMessage(message) )                                                                   \
                                                            {                                                                                           \
                                                                HeapFree(GetHeapId(message), allocatedMemory);                                          \
                                                                return;                                                                                 \
                                                            }

#define GET_MESSAGE_LENGTH(message)                         if ( IsMessage(message) )                                                                   \
                                                            {                                                                                           \
                                                                return sizeof(T##message);                                                              \
                                                            }
                                                            
#define DEFINE_MESSAGE_HEAP(message, size)  DefineHeapSized(message, size); \
                                            DefineHeapId(message)

DEFINE_MESSAGE_HEAP(LogInd, 10);
DEFINE_MESSAGE_HEAP(FaultInd, 3);
DEFINE_MESSAGE_HEAP(PollingRequest, 2);
DEFINE_MESSAGE_HEAP(PollingResponse, 2);
DEFINE_MESSAGE_HEAP(UnexpectedMasterMessageInd, 2);

void* MasterDataMemoryManager_allocate(EMessageId messageId)
{
    void* allocatedMessage = NULL;
    
    ALLOCATE_MESSAGE_CALLOC_HANDLER(LogInd);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(FaultInd);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(PollingRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(PollingResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(UnexpectedMasterMessageInd);
    
    return NULL;
}

void MasterDataMemoryManager_free(EMessageId messageId, void* allocatedMemory)
{
    FREE_ALLOCATED_MESSAGE_HANDLER(LogInd);
    FREE_ALLOCATED_MESSAGE_HANDLER(FaultInd);
    FREE_ALLOCATED_MESSAGE_HANDLER(PollingRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(PollingResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(UnexpectedMasterMessageInd);
}

u8 MasterDataMemoryManager_getLength(EMessageId messageId)
{
    GET_MESSAGE_LENGTH(LogInd);
    GET_MESSAGE_LENGTH(FaultInd);
    GET_MESSAGE_LENGTH(PollingRequest);
    GET_MESSAGE_LENGTH(PollingResponse);
    GET_MESSAGE_LENGTH(UnexpectedMasterMessageInd);
    
    return 0;
}
