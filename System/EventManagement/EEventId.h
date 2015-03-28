#ifndef _E_EVENT_ID_H_

#define _E_EVENT_ID_H_

typedef enum _EEventId
{
    EEventId_Unknown                            = 0,
    EEventId_Start                              = 1,
    EEventId_StartAck                           = 2,
    EEventId_Stop                               = 3,
    EEventId_StopAck                            = 4,
    EEventId_DeviceDataReadyInd                 = 5,
    EEventId_NewRTDValueInd                     = 6,
    EEventId_NewThermocoupleVoltageValueInd     = 7,
    EEventId_CallibrationDoneInd                = 8,
    EEventId_TransmitDataToMaster               = 9,
    EEventId_DataToMasterTransmittedInd         = 10,
    EEventId_DataFromMasterReceivedInd          = 11,
    EEventId_TransmitData                       = 12,
    EEventId_ReceiveData                        = 13,
    EEventId_StartReceivingData                 = 14,
    EEventId_Terminate                          = 99
} EEventId;

#endif
