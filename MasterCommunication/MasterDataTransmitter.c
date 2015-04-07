#include "MasterCommunication/MasterDataTransmitter.h"
#include "MasterCommunication/MasterDataMemoryManager.h"

#include "Peripherals/UART1.h"
#include "SharedDefines/EMessagePart.h"
#include "SharedDefines/EMessageId.h"
#include "SharedDefines/TMessage.h"
#include "System/EventManagement/EEventId.h"
#include "Utilities/CopyObject.h"
#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"
#include "cmsis_os.h"

THREAD_DEFINES(MasterDataTransmitter, MasterDataTransmitter)
EVENT_HANDLER_PROTOTYPE(DataToMasterTransmittedInd)
EVENT_HANDLER_PROTOTYPE(TransmitData)

#define MESSAGES_BUFFER_SIZE 20

static osMutexDef(mMutexBufferOverflow);
static osMutexId mMutexBufferOverflowId;

static TMessage mMessagesBuffer [MESSAGES_BUFFER_SIZE];
static TMessage* mTransmittingMessage = NULL;
static u8 mMessagesBufferHead = 0;
static u8 mMessagesBufferTail = 0;
static bool mIsTransmittionOngoing = false;
static EMessagePart mTransmittingMessagePart = EMessagePart_Header;
static TByte mMessageHeader [8];
static TByte mMessageEnd [4];

static void dataTransmittedCallback(void);
static u8 getNextBufferIndex(u8 index);

THREAD(MasterDataTransmitter)
{
    THREAD_SKELETON_START
    
        EVENT_HANDLING(DataToMasterTransmittedInd)
        EVENT_HANDLING(TransmitData)
    
    THREAD_SKELETON_END
}

EVENT_HANDLER(TransmitData)
{
    osMutexWait(mMutexBufferOverflowId, osWaitForever);
    
    mMessagesBufferHead = getNextBufferIndex(mMessagesBufferHead);
    
    Logger_debugSystem("%s: Processing with message from TX buffer: %u.", getLoggerPrefix(), mMessagesBufferHead);
    
    mTransmittingMessagePart = EMessagePart_Header;
    mTransmittingMessage = &(mMessagesBuffer[mMessagesBufferHead]);
    mIsTransmittionOngoing = true;
    
    mMessageHeader[0] = 'M';
    mMessageHeader[1] = 'S';
    mMessageHeader[2] = 'G';
    mMessageHeader[3] = mTransmittingMessage->id;
    mMessageHeader[4] = mTransmittingMessage->transactionId;
    mMessageHeader[5] = ( mTransmittingMessage->crc & 0xFF );
    mMessageHeader[6] = ( ( mTransmittingMessage->crc >> 8 ) & 0xFF );
    mMessageHeader[7] = mTransmittingMessage->length;
    
    for (u16 iter = 0; 8 > iter; ++iter)
    {
        Logger_debugSystem("%s: Header byte[%u]: 0x%02X.", getLoggerPrefix(), iter, mMessageHeader[iter]);
    }
    
    if (!UART1_transmit(mMessageHeader, 8))
    {
        mIsTransmittionOngoing = false;
        Logger_debugSystem("%s: Transmitting header failed (Message: %s).", getLoggerPrefix(), CStringConverter_EMessageId(mTransmittingMessage->id));
        assert_param(0);
    }
    
    Logger_debugSystem("%s: Transmitted header (Message: %s).", getLoggerPrefix(), CStringConverter_EMessageId(mTransmittingMessage->id));
}

EVENT_HANDLER(DataToMasterTransmittedInd)
{
    switch (mTransmittingMessagePart)
    {
        case EMessagePart_Header :
        {
            mTransmittingMessagePart = EMessagePart_Data;
            
            for (u16 iter = 0; mTransmittingMessage->length > iter; ++iter)
            {
                Logger_debugSystem("%s: Data byte[%u]: 0x%02X.", getLoggerPrefix(), iter, mTransmittingMessage->data[iter]);
            }
            
            if (!UART1_transmit(mTransmittingMessage->data, mTransmittingMessage->length))
            {
                mIsTransmittionOngoing = false;
                Logger_debugSystem("%s: Transmitting data failed (Message: %s).", getLoggerPrefix(), CStringConverter_EMessageId(mTransmittingMessage->id));
                assert_param(0);
            }
            
            Logger_debugSystem("%s: Transmitted data(Message: %s).", getLoggerPrefix(), CStringConverter_EMessageId(mTransmittingMessage->id));
            
            break;
        }
        
        case EMessagePart_Data :
        {
            mTransmittingMessagePart = EMessagePart_End;
            
            mMessageEnd[0] = 'E';
            mMessageEnd[1] = 'N';
            mMessageEnd[2] = 'D';
            mMessageEnd[3] = '\n';
            
            for (u16 iter = 0; 4 > iter; ++iter)
            {
                Logger_debugSystem("%s: End byte[%u]: 0x%02X.", getLoggerPrefix(), iter, mMessageEnd[iter]);
            }
            
            if (!UART1_transmit(mMessageEnd, 4))
            {
                mIsTransmittionOngoing = false;
                Logger_debugSystem("%s: Transmitting end of message failed (Message: %s).", getLoggerPrefix(), CStringConverter_EMessageId(mTransmittingMessage->id));
                assert_param(0);
            }
            
            Logger_debugSystem("%s: Transmitted end of message (Message: %s).", getLoggerPrefix(), CStringConverter_EMessageId(mTransmittingMessage->id));
            
            break;
        }
        
        case EMessagePart_End :
        {
            MasterDataMemoryManager_free(mTransmittingMessage->id, mTransmittingMessage->data);
            
            if (mMessagesBufferHead == mMessagesBufferTail)
            {
                mIsTransmittionOngoing = false;
                Logger_debugSystem
                (
                    "%s: Transmitting message done (Message: %s). No new messages in TX buffer.",
                    getLoggerPrefix(),
                    CStringConverter_EMessageId(mTransmittingMessage->id)
                );
            }
            else
            {
                Logger_debugSystem
                (
                    "%s: Transmitting message done (Message: %s). New message found in TX buffer.",
                    getLoggerPrefix(),
                    CStringConverter_EMessageId(mTransmittingMessage->id)
                );
                
                CREATE_EVENT_ISR(TransmitData, mThreadId);
                SEND_EVENT();
            }
            
            osMutexRelease(mMutexBufferOverflowId);
            
            break;
        }
    }
}

void MasterDataTransmitter_setup(void)
{
    THREAD_INITIALIZE_MUTEX
    mMutexBufferOverflowId = osMutexCreate(osMutex(mMutexBufferOverflow));
}

void MasterDataTransmitter_initialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    UART1_registerDataTransmittingDoneCallback(dataTransmittedCallback);
    Logger_debug("%s: Initialized!", getLoggerPrefix());
    osMutexRelease(mMutexId);
}

void MasterDataTransmitter_transmit(TMessage* message)
{
    osMutexWait(mMutexId, osWaitForever);
    
    Logger_debugSystem("%s: Transmitting message %s.", getLoggerPrefix(), CStringConverter_EMessageId(message->id));
    
    u8 nextTail = getNextBufferIndex(mMessagesBufferTail);
    while (nextTail == mMessagesBufferHead)
    {
        osMutexRelease(mMutexId);
        Logger_debugSystem("%s: TX buffer is full. Waiting for free place in buffer...", getLoggerPrefix());
        osMutexWait(mMutexBufferOverflowId, osWaitForever);
        osMutexWait(mMutexId, osWaitForever);
        nextTail = getNextBufferIndex(mMessagesBufferTail);
    }
    
    CopyObject_TMessage(message, &(mMessagesBuffer[nextTail]));
    mMessagesBufferTail = nextTail;
    Logger_debugSystem("%s: Copied message to TX buffer. Position: %u.", getLoggerPrefix(), mMessagesBufferTail);
    
    if (!mIsTransmittionOngoing)
    {
        mIsTransmittionOngoing = true;
        CREATE_EVENT_ISR(TransmitData, mThreadId);
        SEND_EVENT();
    }
    
    osMutexRelease(mMutexId);
}

void dataTransmittedCallback(void)
{
    CREATE_EVENT_ISR(DataToMasterTransmittedInd, mThreadId);
    SEND_EVENT();
}

u8 getNextBufferIndex(u8 index)
{
    ++index;
    if (MESSAGES_BUFFER_SIZE <= index)
    {
        return 0;
    }
    else
    {
        return index;
    }
}

#undef MESSAGES_BUFFER_SIZE
