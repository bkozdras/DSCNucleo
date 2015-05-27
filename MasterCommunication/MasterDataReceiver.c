#include "MasterCommunication/MasterDataReceiver.h"
#include "MasterCommunication/MasterUartGateway.h"
#include "MasterCommunication/MasterDataMemoryManager.h"

#include "Peripherals/UART1.h"
#include "SharedDefines/EMessagePart.h"
#include "SharedDefines/EMessageId.h"
#include "SharedDefines/TMessage.h"
#include "System/EventManagement/EEventId.h"
#include "Utilities/CopyObject.h"

#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"
#include "Peripherals/LED.h"
#include "FaultManagement/FaultIndication.h"

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
            {
                for (u8 iter = 0; 8 > iter; ++iter)
                {
                    Logger_debugSystemMasterDataExtended("%s: Header byte[%u]: 0x%02X.", getLoggerPrefix(), iter, mMessageHeader[iter]);
                }
            }
            
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
                 
                Logger_debugSystem("%s: Receiving %s message from Master device.", getLoggerPrefix(), CStringConverter_EMessageId(mActiveMessage.id));

                mActiveMessage.data = (TByte*) ( MasterDataMemoryManager_allocate(mActiveMessage.id) );

                if (NULL == mActiveMessage.data)
                {
                    Logger_error("%s: Allocating memory for message %s failed. System failure!", getLoggerPrefix(), CStringConverter_EMessageId(mActiveMessage.id));
                    FaultIndication_start(EFaultId_NoMemory, EUnitId_Nucleo, EUnitId_Empty);
                    mIsMessageCorrupted = true;
                }
            }
            
            if (!mIsMessageCorrupted)
            {
                if (!UART1_receive(mActiveMessage.data, mActiveMessage.length))
                {
                    assert_param(0);
                }
                
                mReceivingMessagePart = EMessagePart_Data;
                Logger_debugSystemMasterDataExtended("%s: Message part awaiting: %u.", getLoggerPrefix(), mReceivingMessagePart);
            }
            
            break;
        }
        
        case EMessagePart_Data :
        {
            mReceivingMessagePart = EMessagePart_End;
            /*
            if (UART1_receive(mMessageEnd, 4))
            {
                Logger_debugSystemMasterDataExtended("%s: Waiting for message END.", getLoggerPrefix());
                Logger_debugSystemMasterDataExtended("%s: Message part awaiting: %u.", getLoggerPrefix(), mReceivingMessagePart);
            }
            else
            {
                assert_param(0);
            }
            */
            {
                for (u8 iter = 0; mActiveMessage.length > iter; ++iter)
                {
                    Logger_debugSystemMasterDataExtended("%s: Data byte[%u]: 0x%02X.", getLoggerPrefix(), iter, mActiveMessage.data[iter]);
                }
            }
            
            //break;
        }
        
        case EMessagePart_End :
        {
            mReceivingMessagePart = EMessagePart_Header;
            Logger_debugSystemMasterDataExtended("%s: Message part awaiting: %u.", getLoggerPrefix(), mReceivingMessagePart);
            
            if (!UART1_receive(mMessageHeader, 8))
            {
                assert_param(0);
            }
            /*
            {
                for (u8 iter = 0; 4 > iter; ++iter)
                {
                    Logger_debugSystemMasterDataExtended("%s: End byte[%u]: 0x%02X.", getLoggerPrefix(), iter, mMessageEnd[iter]);
                }
            }
            
            {
                if (! ( ('E' == mMessageEnd[0]) && ('N' == mMessageEnd[1]) && ('D' == mMessageEnd[2]) && ('\n' == mMessageEnd[3]) ) )
                {
                    mIsMessageCorrupted = true;
                }
            }*/
            
            if (!mIsMessageCorrupted)
            {
                MasterUartGateway_handleReceivedMessage(mActiveMessage);
            }
            else
            {
                Logger_error("%s: Message %s is corrupted and will be skipped.", getLoggerPrefix(), CStringConverter_EMessageId(mActiveMessage.id));
                MasterDataMemoryManager_free(mActiveMessage.id, mActiveMessage.data);
            }
            
            mIsMessageCorrupted = false;
            
            break;
        }
        
        default :
            break;
    }
}

EVENT_HANDLER(StartReceivingData)
{
    Logger_debugSystem("%s: Starting receiving data from Master...", getLoggerPrefix());
    
    mIsMessageCorrupted = false;
    mReceivingMessagePart = EMessagePart_Header;
    Logger_debugSystemMasterDataExtended("%s: Message part awaiting: %u.", getLoggerPrefix(), mReceivingMessagePart);
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
    static u8 iter = 0;
    Logger_debugSystem("%s: Data received callback. Iteration: %u.", getLoggerPrefix(), ++iter);
    CREATE_EVENT_ISR(DataFromMasterReceivedInd, mThreadId);
    SEND_EVENT();
}
