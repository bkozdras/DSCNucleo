#include "MasterCommunication/MasterDataReceiver.h"
#include "MasterCommunication/MasterUartGateway.h"
#include "MasterCommunication/MasterDataMemoryManager.h"

#include "Peripherals/UART1.h"
#include "SharedDefines/EMessagePart.h"
#include "SharedDefines/EMessageId.h"
#include "SharedDefines/TMessage.h"
#include "System/EventManagement/EEventId.h"
#include "Utilities/CopyObject.h"

#include "cmsis_os.h"

THREAD_DEFINES(MasterDataReceiver, MasterDataReceiver)
EVENT_HANDLER_PROTOTYPE(DataFromMasterReceivedInd)
EVENT_HANDLER_PROTOTYPE(StartReceivingData)

#define MESSAGES_BUFFER_SIZE 15

static TByte mMessageHeader [8];
static TByte mMessageEnd [4];
static TMessage mActiveMessage;
static EMessagePart mReceivingMessagePart = EMessagePart_End;
static bool mIsMessageCorrupted = true;

static void dataReceivedCallback(void);

THREAD(MasterDataReceiver)
{
    THREAD_SKELETON_START
    
        EVENT_HANDLING(DataFromMasterReceivedInd)
        EVENT_HANDLING(StartReceivingData)
    
    THREAD_SKELETON_END
}

EVENT_HANDLER(DataFromMasterReceivedInd)
{
    switch (mReceivingMessagePart)
    {
        case EMessagePart_Header :
        {
            mReceivingMessagePart = EMessagePart_Data;
            
            {
                if (! ( ('M' == mMessageHeader[0]) && ('S' == mMessageHeader[1]) && ('G' == mMessageHeader[2]) ) )
                {
                    mIsMessageCorrupted = true;
                }
            }
            
            {
                mActiveMessage.id = (EMessageId) ( mMessageHeader[3] );
                mActiveMessage.transactionId = mMessageHeader[4];
                mActiveMessage.crc = ( mMessageHeader[5] | ( ( ( (u16) ( mMessageHeader[6] )) << 8 ) & 0xFF00 ) );
                mActiveMessage.length = mMessageHeader[7];
                    
                mActiveMessage.data = (TByte*) ( MasterDataMemoryManager_allocate(mActiveMessage.id) );
            }
            
            if (!UART1_receive(mActiveMessage.data, mActiveMessage.length))
            {
                assert_param(0);
            }
            
            break;
        }
        
        case EMessagePart_Data :
        {
            mReceivingMessagePart = EMessagePart_End;
            
            if (!UART1_receive(mMessageEnd, 3))
            {
                assert_param(0);
            }
            
            break;
        }
        
        case EMessagePart_End :
        {
            mReceivingMessagePart = EMessagePart_Header;
            
            if (!UART1_receive(mMessageHeader, 8))
            {
                assert_param(0);
            }
            
            {
                if (! ( ('E' == mMessageEnd[0]) && ('N' == mMessageEnd[1]) && ('D' == mMessageEnd[2]) && ('\n' == mMessageEnd[3]) ) )
                {
                    mIsMessageCorrupted = true;
                }
            }
            
            if (!mIsMessageCorrupted)
            {
                MasterUartGateway_handleReceivedMessage(mActiveMessage);
            }
            
            mIsMessageCorrupted = false;
            
            break;
        }
    }
}

EVENT_HANDLER(StartReceivingData)
{
    Logger_debugSystem("%s: Starting receiving data from Master...", getLoggerPrefix());
    
    mIsMessageCorrupted = false;
    mReceivingMessagePart = EMessagePart_Header;
    if (!UART1_receive(mMessageHeader, 8))
    {
        Logger_error("%s: Starting receiving data from Master failed! UART failure.", getLoggerPrefix());
        assert_param(0);
    }
    
    Logger_debugSystem("%s: Receiving data from Master started.", getLoggerPrefix());
}

void MasterDataReceiver_setup(void)
{
    THREAD_INITIALIZE_MUTEX
}

void MasterDataReceiver_initialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    UART1_registerDataReceivingDoneCallback(dataReceivedCallback);
    Logger_debug("%s: Initialized!", getLoggerPrefix());
    osMutexRelease(mMutexId);
}

void dataReceivedCallback(void)
{
    CREATE_EVENT_ISR(DataFromMasterReceivedInd, mThreadId);
    SEND_EVENT();
}
