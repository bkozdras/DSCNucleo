#ifndef _E_FAULT_ID_H_
#define _E_FAULT_ID_H_

typedef enum _EFaultId
{
    EFaultId_Unknown            = 0,
    EFaultId_Assert             = 1,
    EFaultId_System             = 2,
    EFaultId_NoMemory           = 3,
    EFaultId_I2C                = 4,
    EFaultId_Spi                = 5,
    EFaultId_Uart               = 6,
    EFaultId_Communication      = 7,
    EFaultId_WrongData          = 8,
    EFaultId_CRCFailure         = 9,
    EFaultId_OverCurrent        = 10,
    EFaultId_TemperatureTooHigh = 11
} EFaultId;

#endif
