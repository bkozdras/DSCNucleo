#include "MasterCommunication/MasterDataTransmitter.h"
#include "MasterCommunication/MasterDataMemoryManager.h"

#include "Peripherals/UART1.h"
#include "SharedDefines/EMessagePart.h"
#include "SharedDefines/EMessageId.h"
#include "SharedDefines/TMessage.h"
#include "System/EventManagement/EEventId.h"
#include "Utilities/CopyObject.h"
#include "cmsis_os.h"

THREAD_DEFINES(MasterDataTransmitter, MasterDataTransmitter)
EVENT_HANDLER_PROTOTYPE(DataToMasterTransmittedInd)
EVENT_HANDLER_PROTOTYPE(TransmitData)

#define MESSAGES_BUFFER_SIZE 15

static osMutexDef(mMutexBufferOverflow);
static osMutexId mMutexBufferOverflowId;

static TMessage mMessagesBuffer [15];
static TMessage* mTransmittingMessage = NULL;
static u8 mMessagesBufferHead = 0;
static u8 mMessagesBufferTail = 0;
static bool mIsTransmittionOngoing = false;
static EMessagePart mTransmittingMessagePart = EMessagePart_Header;

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
    osMutexWait(mMutexId, osWaitForever);
    
    mMessagesBufferHead = getNextBufferIndex(mMessagesBufferHead);
    
    osMutexWait(mMutexBufferOverflowId, osWaitForever);
    
    mTransmittingMessagePart = EMessagePart_Header;
    mTransmittingMessage = &(mMessagesBuffer[mMessagesBufferHead]);
    mIsTransmittionOngoing = true;
    
    TByte header [8];
    header[0] = 'M';
    header[1] = 'S';
    header[2] = 'G';
    header[3] = mTransmittingMessage->id;
    header[4] = mTransmittingMessage->transactionId;
    header[5] = ( mTransmittingMessage->crc & 0xFF );
    header[6] = ( ( mTransmittingMessage->crc >> 8 ) & 0xFF );
    header[7] = mTransmittingMessage->length;
    
    if (!UART1_transmit(header, 8))
    {
        mIsTransmittionOngoing = false;
        assert_param(0);
    }
    
    osMutexRelease(mMutexId);
}

EVENT_HANDLER(DataToMasterTransmittedInd)
{
    switch (mTransmittingMessagePart)
    {
        case EMessagePart_Header :
        {
            mTransmittingMessagePart = EMessagePart_Data;
            
            if (!UART1_transmit(mTransmittingMessage->data, mTransmittingMessage->length))
            {
                mIsTransmittionOngoing = false;
                assert_param(0);
            }
            
            break;
        }
        
        case EMessagePart_Data :
        {
            mTransmittingMessagePart = EMessagePart_End;
            
            TByte end [3];
            end[0] = 'E';
            end[1] = 'N';
            end[2] = 'D';
            
            if (!UART1_transmit(end, 3))
            {
                mIsTransmittionOngoing = false;
                assert_param(0);
            }
            
            break;
        }
        
        case EMessagePart_End :
        {
            MasterDataMemoryManager_free(mTransmittingMessage->id, mTransmittingMessage->data);
            
            if (mMessagesBufferHead == mMessagesBufferTail)
            {
                mIsTransmittionOngoing = false;
            }
            else
            {
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
    
    u8 nextTail = getNextBufferIndex(mMessagesBufferTail);
    while (nextTail == mMessagesBufferHead)
    {
        osMutexRelease(mMutexId);
        osMutexWait(mMutexBufferOverflowId, osWaitForever);
        osMutexWait(mMutexId, osWaitForever);
        nextTail = getNextBufferIndex(mMessagesBufferTail);
    }
    
    CopyObject_TMessage(message, &(mMessagesBuffer[nextTail]));
    mMessagesBufferTail = nextTail;
    
    if (!mIsTransmittionOngoing)
    {
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
