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
#include "SharedDefines/EControllerDataType.h"

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
                                            break;                                                  \
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
static void handleSetNewDeviceModeADS1248Request(TSetNewDeviceModeADS1248Request* request);
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
static void handleSetHeaterTemperatureInFeedbackModeRequest(TSetHeaterTemperatureInFeedbackModeRequest* request);

static void handleUnexpectedMessage(u8 messageId);

// Indications to Master callbacks
static void logIndCallback(TLogInd* logInd);
static void faultIndCallback(SFaultIndication* faultIndication);
static void sampleCarrierDataIndCallback(SSampleCarrierData* sampleCarrierData);
static void heaterTemperatureIndCallback(float temperature);
static void referenceTemperatureIndCallback(float temperature);
//static void controllerDataIndCallback(SControllerData* controllerData);
static void controllerDataIndCallback(EControllerDataType type, float value);
static void segmentStartedInd(u16 segmentNumber, u8 leftRegisteredSegments);
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
    
    FaultIndication_registerNewFaultCallback(faultIndCallback);
    SystemManager_registerUnitReadyIndCallback(unitReadyIndCallback);
    //Logger_registerMasterMessageLogIndCallback(logIndCallback);
    SegmentsManager_registerSegmentsProgramDoneIndCallback(segmentsProgramDoneInd);
    SegmentsManager_registerSegmentStartedIndCallback(segmentStartedInd);
    
    Logger_info("%s: Initialized!", getLoggerPrefix());
    osMutexRelease(mMutexId);
}

void handleMasterData(TMessage* message)
{
    Logger_debugSystem("%s: Handling Master message: %s.", getLoggerPrefix(), CStringConverter_EMessageId(message->id));
    
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
        HANDLE_REQUEST(SetNewDeviceModeADS1248Request)
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
        HANDLE_REQUEST(SetHeaterTemperatureInFeedbackModeRequest)
        
        default :
            handleUnexpectedMessage(message->id);
            break;
    }
    
    Logger_debugSystem("%s: Request from Master proceeded and message %s will be freed.", getLoggerPrefix(), CStringConverter_EMessageId(message->id));
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
    response->power = request->power;
    response->success = HeaterTemperatureController_setPowerInPercent(request->power);
    MasterUartGateway_sendMessage(EMessageId_SetHeaterPowerResponse, response);
}

void handleCallibreADS1248Request(TCallibreADS1248Request* request)
{
    if (!ADS1248_startCallibration(request->callibrationType, callibreADS1248ResponseCallback))
    {
        TCallibreADS1248Response* response = MasterDataMemoryManager_allocate(EMessageId_CallibreADS1248Response);
        response->callibrationType = request->callibrationType;
        response->success = false;
    }
}

void handleSetChannelGainADS1248Request(TSetChannelGainADS1248Request* request)
{
    TSetChannelGainADS1248Response* response = MasterDataMemoryManager_allocate(EMessageId_SetChannelGainADS1248Response);
    response->value = request->value;
    response->success = ADS1248_setChannelGain(request->value);
    MasterUartGateway_sendMessage(EMessageId_SetChannelGainADS1248Response, response);
}

void handleSetChannelSamplingSpeedADS1248Request(TSetChannelSamplingSpeedADS1248Request* request)
{
    TSetChannelSamplingSpeedADS1248Response* response = MasterDataMemoryManager_allocate(EMessageId_SetChannelSamplingSpeedADS1248Response);
    response->value = request->value;
    response->success = ADS1248_setChannelSamplingSpeed(request->value);
    MasterUartGateway_sendMessage(EMessageId_SetChannelSamplingSpeedADS1248Response, response);
}

void handleStartRegisteringDataRequest(TStartRegisteringDataRequest* request)
{
    TStartRegisteringDataResponse* response = MasterDataMemoryManager_allocate(EMessageId_StartRegisteringDataResponse);
    
    response->dataType = request->dataType;
    response->success = true;
    
    switch(request->dataType)
    {
        case ERegisteringDataType_ControllerData :
        {   
            response->success = HeaterTemperatureController_registerNewControllerDataCallback(controllerDataIndCallback, request->period);
            break;
        }
        
        case ERegisteringDataType_HeaterTemperature :
        {
            response->success = HeaterTemperatureReader_registerNewTemperatureValueCallback(heaterTemperatureIndCallback, request->period);
            break;
        }
        
        case ERegisteringDataType_ReferenceTemperature :
        {   
            ReferenceTemperatureReader_registerDataReadyCallback(referenceTemperatureIndCallback);
            break;
        }
        
        case ERegisteringDataType_SampleCarrierData :
        {
            if (!ADS1248_isReadingStarted())
            {
                response->success = ADS1248_startReading();
            }
            
            if (response->success)
            {
                SampleCarrierDataManager_registerDataReadyCallback(sampleCarrierDataIndCallback);
            }
            
            break;
        }
        
        case ERegisteringDataType_All :
        {
            response->success = HeaterTemperatureController_registerNewControllerDataCallback(controllerDataIndCallback, 1000U);
            HeaterTemperatureReader_registerNewTemperatureValueCallback(heaterTemperatureIndCallback, 1000U);
            ReferenceTemperatureReader_registerDataReadyCallback(referenceTemperatureIndCallback);
            SampleCarrierDataManager_registerDataReadyCallback(sampleCarrierDataIndCallback);
            break;
        }
    }
    
    MasterUartGateway_sendMessage(EMessageId_StartRegisteringDataResponse, response);
}

void handleStopRegisteringDataRequest(TStopRegisteringDataRequest* request)
{
    TStopRegisteringDataResponse* response = MasterDataMemoryManager_allocate(EMessageId_StopRegisteringDataResponse);
    
    response->dataType = request->dataType;
    response->success = true;
    
    switch(request->dataType)
    {
        case ERegisteringDataType_ControllerData :
        {   
            HeaterTemperatureController_deregisterNewControllerDataCallback();
            break;
        }
        
        case ERegisteringDataType_HeaterTemperature :
        {
            response->success = HeaterTemperatureReader_deregisterNewTemperatureValueCallback();
            break;
        }
        
        case ERegisteringDataType_ReferenceTemperature :
        {
            ReferenceTemperatureReader_deregisterDataReadyCallback();
            break;
        }
        
        case ERegisteringDataType_SampleCarrierData :
        {
            SampleCarrierDataManager_deregisterDataReadyCallback();
            break;
        }
        
        case ERegisteringDataType_All :
        {
            HeaterTemperatureController_deregisterNewControllerDataCallback();
            HeaterTemperatureReader_deregisterNewTemperatureValueCallback();
            ReferenceTemperatureReader_deregisterDataReadyCallback();
            SampleCarrierDataManager_deregisterDataReadyCallback();
            break;
        }
    }
    
    MasterUartGateway_sendMessage(EMessageId_StopRegisteringDataResponse, response);
}

void handleSetNewDeviceModeADS1248Request(TSetNewDeviceModeADS1248Request* request)
{
    TSetNewDeviceModeADS1248Response* response = MasterDataMemoryManager_allocate(EMessageId_SetNewDeviceModeADS1248Response);
    
    response->mode = request->mode;
    response->success = ADS1248_changeMode(request->mode);
    
    MasterUartGateway_sendMessage(EMessageId_SetNewDeviceModeADS1248Response, response);
}

void handleSetNewDeviceModeLMP90100ControlSystemRequest(TSetNewDeviceModeLMP90100ControlSystemRequest* request)
{
    TSetNewDeviceModeLMP90100ControlSystemResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetNewDeviceModeLMP90100ControlSystemResponse);
    
    response->mode = request->mode;
    response->success = LMP90100ControlSystem_changeMode(request->mode);
    
    MasterUartGateway_sendMessage(EMessageId_SetNewDeviceModeLMP90100ControlSystemResponse, response);
}

void handleSetNewDeviceModeLMP90100SignalsMeasurementRequest(TSetNewDeviceModeLMP90100SignalsMeasurementRequest* request)
{
    TSetNewDeviceModeLMP90100SignalsMeasurementResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetNewDeviceModeLMP90100SignalsMeasurementResponse);
    
    response->mode = request->mode;
    response->success = LMP90100SignalsMeasurement_changeMode(request->mode);
    
    MasterUartGateway_sendMessage(EMessageId_SetNewDeviceModeLMP90100SignalsMeasurementResponse, response);
}

void handleSetControlSystemTypeRequest(TSetControlSystemTypeRequest* request)
{
    TSetControlSystemTypeResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetControlSystemTypeResponse);
    
    response->type = request->type;
    response->success = HeaterTemperatureController_setSystemType(request->type);
    
    MasterUartGateway_sendMessage(EMessageId_SetControlSystemTypeResponse, response);
}

void handleSetControllerTunesRequest(TSetControllerTunesRequest* request)
{
    TSetControllerTunesResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetControllerTunesResponse);
    response->pid = request->pid;
    response->success = HeaterTemperatureController_setTunes(request->pid, &(request->tunes));
    MasterUartGateway_sendMessage(EMessageId_SetControllerTunesResponse, response);
}

void handleSetProcessModelParametersRequest(TSetProcessModelParametersRequest* request)
{
    TSetProcessModelParametersResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetProcessModelParametersResponse);
    
    response->parameters = request->parameters;
    response->success = HeaterTemperatureController_setProcessModelParameters(&(request->parameters));
    
    MasterUartGateway_sendMessage(EMessageId_SetProcessModelParametersResponse, response);
}

void handleSetControllingAlgorithmExecutionPeriodRequest(TSetControllingAlgorithmExecutionPeriodRequest* request)
{
    TSetControllingAlgorithmExecutionPeriodResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetControllingAlgorithmExecutionPeriodResponse);
    
    response->value = request->value;
    response->success = HeaterTemperatureController_setAlgorithmExecutionPeriod(request->value);
    
    MasterUartGateway_sendMessage(EMessageId_SetControllingAlgorithmExecutionPeriodResponse, response);
}

void handleRegisterNewSegmentToProgramRequest(TRegisterNewSegmentToProgramRequest* request)
{
    TRegisterNewSegmentToProgramResponse* response = MasterDataMemoryManager_allocate(EMessageId_RegisterNewSegmentToProgramResponse);
    
    response->segmentNumber = request->segment.number;
    response->success = SegmentsManager_registerNewSegment(&(request->segment));
    
    MasterUartGateway_sendMessage(EMessageId_RegisterNewSegmentToProgramResponse, response);
}

void handleDeregisterSegmentFromProgramRequest(TDeregisterSegmentFromProgramRequest* request)
{
    TDeregisterSegmentFromProgramResponse* response = MasterDataMemoryManager_allocate(EMessageId_DeregisterSegmentFromProgramResponse);
    
    response->segmentNumber = request->segmentNumber;
    response->success = SegmentsManager_deregisterSegment(request->segmentNumber);
    response->numberOfRegisteredSegments = SegmentsManager_getNumberOfRegisteredSegments();
    
    MasterUartGateway_sendMessage(EMessageId_DeregisterSegmentFromProgramResponse, response);
}

void handleStartSegmentProgramRequest(TStartSegmentProgramRequest* request)
{
    TStartSegmentProgramResponse* response = MasterDataMemoryManager_allocate(EMessageId_StartSegmentProgramResponse);
    
    //if (!ADS1248_isReadingStarted())
    //{
    //    response->success = ADS1248_startReading();
    //}
    //if (response->success)
    {
        response->success = SegmentsManager_startProgram();
    }
    
    MasterUartGateway_sendMessage(EMessageId_StartSegmentProgramResponse, response);
}

void handleStopSegmentProgramRequest(TStopSegmentProgramRequest* request)
{
    TStopSegmentProgramResponse* response = MasterDataMemoryManager_allocate(EMessageId_StopSegmentProgramResponse);
    
    response->success = ADS1248_stopReading();
    if (response->success)
    {
        response->success = SegmentsManager_stopProgram();
    }
    
    MasterUartGateway_sendMessage(EMessageId_StopSegmentProgramResponse, response);
}

void handleStartReferenceTemperatureStabilizationRequest(TStartReferenceTemperatureStabilizationRequest* request)
{
    TStartReferenceTemperatureStabilizationResponse* response =
        MasterDataMemoryManager_allocate(EMessageId_StartReferenceTemperatureStabilizationResponse);
    
    response->success = ReferenceTemperatureController_startStabilization();
    
    MasterUartGateway_sendMessage(EMessageId_StartReferenceTemperatureStabilizationResponse, response);
}

void handleStopReferenceTemperatureStabilizationRequest(TStopReferenceTemperatureStabilizationRequest* request)
{
    TStopReferenceTemperatureStabilizationResponse* response =
        MasterDataMemoryManager_allocate(EMessageId_StopReferenceTemperatureStabilizationResponse);
    
    ReferenceTemperatureController_stopStabilization();
    response->success = true;
    
    MasterUartGateway_sendMessage(EMessageId_StopReferenceTemperatureStabilizationResponse, response);
}

void handleSetRTDPolynomialCoefficientsRequest(TSetRTDPolynomialCoefficientsRequest* request)
{
    TSetRTDPolynomialCoefficientsResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetRTDPolynomialCoefficientsResponse);
    
    SampleCarrierManager_setRTDPolynomialCoefficients(&(request->coefficients));
    response->success = true;
    
    MasterUartGateway_sendMessage(EMessageId_SetRTDPolynomialCoefficientsResponse, response);
}

void handleSetHeaterTemperatureInFeedbackModeRequest(TSetHeaterTemperatureInFeedbackModeRequest* request)
{
    TSetHeaterTemperatureInFeedbackModeResponse* response = MasterDataMemoryManager_allocate(EMessageId_SetHeaterTemperatureInFeedbackModeResponse);
    
    response->temperature = request->temperature;
    HeaterTemperatureController_resetPidStates(EPid_ProcessController);
    HeaterTemperatureController_enableDerivativeElement(EPid_ProcessController);
    response->success = HeaterTemperatureController_setTemperature(request->temperature);
    
    MasterUartGateway_sendMessage(EMessageId_SetHeaterTemperatureInFeedbackModeResponse, response);
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

void controllerDataIndCallback(EControllerDataType type, float value)
{
    TControllerDataInd* indication = MasterDataMemoryManager_allocate(EMessageId_ControllerDataInd);
    //CopyObject_SControllerData(controllerData, &(indication->data));
    indication->type = type;
    indication->value = value;
    MasterUartGateway_sendMessage(EMessageId_ControllerDataInd, indication);
}

void segmentsProgramDoneInd(u16 realizedSegmentsCount, u16 lastSegmentDoneNumber)
{
    TSegmentsProgramDoneInd* indication = MasterDataMemoryManager_allocate(EMessageId_SegmentsProgramDoneInd);
    indication->numberOfLastDoneSegment = lastSegmentDoneNumber;
    indication->realizedSegmentsCount = realizedSegmentsCount;
    MasterUartGateway_sendMessage(EMessageId_SegmentsProgramDoneInd, indication);
}

void segmentStartedInd(u16 segmentNumber, u8 leftRegisteredSegments)
{
    TSegmentStartedInd* indication = MasterDataMemoryManager_allocate(EMessageId_SegmentStartedInd);
    indication->segmentNumber = segmentNumber;
    indication->leftRegisteredSegments = leftRegisteredSegments;
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
