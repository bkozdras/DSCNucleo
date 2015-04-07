#ifndef _E_THREAD_ID_H_

#define _E_THREAD_ID_H_

#define _THREAD_IDs_COUNT 15

typedef enum _EThreadId
{
    EThreadId_SampleThread                              = 0,
    EThreadId_SystemManager                             = 1,
    EThreadId_LMP90100ControlSystemController           = 2,
    EThreadId_ADS1248Controller                         = 3,
    EThreadId_LMP90100SignalsMeasurementController      = 4,
    EThreadId_HeaterTemperatureReader                   = 5,
    EThreadId_SampleCarrierDataManager                  = 6,
    EThreadId_ReferenceTemperatureReader                = 7,
    EThreadId_MasterDataTransmitter                     = 8,
    EThreadId_MasterDataReceiver                        = 9,
    EThreadId_MasterDataManager                         = 10,
    EThreadId_StaticSegmentProgramExecutor              = 11,
    EThreadId_ISR                                       = 98,
    EThreadId_Unknown                                   = 99
} EThreadId;

#endif
