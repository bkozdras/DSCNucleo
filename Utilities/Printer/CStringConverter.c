#include "Utilities/Printer/CStringConverter.h"

#include "cmsis_os.h"

// SHARED DEFINES

const char* CStringConverter_EMessageId(EMessageId messageId)
{
    switch (messageId)
    {
        case EMessageId_PollingRequest :
            return "PollingRequest";
        
        case EMessageId_PollingResponse :
            return "PollingResponse";
        
        case EMessageId_Unknown :
            return "Unknown";
    }
    
    return "Unknown EMessageId";
}

const char* CStringConverter_EFaultId(EFaultId faultId)
{
    switch (faultId)
    {
        case EFaultId_Assert :
            return "Assert";
        
        case EFaultId_Unknown :
            return "Unknown";
        
        case EFaultId_System :
            return "System";
        
        case EFaultId_NoMemory :
            return "NoMemory";
        
        case EFaultId_Communication :
            return "Communication";
        
        case EFaultId_I2C :
            return "I2C";
        
        case EFaultId_Spi :
            return "Spi";
        
        case EFaultId_Uart :
            return "Uart";
        
        case EFaultId_WrongData :
            return "WrongData";
        
        case EFaultId_CRCFailure :
            return "CRCFailure";
    }
    
    return "Unknown EFaultId";
}

const char* CStringConverter_EFaultIndicationState(EFaultIndicationState state)
{
    switch (state)
    {
        case EFaultIndicationState_Start :
            return "Start";
        
        case EFaultIndicationState_Cancel :
            return "Cancel";
    }
    
    return "Unknown EFaultIndicationState";
}

const char* CStringConverter_EUnitId(EUnitId unitId)
{
    switch (unitId)
    {
        case EUnitId_Nucleo :
            return "Nucleo";
        
        case EUnitId_ADS1248 :
            return "ADS1248";
        
        case EUnitId_LMP90100ControlSystem :
            return "LMP90100 ControlSystem";
        
        case EUnitId_LMP90100SignalsMeasurement :
            return "LMP90100 SignalsMeasurement";
        
        case EUnitId_MCP4716 :
            return "MCP4716";
        
        case EUnitId_RtdPt1000 :
            return "RTD Pt1000";
        
        case EUnitId_Rtd1Pt100 :
            return "RTD 1 Pt100";
        
        case EUnitId_Rtd2Pt100 :
            return "RTD 2 Pt100";
        
        case EUnitId_ThermocoupleReference :
            return "Thermocouple Reference";
        
        case EUnitId_Thermocouple1 :
            return "Thermocouple 1";
        
        case EUnitId_Thermocouple2 :
            return "Thermocouple 2";
                
        case EUnitId_Thermocouple3 :
            return "Thermocouple 3";
                        
        case EUnitId_Thermocouple4 :
            return "Thermocouple 4";

        case EUnitId_Empty :
            return "Empty";
        
        case EUnitId_Unknown :
            return "Unknown";
        
        default :
            break;
    }
    
    return "Unknown EFaultId";
}

// OTHERS

const char* CStringConverter_ELedState(ELedState ledState)
{
    switch (ledState)
    {
        case ELedState_On :
            return "On";
        
        case ELedState_Off :
            return "Off";
    }
    
    return "Unknown ELedState";
}

const char* CStringConverter_EExtiType(EExtiType extiType)
{
    switch (extiType)
    {
        case EExtiType_EXTI0 :
            return "EXTI0";

        case EExtiType_EXTI1 :
            return "EXTI1";

        case EExtiType_EXTI2 :
            return "EXTI2";

        case EExtiType_EXTI3 :
            return "EXTI3";

        case EExtiType_EXTI4 :
            return "EXTI4";

        case EExtiType_EXTI9_5 :
            return "EXTI9_5";
        
        case EExtiType_EXTI15_10 :
            return "EXTI15_10";
    }
    
    return "Unknown EExtiType";
}

const char* CStringConverter_ELoggerLevel(ELoggerLevel loggerLevel)
{
    switch (loggerLevel)
    {
        case ELoggerLevel_Off :
            return "Off";
        
        case ELoggerLevel_Debug :
            return "Debug";
        
        case ELoggerLevel_Info :
            return "Info";
    }
    
    return "Unknown ELoggerLevel";
}

const char* CStringConverter_ELoggerType(ELoggerType loggerType)
{
    switch (loggerType)
    {
        case ELoggerType_EvalCOM1 :
            return "EvalCOM1";
        
        case ELoggerType_ATCommand :
            return "ATCommand";
        
        case ELoggerType_EvalCOM1AndATCommand :
            return "EvalCOM1_ATCommand";
        
        case ELoggerType_Off :
            return "Off";
    }
    
    return "Unknown_ELoggerType";
}

const char* CStringConverter_EEventId(EEventId eventId)
{
    switch (eventId)
    {
        case EEventId_Unknown :
            return "Unknown";
        
        case EEventId_Start :
            return "Start";
        
        case EEventId_StartAck :
            return "StartAck";
        
        case EEventId_Stop :
            return "Stop";
        
        case EEventId_StopAck :
            return "StopAck";
        
        case EEventId_DeviceDataReadyInd :
            return "DeviceDataReadyInd";
        
        case EEventId_NewRTDValueInd :
            return "NewRTDValueInd";
        
        case EEventId_CallibrationDoneInd :
            return "CallibrationDoneInd";
        
        case EEventId_NewThermocoupleVoltageValueInd :
            return "NewThermocoupleVoltageValueInd";
        
        case EEventId_DataFromMasterReceivedInd :
            return "DataFromMasterReceivedInd";
        
        case EEventId_DataToMasterTransmittedInd :
            return "DataToMasterTransmittedInd";
        
        case EEventId_ReceiveData :
            return "ReceiveData";
        
        case EEventId_StartReceivingData :
            return "StartReceivingData";
        
        case EEventId_TransmitData :
            return "TransmitData";
        
        case EEventId_TransmitDataToMaster :
            return "TransmitDataToMaster";
        
        case EEventId_Terminate :
            return "Terminate";
    }
    
    return "Unknown EEventId";
}

const char* CStringConverter_EThreadId(EThreadId threadId)
{
    switch (threadId)
    {
        case EThreadId_Unknown :
            return "Unknown";
        
        case EThreadId_SampleThread :
            return "SampleThread";
        
        case EThreadId_SystemManager :
            return "SystemManager";
        
        case EThreadId_ADS1248Controller :
            return "ADS1248Controller";
        
        case EThreadId_LMP90100ControlSystemController :
            return "LMP90100ControlSystemController";
        
        case EThreadId_LMP90100SignalsMeasurementController :
            return "LMP90100SignalsMeasurementController";
        
        case EThreadId_HeaterTemperatureReader :
            return "HeaterTemperatureReader";
        
        case EThreadId_SampleCarrierDataManager :
            return "SampleCarrierDataManager";
        
        case EThreadId_ReferenceThermocoupleTemperatureReader :
            return "ReferenceThermcoupleTemperatureReader";
        
        case EThreadId_MasterDataManager :
            return "MasterDataManager";
        
        case EThreadId_MasterDataReceiver :
            return "MasterDataReceiver";
        
        case EThreadId_MasterDataTransmitter :
            return "MasterDataTransmitter";
        
        case EThreadId_ISR :
            return "ISR";
    }
    
    return "Unknown EThreadId";
}

const char* CStringConverter_ETimerId(ETimerId timerId)
{
    switch (timerId)
    {
        case ETimerId_HeaterTemperatureController :
            return "HeaterTemperatureController";
        
        case ETimerId_DevicesDiagnostic :
            return "DeviceDiagnostic";
        
        case ETimerId_Unknown :
            return "Unknown";
    }
    
    return "Unknown ETimerId";
}

const char* CStringConverter_ELMP90100Mode(ELMP90100Mode lmp90100Mode)
{
    switch (lmp90100Mode)
    {
        case ELMP90100Mode_Off :
            return "Off";
        
        case ELMP90100Mode_On_1_6775_SPS :
            return "On 1.6775 SPS";
        
        case ELMP90100Mode_On_3_355_SPS :
            return "On 3.355 SPS";
        
        case ELMP90100Mode_On_6_71_SPS :
            return "On 6.71 SPS";
        
        case ELMP90100Mode_On_13_42_SPS :
            return "On 13.42 SPS";
        
        case ELMP90100Mode_On_26_83125_SPS :
            return "On 26.83125 SPS";
        
        case ELMP90100Mode_On_53_6625_SPS :
            return "On 53.6625 SPS";
        
        case ELMP90100Mode_On_107_325_SPS :
            return "On 107.325 SPS";
        
        case ELMP90100Mode_On_214_65_SPS :
            return "On 214.65 SPS";
    }
    
    return "Unknown ELMP90100Mode";
}

const char* CStringConverter_EADS1248Mode(EADS1248Mode ads1248Mode)
{
    switch (ads1248Mode)
    {
        case EADS1248Mode_Off :
            return "Off";
        
        case EADS1248Mode_On :
            return "On";
    }
    
    return "Unknown EADS1248Mode";
}

const char* CStringConverter_EADS1248CallibrationType(EADS1248CallibrationType ads1248CallibrationType)
{
    switch (ads1248CallibrationType)
    {
        case EADS1248CallibrationType_OffsetSystem :
            return "Offset System";
        
        case EADS1248CallibrationType_SelfOffset :
            return "Self Offset";
        
        case EADS1248CallibrationType_SystemGain :
            return "System Gain";
        
        case EADS1248CallibrationType_Idle :
            return "Idle";
    }
    
    return "Unknown EADS1248CallibrationType";
}

const char* CStringConverter_EADS1248GainValue(EADS1248GainValue ads1248GainValue)
{
    switch (ads1248GainValue)
    {
        case EADS1248GainValue_1 :
            return "1";
        
        case EADS1248GainValue_2 :
            return "2";
                
        case EADS1248GainValue_4 :
            return "4";
                        
        case EADS1248GainValue_8 :
            return "8";
                                
        case EADS1248GainValue_16 :
            return "16";
                                        
        case EADS1248GainValue_32 :
            return "32";
                                                
        case EADS1248GainValue_64 :
            return "64";
        
        case EADS1248GainValue_128 :
            return "128";
    }
    
    return "Unknown EADS1248GainValue";
}

const char* CStringConverter_EADS1248SamplingSpeed(EADS1248SamplingSpeed ads1248SamplingSpeed)
{
    switch (ads1248SamplingSpeed)
    {
        case EADS1248SamplingSpeed_5SPS :
            return "5 SPS";
        
        case EADS1248SamplingSpeed_10SPS :
            return "10 SPS";
                
        case EADS1248SamplingSpeed_20SPS :
            return "20 SPS";
                        
        case EADS1248SamplingSpeed_40SPS :
            return "40 SPS";
                                
        case EADS1248SamplingSpeed_80SPS :
            return "80 SPS";
                                        
        case EADS1248SamplingSpeed_160SPS :
            return "160 SPS";
                                                
        case EADS1248SamplingSpeed_320SPS :
            return "320 SPS";
                                                        
        case EADS1248SamplingSpeed_640SPS :
            return "640 SPS";
                                                                
        case EADS1248SamplingSpeed_1000SPS :
            return "1000 SPS";
        
        case EADS1248SamplingSpeed_2000SPS :
            return "2000 SPS";
    }
    
    return "Unknown EADS1248SamplingSpeed";
}

const char* CStringConverter_osStatus(osStatus status)
{
    switch (status)
    {
        case osOK :
            return "OK";
        
        case osEventSignal :
            return "EventSignal";
        
        case osEventMessage :
            return "EventMessage";
        
        case osEventMail :
            return "EventMail";
        
        case osEventTimeout :
            return "EventTimeout";
        
        case osErrorParameter :
            return "ErrorParameter";
        
        case osErrorResource :
            return "ErrorResource";
        
        case osErrorTimeoutResource :
            return "ErrorTimeoutResource";
        
        case osErrorISR :
            return "ErrorISR";
        
        case osErrorISRRecursive :
            return "ErrorISRRecursive";
        
        case osErrorPriority :
            return "ErrorPriority";
        
        case osErrorNoMemory :
            return "ErrorNoMemory";
        
        case osErrorValue :
            return "ErrorValue";
        
        case osErrorOS :
            return "ErrorOS";
        
        case os_status_reserved :
            return "Reserved";
    }
    
    return "Unknown osStatus";
}

const char* CStringConverter_HAL_StatusTypeDef(HAL_StatusTypeDef halStatus)
{
    switch(halStatus)
    {
        case HAL_OK :
            return "OK";
        
        case HAL_BUSY :
            return "BUSY";
        
        case HAL_ERROR :
            return "ERROR";
        
        case HAL_TIMEOUT :
            return "TIMEOUT";
    }
    
    return "Unknown HAL_StatusTypeDef";
}

const char* CStringConverter_TI2CBusError(TI2CBusError i2cBusError)
{
    switch(i2cBusError)
    {
        case HAL_I2C_ERROR_NONE :
            return "NoError";
        
        case HAL_I2C_ERROR_BERR :
            return "BERR";
        
        case HAL_I2C_ERROR_ARLO :
            return "ARLO";
        
        case HAL_I2C_ERROR_AF :
            return "AF";
        
        case HAL_I2C_ERROR_OVR :
            return "OVR";
        
        case HAL_I2C_ERROR_DMA :
            return "DMA";
        
        case HAL_I2C_ERROR_TIMEOUT :
            return "Timeout";
    }
    
    return "Unknown TI2CBusError";
}

const char* CStringConverter_TSpiBusError(TSpiBusError spiBusError)
{
    switch(spiBusError)
    {
        case HAL_SPI_ERROR_NONE :
            return "NoError";
        
        case HAL_SPI_ERROR_MODF :
            return "MODF";
        
        case HAL_SPI_ERROR_CRC :
            return "CRC";
        
        case HAL_SPI_ERROR_OVR :
            return "OVR";
        
        case HAL_SPI_ERROR_FRE :
            return "FRE";
        
        case HAL_SPI_ERROR_DMA :
            return "DMA";
        
        case HAL_SPI_ERROR_FLAG :
            return "FLAG";
    }
    
    return "Unknown TSpiBusError";
}
