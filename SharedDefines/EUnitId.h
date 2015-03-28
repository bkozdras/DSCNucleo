#ifndef _E_UNIT_ID_H_

#define _E_UNIT_ID_H_

typedef enum _EUnitId
{
    EUnitId_Unknown                     = 0,
    EUnitId_Empty                       = 1,
    EUnitId_Nucleo                      = 2,
    EUnitId_ADS1248                     = 3,
    EUnitId_LMP90100ControlSystem       = 4,
    EUnitId_LMP90100SignalsMeasurement  = 5,
    EUnitId_MCP4716                     = 6,
    EUnitId_RtdPt1000                   = 7,
    EUnitId_Rtd1Pt100                   = 8,
    EUnitId_Rtd2Pt100                   = 9,
    EUnitId_ThermocoupleReference       = 10,
    EUnitId_Thermocouple1               = 11,
    EUnitId_Thermocouple2               = 12,
    EUnitId_Thermocouple3               = 13,
    EUnitId_Thermocouple4               = 14,
    
    EUnitId_Raspberry                   = 100
} EUnitId;

#endif
