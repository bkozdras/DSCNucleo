#include "MasterCommunication/MasterUartGateway.h"
#include "MasterCommunication/MasterDataMemoryManager.h"
#include "MasterCommunication/MasterDataManager.h"
#include "MasterCommunication/MasterDataTransmitter.h"

#include "SharedDefines/TMessage.h"
#include "System/ThreadMacros.h"
#include "Utilities/CopyObject.h"

#include "cmsis_os.h"

static osMutexDef(mMutex);
static osMutexId mMutexId = NULL;

static u16 calculateCrcValue(u8 dataLength, TByte* data);

void MasterUartGateway_setup(void)
{
    mMutexId = osMutexCreate(osMutex(mMutex));
}

void MasterUartGateway_initialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    Logger_debug("MasterUartGateway: Initialized!");
    osMutexRelease(mMutexId);
}

void MasterUartGateway_sendMessage(EMessageId messageType, void* message)
{
    osMutexWait(mMutexId, osWaitForever);
    
    TMessage packedMessage;
    packedMessage.id = messageType;
    packedMessage.transactionId = 0;
    packedMessage.data = message;
    packedMessage.length = MasterDataMemoryManager_getLength(messageType);
    packedMessage.crc = calculateCrcValue(packedMessage.length, packedMessage.data);
    
    MasterDataTransmitter_transmit(&packedMessage);
    
    osMutexRelease(mMutexId);
}

void MasterUartGateway_verifyReceivedMessage(TMessage message)
{
    bool verifyingResult = ( calculateCrcValue(message.length, message.data) == message.crc );
    if (verifyingResult)
    {
        CREATE_EVENT_ISR(DataFromMasterReceivedInd, EThreadId_MasterDataManager);
        CREATE_EVENT_MESSAGE(DataFromMasterReceivedInd);
        
        CopyObject_TMessage(&(eventMessage->message), &message);
        
        SEND_EVENT();
    }
}

u16 calculateCrcValue(u8 dataLength, TByte* data)
{
    return 0;
}
