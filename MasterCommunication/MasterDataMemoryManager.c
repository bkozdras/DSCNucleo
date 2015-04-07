#include "MasterCommunication/MasterDataMemoryManager.h"

#include "SharedDefines/EMessageId.h"
#include "SharedDefines/MessagesDefines.h"
#include "SharedDefines/ELogSeverity.h"

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

#define HeapSizedDef(name, size, type) osPoolDef(name, size, type)
#define Heap(name) osPool(name)
#define HeapCreate(name) osPoolCreate(Heap(name))
#define HeapAlloc(heapId) osPoolAlloc(heapId)
#define HeapCalloc(heapId) osPoolCAlloc(heapId)
#define HeapFree(heapId, event) osPoolFree(heapId, event)
#define HeapId static osPoolId

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
                                                            
#define DEFINE_MESSAGE_HEAP(message, size)                  DefineHeapSized(message, size);                                                             \
                                                            DefineHeapId(message)

#define CREATE_EVENT_HEAP(message)                          GetHeapId(message) = HeapCreate(GetHeapName(message))

/***************************************************/
                                                            
DEFINE_MESSAGE_HEAP(LogInd, 10);
DEFINE_MESSAGE_HEAP(FaultInd, 3);
DEFINE_MESSAGE_HEAP(PollingRequest, 3);
DEFINE_MESSAGE_HEAP(PollingResponse, 3);
DEFINE_MESSAGE_HEAP(ResetUnitRequest, 1);
DEFINE_MESSAGE_HEAP(ResetUnitResponse, 1);
DEFINE_MESSAGE_HEAP(SampleCarrierDataInd, 5);
DEFINE_MESSAGE_HEAP(HeaterTemperatureInd, 5);
DEFINE_MESSAGE_HEAP(ReferenceTemperatureInd, 5);
DEFINE_MESSAGE_HEAP(ControllerDataInd, 5);
DEFINE_MESSAGE_HEAP(SetHeaterPowerRequest, 2);
DEFINE_MESSAGE_HEAP(SetHeaterPowerResponse, 2);
DEFINE_MESSAGE_HEAP(CallibreADS1248Request, 1);
DEFINE_MESSAGE_HEAP(CallibreADS1248Response, 1);
DEFINE_MESSAGE_HEAP(SetChannelGainADS1248Request, 1);
DEFINE_MESSAGE_HEAP(SetChannelGainADS1248Response, 1);
DEFINE_MESSAGE_HEAP(SetChannelSamplingSpeedADS1248Request, 1);
DEFINE_MESSAGE_HEAP(SetChannelSamplingSpeedADS1248Response, 1);
DEFINE_MESSAGE_HEAP(StartRegisteringDataRequest, 1);
DEFINE_MESSAGE_HEAP(StartRegisteringDataResponse, 1);
DEFINE_MESSAGE_HEAP(StopRegisteringDataRequest, 1);
DEFINE_MESSAGE_HEAP(StopRegisteringDataResponse, 1);
DEFINE_MESSAGE_HEAP(SetNewDeviceModeLMP90100ControlSystemRequest, 1);
DEFINE_MESSAGE_HEAP(SetNewDeviceModeLMP90100ControlSystemResponse, 1);
DEFINE_MESSAGE_HEAP(SetNewDeviceModeLMP90100SignalsMeasurementRequest, 1);
DEFINE_MESSAGE_HEAP(SetNewDeviceModeLMP90100SignalsMeasurementResponse, 1);
DEFINE_MESSAGE_HEAP(SetControlSystemTypeRequest, 1);
DEFINE_MESSAGE_HEAP(SetControlSystemTypeResponse, 1);
DEFINE_MESSAGE_HEAP(SetControllerTunesRequest, 2);
DEFINE_MESSAGE_HEAP(SetControllerTunesResponse, 2);
DEFINE_MESSAGE_HEAP(SetProcessModelParametersRequest, 1);
DEFINE_MESSAGE_HEAP(SetProcessModelParametersResponse, 1);
DEFINE_MESSAGE_HEAP(SetControllingAlgorithmExecutionPeriodRequest, 1);
DEFINE_MESSAGE_HEAP(SetControllingAlgorithmExecutionPeriodResponse, 1);
DEFINE_MESSAGE_HEAP(RegisterNewSegmentToProgramRequest, 2);
DEFINE_MESSAGE_HEAP(RegisterNewSegmentToProgramResponse, 2);
DEFINE_MESSAGE_HEAP(DeregisterSegmentFromProgramRequest, 2);
DEFINE_MESSAGE_HEAP(DeregisterSegmentFromProgramResponse, 2);
DEFINE_MESSAGE_HEAP(StartSegmentProgramRequest, 1);
DEFINE_MESSAGE_HEAP(StartSegmentProgramResponse, 1);
DEFINE_MESSAGE_HEAP(StopSegmentProgramRequest, 1);
DEFINE_MESSAGE_HEAP(StopSegmentProgramResponse, 1);
DEFINE_MESSAGE_HEAP(SegmentStartedInd, 2);
DEFINE_MESSAGE_HEAP(SegmentsProgramDoneInd, 1);
DEFINE_MESSAGE_HEAP(StartReferenceTemperatureStabilizationRequest, 1);
DEFINE_MESSAGE_HEAP(StartReferenceTemperatureStabilizationResponse, 1);
DEFINE_MESSAGE_HEAP(StopReferenceTemperatureStabilizationRequest, 1);
DEFINE_MESSAGE_HEAP(StopReferenceTemperatureStabilizationResponse, 1);
DEFINE_MESSAGE_HEAP(SetRTDPolynomialCoefficientsRequest, 2);
DEFINE_MESSAGE_HEAP(SetRTDPolynomialCoefficientsResponse, 2);
DEFINE_MESSAGE_HEAP(UnitReadyInd, 8);
DEFINE_MESSAGE_HEAP(UnexpectedMasterMessageInd, 2);

static osMutexDef(mMutex);
static osMutexId mMutexId = NULL;
                                                            
static void* allocate(EMessageId messageId);
static void free(EMessageId messageId, void* allocatedMemory);
                                                            
void MasterDataMemoryManager_setup(void)
{
    mMutexId = osMutexCreate(osMutex(mMutex));
    
    CREATE_EVENT_HEAP(LogInd);
    CREATE_EVENT_HEAP(FaultInd);
    CREATE_EVENT_HEAP(PollingRequest);
    CREATE_EVENT_HEAP(PollingResponse);
    CREATE_EVENT_HEAP(ResetUnitRequest);
    CREATE_EVENT_HEAP(ResetUnitResponse);
    CREATE_EVENT_HEAP(SampleCarrierDataInd);
    CREATE_EVENT_HEAP(HeaterTemperatureInd);
    CREATE_EVENT_HEAP(ReferenceTemperatureInd);
    CREATE_EVENT_HEAP(ControllerDataInd);
    CREATE_EVENT_HEAP(SetHeaterPowerResponse);
    CREATE_EVENT_HEAP(CallibreADS1248Request);
    CREATE_EVENT_HEAP(CallibreADS1248Response);
    CREATE_EVENT_HEAP(SetChannelGainADS1248Request);
    CREATE_EVENT_HEAP(SetChannelGainADS1248Response);
    CREATE_EVENT_HEAP(SetChannelSamplingSpeedADS1248Request);
    CREATE_EVENT_HEAP(SetChannelSamplingSpeedADS1248Response);
    CREATE_EVENT_HEAP(StartRegisteringDataRequest);
    CREATE_EVENT_HEAP(StartRegisteringDataResponse);
    CREATE_EVENT_HEAP(StopRegisteringDataRequest);
    CREATE_EVENT_HEAP(StopRegisteringDataResponse);
    CREATE_EVENT_HEAP(SetNewDeviceModeLMP90100ControlSystemRequest);
    CREATE_EVENT_HEAP(SetNewDeviceModeLMP90100ControlSystemResponse);
    CREATE_EVENT_HEAP(SetNewDeviceModeLMP90100SignalsMeasurementRequest);
    CREATE_EVENT_HEAP(SetNewDeviceModeLMP90100SignalsMeasurementResponse);
    CREATE_EVENT_HEAP(SetControlSystemTypeRequest);
    CREATE_EVENT_HEAP(SetControlSystemTypeResponse);
    CREATE_EVENT_HEAP(SetControllerTunesRequest);
    CREATE_EVENT_HEAP(SetControllerTunesResponse);
    CREATE_EVENT_HEAP(SetProcessModelParametersRequest);
    CREATE_EVENT_HEAP(SetProcessModelParametersResponse);
    CREATE_EVENT_HEAP(SetControllingAlgorithmExecutionPeriodRequest);
    CREATE_EVENT_HEAP(SetControllingAlgorithmExecutionPeriodResponse);
    CREATE_EVENT_HEAP(RegisterNewSegmentToProgramRequest);
    CREATE_EVENT_HEAP(RegisterNewSegmentToProgramResponse);
    CREATE_EVENT_HEAP(DeregisterSegmentFromProgramRequest);
    CREATE_EVENT_HEAP(DeregisterSegmentFromProgramResponse);
    CREATE_EVENT_HEAP(StartSegmentProgramRequest);
    CREATE_EVENT_HEAP(StartSegmentProgramResponse);
    CREATE_EVENT_HEAP(StopSegmentProgramRequest);
    CREATE_EVENT_HEAP(StopSegmentProgramResponse);
    CREATE_EVENT_HEAP(SegmentStartedInd);
    CREATE_EVENT_HEAP(SegmentsProgramDoneInd);
    CREATE_EVENT_HEAP(StartReferenceTemperatureStabilizationRequest);
    CREATE_EVENT_HEAP(StartReferenceTemperatureStabilizationResponse);
    CREATE_EVENT_HEAP(StopReferenceTemperatureStabilizationRequest);
    CREATE_EVENT_HEAP(StopReferenceTemperatureStabilizationResponse);
    CREATE_EVENT_HEAP(SetRTDPolynomialCoefficientsRequest);
    CREATE_EVENT_HEAP(SetRTDPolynomialCoefficientsResponse);
    CREATE_EVENT_HEAP(UnitReadyInd);
    CREATE_EVENT_HEAP(UnexpectedMasterMessageInd);
}

void* MasterDataMemoryManager_allocate(EMessageId messageId)
{
    osMutexWait(mMutexId, osWaitForever);
    void* allocatedMemory = allocate(messageId);
    osMutexRelease(mMutexId);
    return allocatedMemory;
}

void MasterDataMemoryManager_free(EMessageId messageId, void* allocatedMemory)
{
    osMutexWait(mMutexId, osWaitForever);
    free(messageId, allocatedMemory);
    osMutexRelease(mMutexId);
}

u8 MasterDataMemoryManager_getLength(EMessageId messageId)
{
    GET_MESSAGE_LENGTH(LogInd);
    GET_MESSAGE_LENGTH(FaultInd);
    GET_MESSAGE_LENGTH(PollingRequest);
    GET_MESSAGE_LENGTH(PollingResponse);
    GET_MESSAGE_LENGTH(ResetUnitRequest);
    GET_MESSAGE_LENGTH(ResetUnitResponse);
    GET_MESSAGE_LENGTH(SampleCarrierDataInd);
    GET_MESSAGE_LENGTH(HeaterTemperatureInd);
    GET_MESSAGE_LENGTH(ReferenceTemperatureInd);
    GET_MESSAGE_LENGTH(ControllerDataInd);
    GET_MESSAGE_LENGTH(SetHeaterPowerRequest);
    GET_MESSAGE_LENGTH(SetHeaterPowerResponse);
    GET_MESSAGE_LENGTH(CallibreADS1248Request);
    GET_MESSAGE_LENGTH(CallibreADS1248Response);
    GET_MESSAGE_LENGTH(SetChannelGainADS1248Request);
    GET_MESSAGE_LENGTH(SetChannelGainADS1248Response);
    GET_MESSAGE_LENGTH(SetChannelSamplingSpeedADS1248Request);
    GET_MESSAGE_LENGTH(SetChannelSamplingSpeedADS1248Response);
    GET_MESSAGE_LENGTH(StartRegisteringDataRequest);
    GET_MESSAGE_LENGTH(StartRegisteringDataResponse);
    GET_MESSAGE_LENGTH(StopRegisteringDataRequest);
    GET_MESSAGE_LENGTH(StopRegisteringDataResponse);
    GET_MESSAGE_LENGTH(SetNewDeviceModeLMP90100ControlSystemRequest);
    GET_MESSAGE_LENGTH(SetNewDeviceModeLMP90100ControlSystemResponse);
    GET_MESSAGE_LENGTH(SetNewDeviceModeLMP90100SignalsMeasurementRequest);
    GET_MESSAGE_LENGTH(SetNewDeviceModeLMP90100SignalsMeasurementResponse);
    GET_MESSAGE_LENGTH(SetControlSystemTypeRequest);
    GET_MESSAGE_LENGTH(SetControlSystemTypeResponse);
    GET_MESSAGE_LENGTH(SetControllerTunesRequest);
    GET_MESSAGE_LENGTH(SetControllerTunesResponse);
    GET_MESSAGE_LENGTH(SetProcessModelParametersRequest);
    GET_MESSAGE_LENGTH(SetProcessModelParametersResponse);
    GET_MESSAGE_LENGTH(SetControllingAlgorithmExecutionPeriodRequest);
    GET_MESSAGE_LENGTH(SetControllingAlgorithmExecutionPeriodResponse);
    GET_MESSAGE_LENGTH(RegisterNewSegmentToProgramRequest);
    GET_MESSAGE_LENGTH(RegisterNewSegmentToProgramResponse);
    GET_MESSAGE_LENGTH(DeregisterSegmentFromProgramRequest);
    GET_MESSAGE_LENGTH(DeregisterSegmentFromProgramResponse);
    GET_MESSAGE_LENGTH(StartSegmentProgramRequest);
    GET_MESSAGE_LENGTH(StartSegmentProgramResponse);
    GET_MESSAGE_LENGTH(StopSegmentProgramRequest);
    GET_MESSAGE_LENGTH(StopSegmentProgramResponse);
    GET_MESSAGE_LENGTH(SegmentStartedInd);
    GET_MESSAGE_LENGTH(SegmentsProgramDoneInd);
    GET_MESSAGE_LENGTH(StartReferenceTemperatureStabilizationRequest);
    GET_MESSAGE_LENGTH(StartReferenceTemperatureStabilizationResponse);
    GET_MESSAGE_LENGTH(StopReferenceTemperatureStabilizationRequest);
    GET_MESSAGE_LENGTH(StopReferenceTemperatureStabilizationResponse);
    GET_MESSAGE_LENGTH(SetRTDPolynomialCoefficientsRequest);
    GET_MESSAGE_LENGTH(SetRTDPolynomialCoefficientsResponse);
    GET_MESSAGE_LENGTH(UnitReadyInd);
    GET_MESSAGE_LENGTH(UnexpectedMasterMessageInd);
    
    assert_param(0);
    
    return 0;
}

void* allocate(EMessageId messageId)
{
    void* allocatedMessage = NULL;
    
    ALLOCATE_MESSAGE_CALLOC_HANDLER(LogInd);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(FaultInd);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(PollingRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(PollingResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(ResetUnitRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(ResetUnitResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SampleCarrierDataInd);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(HeaterTemperatureInd);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(ReferenceTemperatureInd);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(ControllerDataInd);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetHeaterPowerRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetHeaterPowerResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(CallibreADS1248Request);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(CallibreADS1248Response);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetChannelGainADS1248Request);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetChannelGainADS1248Response);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetChannelSamplingSpeedADS1248Request);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetChannelSamplingSpeedADS1248Response);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(StartRegisteringDataRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(StartRegisteringDataResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(StopRegisteringDataRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(StopRegisteringDataResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetNewDeviceModeLMP90100ControlSystemRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetNewDeviceModeLMP90100ControlSystemResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetNewDeviceModeLMP90100SignalsMeasurementRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetNewDeviceModeLMP90100SignalsMeasurementResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetControlSystemTypeRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetControlSystemTypeResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetControllerTunesRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetControllerTunesResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetProcessModelParametersRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetProcessModelParametersResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetControllingAlgorithmExecutionPeriodRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetControllingAlgorithmExecutionPeriodResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(RegisterNewSegmentToProgramRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(RegisterNewSegmentToProgramResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(DeregisterSegmentFromProgramRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(DeregisterSegmentFromProgramResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(StartSegmentProgramRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(StartSegmentProgramResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(StopSegmentProgramRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(StopSegmentProgramResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SegmentStartedInd);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SegmentsProgramDoneInd);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(StartReferenceTemperatureStabilizationRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(StartReferenceTemperatureStabilizationResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(StopReferenceTemperatureStabilizationRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(StopReferenceTemperatureStabilizationResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetRTDPolynomialCoefficientsRequest);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(SetRTDPolynomialCoefficientsResponse);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(UnitReadyInd);
    ALLOCATE_MESSAGE_CALLOC_HANDLER(UnexpectedMasterMessageInd);
    
    assert_param(0);
    
    return NULL;
}

void free(EMessageId messageId, void* allocatedMemory)
{
    FREE_ALLOCATED_MESSAGE_HANDLER(LogInd);
    FREE_ALLOCATED_MESSAGE_HANDLER(FaultInd);
    FREE_ALLOCATED_MESSAGE_HANDLER(PollingRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(PollingResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(ResetUnitRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(ResetUnitResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(SampleCarrierDataInd);
    FREE_ALLOCATED_MESSAGE_HANDLER(HeaterTemperatureInd);
    FREE_ALLOCATED_MESSAGE_HANDLER(ReferenceTemperatureInd);
    FREE_ALLOCATED_MESSAGE_HANDLER(ControllerDataInd);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetHeaterPowerRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetHeaterPowerResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(CallibreADS1248Request);
    FREE_ALLOCATED_MESSAGE_HANDLER(CallibreADS1248Response);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetChannelGainADS1248Request);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetChannelGainADS1248Response);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetChannelSamplingSpeedADS1248Request);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetChannelSamplingSpeedADS1248Response);
    FREE_ALLOCATED_MESSAGE_HANDLER(StartRegisteringDataRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(StartRegisteringDataResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(StopRegisteringDataRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(StopRegisteringDataResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetNewDeviceModeLMP90100ControlSystemRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetNewDeviceModeLMP90100ControlSystemResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetNewDeviceModeLMP90100SignalsMeasurementRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetNewDeviceModeLMP90100SignalsMeasurementResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetControlSystemTypeRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetControlSystemTypeResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetControllerTunesRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetControllerTunesResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetProcessModelParametersRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetProcessModelParametersResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetControllingAlgorithmExecutionPeriodRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetControllingAlgorithmExecutionPeriodResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(RegisterNewSegmentToProgramRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(RegisterNewSegmentToProgramResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(DeregisterSegmentFromProgramRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(DeregisterSegmentFromProgramResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(StartSegmentProgramRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(StartSegmentProgramResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(StopSegmentProgramRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(StopSegmentProgramResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(SegmentStartedInd);
    FREE_ALLOCATED_MESSAGE_HANDLER(SegmentsProgramDoneInd);
    FREE_ALLOCATED_MESSAGE_HANDLER(StartReferenceTemperatureStabilizationRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(StartReferenceTemperatureStabilizationResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(StopReferenceTemperatureStabilizationRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(StopReferenceTemperatureStabilizationResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetRTDPolynomialCoefficientsRequest);
    FREE_ALLOCATED_MESSAGE_HANDLER(SetRTDPolynomialCoefficientsResponse);
    FREE_ALLOCATED_MESSAGE_HANDLER(UnitReadyInd);
    FREE_ALLOCATED_MESSAGE_HANDLER(UnexpectedMasterMessageInd);
    
    assert_param(0);
}

#undef HeapSizedDef
#undef Heap
#undef HeapCreate
#undef HeapAlloc
#undef HeapCalloc
#undef HeapFree
#undef HeapId
#undef GetHeapName
#undef GetHeapId
#undef DefineHeapSized
#undef DefineHeapId
#undef MessageId
#undef IsMessage
#undef ALLOCATE_MESSAGE_CALLOC_HANDLER
#undef FREE_ALLOCATED_MESSAGE_HANDLER
#undef GET_MESSAGE_LENGTH
#undef DEFINE_MESSAGE_HEAP
