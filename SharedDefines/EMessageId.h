#ifndef _E_MESSAGE_ID_H_

#define _E_MESSAGE_ID_H_

typedef enum _EMessageId
{
    EMessageId_Unknown                                              = 0,
    EMessageId_LogInd                                               = 1,
    EMessageId_FaultInd                                             = 2,
    EMessageId_PollingRequest                                       = 3,
    EMessageId_PollingResponse                                      = 4,
    EMessageId_ResetUnitRequest                                     = 5,
    EMessageId_ResetUnitResponse                                    = 6,
    EMessageId_SampleCarrierDataInd                                 = 7,
    EMessageId_HeaterTemperatureInd                                 = 8,
    EMessageId_ReferenceThermocoupleTemperatureInd                  = 9,
    EMessageId_SetHeaterPowerRequest                                = 10,
    EMessageId_SetHeaterPowerResponse                               = 11,
    EMessageId_CallibreADS1248Request                               = 12,
    EMessageId_CallibreADS1248Response                              = 13,
    EMessageId_SetChannelGainADS1248Request                         = 14,
    EMessageId_SetChannelGainADS1248Response                        = 15,
    EMessageId_SetChannelSamplingSpeedADS1248Request                = 16,
    EMessageId_SetChannelSamplingSpeedADS1248Response               = 17,
    EMessageId_SampleCarrierDataStartMeasuringRequest               = 18,
    EMessageId_SampleCarrierDataStartMeasuringResponse              = 19,
    EMessageId_SetNewDeviceModeLMP90100ControlSystemRequest         = 20,
    EMessageId_SetNewDeviceModeLMP90100ControlSystemResponse        = 21,
    EMessageId_SetNewDeviceModeLMP90100SignalsMeasurementRequest    = 22,
    EMessageId_SetNewDeviceModeLMP90100SignalsMeasurementResponse   = 23,
    EMessageId_SetControlSystemTypeRequest                          = 24,
    EMessageId_SetControlSystemTypeResponse                         = 25,
    EMessageId_SetControllerTunesRequest                            = 26,
    EMessageId_SetControllerTunesResponse                           = 27,
    EMessageId_SetProcessModelParametersRequest                     = 28,
    EMessageId_SetProcessModelParametersResponse                    = 29,
    EMessageId_RegisterNewSegmentToProgramRequest                   = 30,
    EMessageId_ResisterNewSegmentToProgramResponse                  = 31,
    EMessageId_UnitReadyInd                                         = 32,
    EMessageId_UnexpectedMasterMessageInd                           = 99
} EMessageId;

#endif
