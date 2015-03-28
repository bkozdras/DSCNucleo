#include "MasterCommunication/MasterDataManager.h"
#include "MasterCommunication/MasterDataMemoryManager.h"

#include "SharedDefines/MessagesDefines.h"
#include "SharedDefines/EMessageId.h"
#include "SharedDefines/TMessage.h"
#include "SharedDefines/SFaultIndication.h"
#include "SharedDefines/SSampleCarrierData.h"
#include "SharedDefines/EUnitId.h"
#include "SharedDefines/SRTDPolynomialCoefficients.h"
#include "SharedDefines/ELogSeverity.h"

#include "System/EventManagement/EEventId.h"

#include "cmsis_os.h"

THREAD_DEFINES(MasterDataManager, MasterDataManager)
EVENT_HANDLER_PROTOTYPE(DataFromMasterReceivedInd)

#define HANDLE_REQUEST(request)         case EMessageId_##request :                                 \
                                        {                                                           \
                                            handle##request ( (T##request *) message->data);      \
                                            return;                                                 \
                                        }                                                           \

static void handleMasterData(TMessage* message);

// Requests from Master
static void handlePollingRequest(TPollingRequest* pollingRequest);

// Indications callbacks
static void logIndCallback(void);
static void faultIndCallback(SFaultIndication* faultIndication);
static void sampleCarrierDataIndCallback(SSampleCarrierData* sampleCarrierData);
static void heaterTemperatureIndCallback(float temperature);
static void referenceThermocoupleTemperatureIndCallback(float temperature);
static void unitReadyIndCallback(bool status);

THREAD(MasterDataManager)
{
    THREAD_SKELETON_START
    
        EVENT_HANDLING(DataFromMasterReceivedInd)
    
    THREAD_SKELETON_END
}

EVENT_HANDLER(DataFromMasterReceivedInd)
{
    EVENT_MESSAGE(DataFromMasterReceivedInd)
    
    handleMasterData(&(event->message));
}

void MasterDataManager_setup(void)
{
    THREAD_INITIALIZE_MUTEX
}

void MasterDataManager_initialize(void)
{
    Logger_debug("%s: Initialized!", getLoggerPrefix());
}

void handleMasterData(TMessage* message)
{
    switch(message->id)
    {
        HANDLE_REQUEST(PollingRequest)
        
        default :
            break;
    }
    
    MasterDataMemoryManager_free(message->id, message->data);
}

void handlePollingRequest(TPollingRequest* pollingRequest)
{
    
}

void logIndCallback(void)
{
    
}

void faultIndCallback(SFaultIndication* faultIndication)
{
    
}

void sampleCarrierDataIndCallback(SSampleCarrierData* sampleCarrierData)
{
    
}

void heaterTemperatureIndCallback(float temperature)
{
    
}

void referenceThermocoupleTemperatureIndCallback(float temperature)
{
    
}

void unitReadyIndCallback(bool status)
{
    
}
