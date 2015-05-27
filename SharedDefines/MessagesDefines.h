#ifndef _MESSAGES_DEFINES_H_

#define _MESSAGES_DEFINES_H_

#include "stdbool.h"
#include "Defines/CommonDefines.h"
#include "SharedDefines/SFaultIndication.h"
#include "SharedDefines/ELogSeverity.h"
#include "SharedDefines/SSampleCarrierData.h"
#include "SharedDefines/ADS1248Types.h"
#include "SharedDefines/SRTDPolynomialCoefficients.h"
#include "SharedDefines/LMP90100Types.h"
#include "SharedDefines/EControlSystemType.h"
#include "SharedDefines/EPid.h"
#include "SharedDefines/SPidTunes.h"
#include "SharedDefines/SProcessModelParameters.h"
#include "SharedDefines/SSegmentData.h"
#include "SharedDefines/ERegisteringDataType.h"
#include "SharedDefines/SControllerData.h"
#include "SharedDefines/EControllerDataType.h"

#define MAX_LOG_SIZE 220

typedef struct _TLogInd
{
    ELogSeverity severity;
    u16 length;
    char data [MAX_LOG_SIZE];
} TLogInd;

typedef struct _TFaultInd
{
    SFaultIndication indication;
} TFaultInd;
 
typedef struct _TPollingRequest
{
    bool dummy;
} TPollingRequest;

typedef struct _TPollingResponse
{
    bool success;
} TPollingResponse;

typedef struct _TResetUnitRequest
{
    EUnitId unitId;
} TResetUnitRequest;

typedef struct _TResetUnitResponse
{
    EUnitId unitId;
    bool success;
} TResetUnitResponse;

typedef struct _TSampleCarrierDataInd
{
    SSampleCarrierData data;
} TSampleCarrierDataInd;

typedef struct _THeaterTemperatureInd
{
    float temperature;
} THeaterTemperatureInd;

typedef struct _TReferenceTemperatureInd
{
    float temperature;
} TReferenceTemperatureInd;

typedef struct _TControllerDataInd
{
    //SControllerData data;
    EControllerDataType type;
    float value;
} TControllerDataInd;

typedef struct _TSetHeaterPowerRequest
{
    u16 power;
} TSetHeaterPowerRequest;

typedef struct _TSetHeaterPowerResponse
{
    u16 power;
    bool success;
} TSetHeaterPowerResponse;

typedef struct _TCallibreADS1248Request
{
    EADS1248CallibrationType callibrationType;
} TCallibreADS1248Request;

typedef struct _TCallibreADS1248Response
{
    EADS1248CallibrationType callibrationType;
    bool success;
} TCallibreADS1248Response;

typedef struct _TSetChannelGainADS1248Request
{
    EADS1248GainValue value;
} TSetChannelGainADS1248Request;

typedef struct _TSetChannelGainADS1248Response
{
    EADS1248GainValue value;
    bool success;
} TSetChannelGainADS1248Response;

typedef struct _TSetChannelSamplingSpeedADS1248Request
{
    EADS1248SamplingSpeed value;
} TSetChannelSamplingSpeedADS1248Request;

typedef struct _TSetChannelSamplingSpeedADS1248Response
{
    EADS1248SamplingSpeed value;
    bool success;
} TSetChannelSamplingSpeedADS1248Response;

typedef struct _TStartRegisteringDataRequest
{
    ERegisteringDataType dataType;
    u16 period;
} TStartRegisteringDataRequest;

typedef struct _TStartRegisteringDataResponse
{
    ERegisteringDataType dataType;
    bool success;
} TStartRegisteringDataResponse;

typedef struct _TStopRegisteringDataRequest
{
    ERegisteringDataType dataType;
} TStopRegisteringDataRequest;

typedef struct _TStopRegisteringDataResponse
{
    ERegisteringDataType dataType;
    bool success;
} TStopRegisteringDataResponse;

typedef struct _TSetNewDeviceModeADS1248Request
{
    EADS1248Mode mode;
} TSetNewDeviceModeADS1248Request;

typedef struct _TSetNewDeviceModeADS1248Response
{
    EADS1248Mode mode;
    bool success;
} TSetNewDeviceModeADS1248Response;

typedef struct _TSetNewDeviceModeLMP90100ControlSystemRequest
{
    ELMP90100Mode mode;
} TSetNewDeviceModeLMP90100ControlSystemRequest;

typedef struct _TSetNewDeviceModeLMP90100ControlSystemResponse
{
    ELMP90100Mode mode;
    bool success;
} TSetNewDeviceModeLMP90100ControlSystemResponse;

typedef struct _TSetNewDeviceModeLMP90100SignalsMeasurementRequest
{
    ELMP90100Mode mode;
} TSetNewDeviceModeLMP90100SignalsMeasurementRequest;

typedef struct _TSetNewDeviceModeLMP90100SignalsMeasurementResponse
{
    ELMP90100Mode mode;
    bool success;
} TSetNewDeviceModeLMP90100SignalsMeasurementResponse;

typedef struct _TSetControlSystemTypeRequest
{
    EControlSystemType type;
} TSetControlSystemTypeRequest;

typedef struct _TSetControlSystemTypeResponse
{
    EControlSystemType type;
    bool success;
} TSetControlSystemTypeResponse;

typedef struct _TSetControllerTunesRequest
{
    EPid pid;
    SPidTunes tunes;
} TSetControllerTunesRequest;

typedef struct _TSetControllerTunesResponse
{
    EPid pid;
    bool success;
} TSetControllerTunesResponse;

typedef struct _TSetProcessModelParametersRequest
{
    SProcessModelParameters parameters;
} TSetProcessModelParametersRequest;

typedef struct _TSetProcessModelParametersResponse
{
    SProcessModelParameters parameters;
    bool success;
} TSetProcessModelParametersResponse;

typedef struct _TSetControllingAlgorithmExecutionPeriodRequest
{
    u16 value;
} TSetControllingAlgorithmExecutionPeriodRequest;

typedef struct _TSetControllingAlgorithmExecutionPeriodResponse
{
    u16 value;
    bool success;
} TSetControllingAlgorithmExecutionPeriodResponse;

typedef struct _TRegisterNewSegmentToProgramRequest
{
    SSegmentData segment;
} TRegisterNewSegmentToProgramRequest;

typedef struct _TRegisterNewSegmentToProgramResponse
{
    u16 segmentNumber;
    u16 numberOfRegisteredSegments;
    bool success;
} TRegisterNewSegmentToProgramResponse;

typedef struct _TDeregisterSegmentFromProgramRequest
{
    u16 segmentNumber;
} TDeregisterSegmentFromProgramRequest;

typedef struct _TDeregisterSegmentFromProgramResponse
{
    u16 segmentNumber;
    u16 numberOfRegisteredSegments;
    bool success;
} TDeregisterSegmentFromProgramResponse;

typedef struct _TStartSegmentProgramRequest
{
    bool dummy;
} TStartSegmentProgramRequest;

typedef struct _TStartSegmentProgramResponse
{
    bool success;
} TStartSegmentProgramResponse;

typedef struct _TStopSegmentProgramRequest
{
    bool dummy;
} TStopSegmentProgramRequest;

typedef struct _TStopSegmentProgramResponse
{
    bool success;
} TStopSegmentProgramResponse;

typedef struct _TSegmentStartedInd
{
    u16 segmentNumber;
    u16 leftRegisteredSegments;
} TSegmentStartedInd;

typedef struct _TSegmentsProgramDoneInd
{
    u16 realizedSegmentsCount;
    u16 numberOfLastDoneSegment;
} TSegmentsProgramDoneInd;

typedef struct _TStartReferenceTemperatureStabilizationRequest
{
    bool dummy;
} TStartReferenceTemperatureStabilizationRequest;

typedef struct _TStartReferenceTemperatureStabilizationResponse
{
    bool success;
} TStartReferenceTemperatureStabilizationResponse;

typedef struct _TStopReferenceTemperatureStabilizationRequest
{
    bool dummy;
} TStopReferenceTemperatureStabilizationRequest;

typedef struct _TStopReferenceTemperatureStabilizationResponse
{
    bool success;
} TStopReferenceTemperatureStabilizationResponse;

typedef struct _TSetRTDPolynomialCoefficientsRequest
{
    SRTDPolynomialCoefficients coefficients;
} TSetRTDPolynomialCoefficientsRequest;

typedef struct _TSetRTDPolynomialCoefficientsResponse
{
    bool success;
} TSetRTDPolynomialCoefficientsResponse;

typedef struct _TUnitReadyInd
{
    EUnitId unitId;
    bool success;
} TUnitReadyInd;

typedef struct _TUnexpectedMasterMessageInd
{
    u8 id;
} TUnexpectedMasterMessageInd;

typedef struct _TSetHeaterTemperatureInFeedbackModeRequest
{
    float temperature;
} TSetHeaterTemperatureInFeedbackModeRequest;

typedef struct _TSetHeaterTemperatureInFeedbackModeResponse
{
    float temperature;
    bool success;
} TSetHeaterTemperatureInFeedbackModeResponse;

#endif
