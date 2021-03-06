#include "MasterCommunication/MasterUartGateway.h"
#include "MasterCommunication/MasterDataMemoryManager.h"
#include "MasterCommunication/MasterDataManager.h"
#include "MasterCommunication/MasterDataTransmitter.h"

#include "SharedDefines/TMessage.h"
#include "SharedDefines/MessagesDefines.h"
#include "System/ThreadMacros.h"
#include "Utilities/CopyObject.h"

#include "cmsis_os.h"

static osMutexDef(mMutex);
static osMutexId mMutexId = NULL;

static u16 calculateCrcValue(u8 dataLength, TByte* data);
static const char* getLoggerPrefix(void);

void MasterUartGateway_setup(void)
{
    mMutexId = osMutexCreate(osMutex(mMutex));
}

void MasterUartGateway_initialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    Logger_debugSystem("MasterUartGateway: Initialized!");
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
    
    if (EMessageId_LogInd == messageType)
    {
        TLogInd* logInd = (TLogInd*) message;
        packedMessage.length = packedMessage.length - (MAX_LOG_SIZE - logInd->length);
    }
    
    packedMessage.crc = calculateCrcValue(packedMessage.length, packedMessage.data);
    
    Logger_debugSystem("MasterUartGateway: Message %s prepared and will be sent to Master.", CStringConverter_EMessageId(packedMessage.id));
    
    MasterDataTransmitter_transmitAsync(&packedMessage);
    
    osMutexRelease(mMutexId);
}

void MasterUartGateway_handleReceivedMessage(TMessage message)
{
    osMutexWait(mMutexId, osWaitForever);
    
    bool verifyingResult = ( calculateCrcValue(message.length, message.data) == message.crc );
    if (verifyingResult)
    {
        Logger_info("MasterUartGateway: Message %s received and passed CRC verification.", CStringConverter_EMessageId(message.id));
        
        CREATE_EVENT_ISR(DataFromMasterReceivedInd, EThreadId_MasterDataManager);
        CREATE_EVENT_MESSAGE(DataFromMasterReceivedInd);
        
        CopyObject_TMessage(&message, &(eventMessage->message));
        
        SEND_EVENT();
    }
    else
    {
        Logger_debugSystem("MasterUartGateway: Message %s received but CRC verification failed.", CStringConverter_EMessageId(message.id));
        MasterDataMemoryManager_free(message.id, message.data);
    }
    
    osMutexRelease(mMutexId);
}

u16 calculateCrcValue(u8 dataLength, TByte* data)
{
    return 0;
}

const char* getLoggerPrefix(void)
{
    static const char* loggerPrefix = "MasterUartGateway";
    return loggerPrefix;
}
