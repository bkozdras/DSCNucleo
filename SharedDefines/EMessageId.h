#ifndef _E_MESSAGE_ID_H_

#define _E_MESSAGE_ID_H_

typedef enum _EMessageId
{
    EMessageId_Unknown                                                      = 0,
    EMessageId_LogInd                                                       = 1,
    EMessageId_FaultInd                                                     = 2,
    EMessageId_PollingRequest                                               = 3,
    EMessageId_PollingResponse                                              = 4,
    EMessageId_ResetUnitRequest                                             = 5,
    EMessageId_ResetUnitResponse                                            = 6,
    EMessageId_SampleCarrierDataInd                                         = 7,
    EMessageId_HeaterTemperatureInd                                         = 8,
    EMessageId_ReferenceThermocoupleTemperatureInd                          = 9,
    EMessageId_SetHeaterPowerRequest                                        = 10,
    EMessageId_SetHeaterPowerResponse                                       = 11,
    EMessageId_CallibreADS1248Request                                       = 12,
    EMessageId_CallibreADS1248Response                                      = 13,
    EMessageId_SetChannelGainADS1248Request                                 = 14,
    EMessageId_SetChannelGainADS1248Response                                = 15,
    EMessageId_SetChannelSamplingSpeedADS1248Request                        = 16,
    EMessageId_SetChannelSamplingSpeedADS1248Response                       = 17,
    EMessageId_StartRegisteringDataRequest                                  = 18,
    EMessageId_StartRegisteringDataResponse                                 = 19,
    EMessageId_StopRegisteringDataRequest                                   = 20,
    EMessageId_StopRegisteringDataResponse                                  = 21,
    EMessageId_SetNewDeviceModeLMP90100ControlSystemRequest                 = 22,
    EMessageId_SetNewDeviceModeLMP90100ControlSystemResponse                = 23,
    EMessageId_SetNewDeviceModeLMP90100SignalsMeasurementRequest            = 24,
    EMessageId_SetNewDeviceModeLMP90100SignalsMeasurementResponse           = 25,
    EMessageId_SetControlSystemTypeRequest                                  = 26,
    EMessageId_SetControlSystemTypeResponse                                 = 27,
    EMessageId_SetControllerTunesRequest                                    = 28,
    EMessageId_SetControllerTunesResponse                                   = 29,
    EMessageId_SetProcessModelParametersRequest                             = 30,
    EMessageId_SetProcessModelParametersResponse                            = 31,
    EMessageId_RegisterNewSegmentToProgramRequest                           = 32,
    EMessageId_RegisterNewSegmentToProgramResponse                          = 33,
    EMessageId_SegmentStartedInd                                            = 34,
    EMessageId_StartReferenceTemperatureStabilizationRequest                = 35,
    EMessageId_StartReferenceTemperatureStabilizationResponse               = 36,
    EMessageId_StopReferenceTemperatureStabilizationRequest                 = 37,
    EMessageId_StopReferenceTemperatureStabilizationResponse                = 38,
    EMessageId_SetRTDPolynomialCoefficientsRequest                          = 39,
    EMessageId_SetRTDPolynomialCoefficientsResponse                         = 40,
    EMessageId_UnitReadyInd                                                 = 41,
    EMessageId_UnexpectedMasterMessageInd                                   = 99
} EMessageId;

#endif
