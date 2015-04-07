#include "MasterCommunication/MasterDataManager.h"
#include "MasterCommunication/MasterDataMemoryManager.h"
#include "MasterCommunication/MasterUartGateway.h"

#include "FaultManagement/FaultIndication.h"

#include "SharedDefines/MessagesDefines.h"
#include "SharedDefines/EMessageId.h"
#include "SharedDefines/TMessage.h"
#include "SharedDefines/SFaultIndication.h"
#include "SharedDefines/SSampleCarrierData.h"
#include "SharedDefines/EUnitId.h"
#include "SharedDefines/SRTDPolynomialCoefficients.h"
#include "SharedDefines/ELogSeverity.h"
#include "SharedDefines/SControllerData.h"

#include "System/EventManagement/EEventId.h"
#include "System/SystemManager.h"

#include "Devices/ADS1248.h"
#include "Devices/LMP90100ControlSystem.h"
#include "Devices/LMP90100SignalsMeasurement.h"
#include "Devices/MCP4716.h"

#include "Controller/HeaterTemperatureController.h"
#include "Controller/HeaterTemperatureReader.h"
#include "Controller/ReferenceTemperatureController.h"
#include "Controller/ReferenceTemperatureReader.h"
#include "Controller/SampleCarrierDataManager.h"
#include "Controller/SegmentsManager.h"

#include "Utilities/Printer/CStringConverter.h"
#include "Utilities/Logger/Logger.h"
#include "Utilities/CopyObject.h"

#include "cmsis_os.h"

THREAD_DEFINES(MasterDataManager, MasterDataManager)
EVENT_HANDLER_PROTOTYPE(DataFromMasterReceivedInd)

#define HANDLE_REQUEST(request)         case EMessageId_##request :                                 \
                                        {                                                           \
                                            handle##request ( (T##request *) message->data);        \
                                            return;                                                 \
                                        }                                                           \

static void handleMasterData(TMessage* message);

// Requests from Master
static void handlePollingRequest(TPollingRequest* request);
static void handleResetUnitRequest(TResetUnitRequest* request);
static void handleSetHeaterPowerRequest(TSetHeaterPowerRequest* request);
static void handleCallibreADS1248Request(TCallibreADS1248Request* request);
static void handleSetChannelGainADS1248Request(TSetChannelGainADS1248Request* request);
static void handleSetChannelSamplingSpeedADS1248Request(TSetChannelSamplingSpeedADS1248Request* request);
static void handleStartRegisteringDataRequest(TStartRegisteringDataRequest* request);
static void handleStopRegisteringDataRequest(TStopRegisteringDataRequest* request);
static void handleSetNewDeviceModeLMP90100ControlSystemRequest(TSetNewDeviceModeLMP90100ControlSystemRequest* request);
static void handleSetNewDeviceModeLMP90100SignalsMeasurementRequest(TSetNewDeviceModeLMP90100SignalsMeasurementRequest* request);
static void handleSetControlSystemTypeRequest(TSetControlSystemTypeRequest* request);
static void handleSetControllerTunesRequest(TSetControllerTunesRequest* request);
static void handleSetProcessModelParametersRequest(TSetProcessModelParametersRequest* request);
static void handleSetControllingAlgorithmExecutionPeriodRequest(TSetControllingAlgorithmExecutionPeriodRequest* request);
static void handleRegisterNewSegmentToProgramRequest(TRegisterNewSegmentToProgramRequest* request);
static void handleDeregisterSegmentFromProgramRequest(TDeregisterSegmentFromProgramRequest* request);
static void handleStartSegmentProgramRequest(TStartSegmentProgramRequest* request);
static void handleStopSegmentProgramRequest(TStopSegmentProgramRequest* request);
static void handleStartReferenceTemperatureStabilizationRequest(TStartReferenceTemperatureStabilizationRequest* request);
static void handleStopReferenceTemperatureStabilizationRequest(TStopReferenceTemperatureStabilizationRequest* request);
static void handleSetRTDPolynomialCoefficientsRequest(TSetRTDPolynomialCoefficientsRequest* request);

static void handleUnexpectedMessage(u8 messageId);

// Indications to Master callbacks
static void logIndCallback(TLogInd* logInd);
static void faultIndCallback(SFaultIndication* faultIndication);
static void sampleCarrierDataIndCallback(SSampleCarrierData* sampleCarrierData);
static void heaterTemperatureIndCallback(float temperature);
static void referenceTemperatureIndCallback(float temperature);
static void controllerDataIndCallback(SControllerData* controllerData);
static void segmentStartedInd(u8 segmentNumber);
static void segmentsProgramDoneInd(u16 realizedSegmentsCount, u16 lastSegmentDoneNumber);
static void unitReadyIndCallback(EUnitId unitId, bool status);

// Delayed responses to Master

static void callibreADS1248ResponseCallback(EADS1248CallibrationType type, bool success);

/****************************************************************************************************************************************************/

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
    osMutexWait(mMutexId, osWaitForever);
    
    //HeaterTemperatureController_registerNewControllerDataCallback(controllerDataIndCallback);
    //HeaterTemperatureReader_registerNewTemperatureValueCallback(heaterTemperatureIndCallback);
    //ReferenceTemperatureReader_registerDataReadyCallback(referenceTemperatureIndCallback);
    //SampleCarrierDataManager_registerDataReadyCallback(sampleCarrierDataIndCallback);
    //FaultIndication_registerNewFaultCallback(faultIndCallback);
    //SystemManager_registerUnitReadyIndCallback(unitReadyIndCallback);
    //Logger_registerMasterMessageLogIndCallback(logIndCallback);
    
    Logger_info("%s: Initialized!", getLoggerPrefix());
    osMutexRelease(mMutexId);
}

void handleMasterData(TMessage* message)
{
    switch(message->id)
    {
        HANDLE_REQUEST(PollingRequest)
        HANDLE_REQUEST(ResetUnitRequest)
        HANDLE_REQUEST(SetHeaterPowerRequest)
        HANDLE_REQUEST(CallibreADS1248Request)
        HANDLE_REQUEST(SetChannelGainADS1248Request)
        HANDLE_REQUEST(SetChannelSamplingSpeedADS1248Request)
        HANDLE_REQUEST(StartRegisteringDataRequest)
        HANDLE_REQUEST(StopRegisteringDataRequest)
        HANDLE_REQUEST(SetNewDeviceModeLMP90100ControlSystemRequest)
        HANDLE_REQUEST(SetNewDeviceModeLMP90100SignalsMeasurementRequest)
        HANDLE_REQUEST(SetControlSystemTypeRequest)
        HANDLE_REQUEST(SetControllerTunesRequest)
        HANDLE_REQUEST(SetProcessModelParametersRequest)
        HANDLE_REQUEST(SetControllingAlgorithmExecutionPeriodRequest)
        HANDLE_REQUEST(RegisterNewSegmentToProgramRequest)
        HANDLE_REQUEST(DeregisterSegmentFromProgramRequest)
        HANDLE_REQUEST(StartSegmentProgramRequest)
        HANDLE_REQUEST(StopSegmentProgramRequest)
        HANDLE_REQUEST(StartReferenceTemperatureStabilizationRequest)
        HANDLE_REQUEST(StopReferenceTemperatureStabilizationRequest)
        HANDLE_REQUEST(SetRTDPolynomialCoefficientsRequest)
        
        default :
            handleUnexpectedMessage(message->id);
            break;
    }
    
    MasterDataMemoryManager_free(message->id, message->data);
}

// Requests from Master
void handlePollingRequest(TPollingRequest* request)
{
    TPollingResponse* response = MasterDataMemoryManager_allocate(EMessageId_PollingResponse);
    response->success = true;
    MasterUartGateway_sendMessage(EMessageId_PollingResponse, response);
}

void handleResetUnitRequest(TResetUnitRequest* request)
{
    TResetUnitResponse* response = MasterDataMemoryManager_allocate(EMessageId_ResetUnitResponse);
    response->success = false;
    response->unitId = request->unitId;
    MasterUartGateway_sendMessage(EMessageId_ResetUnitResponse, response);
}

void handleSetHeaterPowerRequest(TSetHeaterPowerRequest* request)
{
    TSetHeaterPowerResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetHeaterPowerResponse);
    
    MasterUartGateway_sendMessage(EMessageId_SetHeaterPowerResponse, response);
}

void handleCallibreADS1248Request(TCallibreADS1248Request* request)
{
    
}

void handleSetChannelGainADS1248Request(TSetChannelGainADS1248Request* request)
{
    TSetChannelGainADS1248Response* response = MasterDataMemoryManager_allocate(EMessageId_SetChannelGainADS1248Response);
    
    MasterUartGateway_sendMessage(EMessageId_SetChannelGainADS1248Response, response);
}

void handleSetChannelSamplingSpeedADS1248Request(TSetChannelSamplingSpeedADS1248Request* request)
{
    TSetChannelSamplingSpeedADS1248Response* response = MasterDataMemoryManager_allocate(EMessageId_SetChannelSamplingSpeedADS1248Response);
    
    MasterUartGateway_sendMessage(EMessageId_SetChannelSamplingSpeedADS1248Response, response);
}

void handleStartRegisteringDataRequest(TStartRegisteringDataRequest* request)
{
    TStartRegisteringDataResponse* response = MasterDataMemoryManager_allocate(EMessageId_StartRegisteringDataResponse);
    
    MasterUartGateway_sendMessage(EMessageId_StartRegisteringDataResponse, response);
}

void handleStopRegisteringDataRequest(TStopRegisteringDataRequest* request)
{
    TStopRegisteringDataResponse* response = MasterDataMemoryManager_allocate(EMessageId_StopRegisteringDataResponse);
    
    MasterUartGateway_sendMessage(EMessageId_StopRegisteringDataResponse, response);
}

void handleSetNewDeviceModeLMP90100ControlSystemRequest(TSetNewDeviceModeLMP90100ControlSystemRequest* request)
{
    TSetNewDeviceModeLMP90100ControlSystemResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetNewDeviceModeLMP90100ControlSystemResponse);
    
    MasterUartGateway_sendMessage(EMessageId_SetNewDeviceModeLMP90100ControlSystemResponse, response);
}

void handleSetNewDeviceModeLMP90100SignalsMeasurementRequest(TSetNewDeviceModeLMP90100SignalsMeasurementRequest* request)
{
    TSetNewDeviceModeLMP90100SignalsMeasurementResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetNewDeviceModeLMP90100SignalsMeasurementResponse);
    
    MasterUartGateway_sendMessage(EMessageId_SetNewDeviceModeLMP90100SignalsMeasurementResponse, response);
}

void handleSetControlSystemTypeRequest(TSetControlSystemTypeRequest* request)
{
    TSetControlSystemTypeResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetControlSystemTypeResponse);
    
    MasterUartGateway_sendMessage(EMessageId_SetControlSystemTypeResponse, response);
}

void handleSetControllerTunesRequest(TSetControllerTunesRequest* request)
{
    TResetUnitResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetControllerTunesResponse);
    
    MasterUartGateway_sendMessage(EMessageId_SetControllerTunesResponse, response);
}

void handleSetProcessModelParametersRequest(TSetProcessModelParametersRequest* request)
{
    TSetProcessModelParametersResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetProcessModelParametersResponse);
    
    MasterUartGateway_sendMessage(EMessageId_SetProcessModelParametersResponse, response);
}

void handleSetControllingAlgorithmExecutionPeriodRequest(TSetControllingAlgorithmExecutionPeriodRequest* request)
{
    TSetControllingAlgorithmExecutionPeriodResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetControllingAlgorithmExecutionPeriodResponse);
    
    MasterUartGateway_sendMessage(EMessageId_SetControllingAlgorithmExecutionPeriodResponse, response);
}

void handleRegisterNewSegmentToProgramRequest(TRegisterNewSegmentToProgramRequest* request)
{
    TRegisterNewSegmentToProgramResponse* response = MasterDataMemoryManager_allocate(EMessageId_RegisterNewSegmentToProgramResponse);
    
    MasterUartGateway_sendMessage(EMessageId_RegisterNewSegmentToProgramResponse, response);
}

void handleDeregisterSegmentFromProgramRequest(TDeregisterSegmentFromProgramRequest* request)
{
    TDeregisterSegmentFromProgramResponse* response = MasterDataMemoryManager_allocate(EMessageId_DeregisterSegmentFromProgramResponse);
    
    MasterUartGateway_sendMessage(EMessageId_DeregisterSegmentFromProgramResponse, response);
}

void handleStartSegmentProgramRequest(TStartSegmentProgramRequest* request)
{
    TStartSegmentProgramResponse* response = MasterDataMemoryManager_allocate(EMessageId_StartSegmentProgramResponse);
    
    MasterUartGateway_sendMessage(EMessageId_StartSegmentProgramResponse, response);
}

void handleStopSegmentProgramRequest(TStopSegmentProgramRequest* request)
{
    TStopSegmentProgramResponse* response = MasterDataMemoryManager_allocate(EMessageId_StopSegmentProgramResponse);
    
    MasterUartGateway_sendMessage(EMessageId_StopSegmentProgramResponse, response);
}

void handleStartReferenceTemperatureStabilizationRequest(TStartReferenceTemperatureStabilizationRequest* request)
{
    TStartReferenceTemperatureStabilizationResponse* response =
        MasterDataMemoryManager_allocate(EMessageId_StartReferenceTemperatureStabilizationResponse);
    
    MasterUartGateway_sendMessage(EMessageId_StartReferenceTemperatureStabilizationResponse, response);
}

void handleStopReferenceTemperatureStabilizationRequest(TStopReferenceTemperatureStabilizationRequest* request)
{
    TStopReferenceTemperatureStabilizationResponse* response =
        MasterDataMemoryManager_allocate(EMessageId_StopReferenceTemperatureStabilizationResponse);
    
    MasterUartGateway_sendMessage(EMessageId_StopReferenceTemperatureStabilizationResponse, response);
}

void handleSetRTDPolynomialCoefficientsRequest(TSetRTDPolynomialCoefficientsRequest* request)
{
    TSetRTDPolynomialCoefficientsResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetRTDPolynomialCoefficientsResponse);
    
    MasterUartGateway_sendMessage(EMessageId_SetRTDPolynomialCoefficientsResponse, response);
}

void handleUnexpectedMessage(u8 messageId)
{
    TUnexpectedMasterMessageInd* indication = MasterDataMemoryManager_allocate(EMessageId_UnexpectedMasterMessageInd);
    indication->id = messageId;
    MasterUartGateway_sendMessage(EMessageId_UnexpectedMasterMessageInd, indication);
}

// Indications to Master callbacks
void logIndCallback(TLogInd* logInd)
{
    MasterUartGateway_sendMessage(EMessageId_LogInd, logInd);
}

void faultIndCallback(SFaultIndication* faultIndication)
{
    TFaultInd* indication = MasterDataMemoryManager_allocate(EMessageId_FaultInd);
    CopyObject_SFaultIndication(faultIndication, &(indication->indication));
    MasterUartGateway_sendMessage(EMessageId_FaultInd, indication);
}

void sampleCarrierDataIndCallback(SSampleCarrierData* sampleCarrierData)
{
    TSampleCarrierDataInd* indication = MasterDataMemoryManager_allocate(EMessageId_SampleCarrierDataInd);
    CopyObject_SSampleCarrierData(sampleCarrierData, &(indication->data));
    MasterUartGateway_sendMessage(EMessageId_SampleCarrierDataInd, indication);
}

void heaterTemperatureIndCallback(float temperature)
{
    THeaterTemperatureInd* indication = MasterDataMemoryManager_allocate(EMessageId_HeaterTemperatureInd);
    indication->temperature = temperature;
    MasterUartGateway_sendMessage(EMessageId_HeaterTemperatureInd, indication);
}

void referenceTemperatureIndCallback(float temperature)
{
    TReferenceTemperatureInd* indication = MasterDataMemoryManager_allocate(EMessageId_ReferenceTemperatureInd);
    indication->temperature = temperature;
    MasterUartGateway_sendMessage(EMessageId_ReferenceTemperatureInd, indication);
}

void controllerDataIndCallback(SControllerData* controllerData)
{
    TControllerDataInd* indication = MasterDataMemoryManager_allocate(EMessageId_ControllerDataInd);
    CopyObject_SControllerData(controllerData, &(indication->data));
    MasterUartGateway_sendMessage(EMessageId_ControllerDataInd, indication);
}

void segmentsProgramDoneInd(u16 realizedSegmentsCount, u16 lastSegmentDoneNumber)
{
    TSegmentsProgramDoneInd* indication = MasterDataMemoryManager_allocate(EMessageId_SegmentsProgramDoneInd);
    indication->numberOfLastDoneSegment = lastSegmentDoneNumber;
    indication->realizedSegmentsCount = realizedSegmentsCount;
    MasterUartGateway_sendMessage(EMessageId_SegmentsProgramDoneInd, indication);
}

void segmentStartedInd(u8 segmentNumber)
{
    TSegmentStartedInd* indication = MasterDataMemoryManager_allocate(EMessageId_SegmentStartedInd);
    indication->segmentNumber = segmentNumber;
    MasterUartGateway_sendMessage(EMessageId_SegmentStartedInd, indication);
}

void unitReadyIndCallback(EUnitId unitId, bool status)
{
    TUnitReadyInd* indication = MasterDataMemoryManager_allocate(EMessageId_UnitReadyInd);
    indication->unitId = unitId;
    indication->success = status;
    MasterUartGateway_sendMessage(EMessageId_UnitReadyInd, indication);
}

// Delayed responses to Master

void callibreADS1248ResponseCallback(EADS1248CallibrationType type, bool success)
{
    TCallibreADS1248Response* response = MasterDataMemoryManager_allocate(EMessageId_CallibreADS1248Response);
    response->callibrationType = type;
    response->success = success;
    MasterUartGateway_sendMessage(EMessageId_CallibreADS1248Response, response);
}
