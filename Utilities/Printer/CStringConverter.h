#ifndef _C_STRING_CONVERTER_H_

#define _C_STRING_CONVERTER_H_

#include "SharedDefines/EMessageId.h"
#include "SharedDefines/EFaultId.h"
#include "SharedDefines/EFaultIndicationState.h"
#include "SharedDefines/EUnitId.h"

#include "Peripherals/TypesLed.h"
#include "Peripherals/TypesExti.h"
#include "Utilities/Logger/TypesLogger.h"
#include "System/EventManagement/EEventId.h"
#include "System/TypesKernel.h"
#include "Devices/LMP90100Types.h"
#include "Devices/ADS1248Types.h"

#include "cmsis_os.h"

#include "stm32f4xx_hal.h"
#include "Peripherals/TypesI2C.h"
#include "Peripherals/TypesSpi.h"

// SHARED DEFINES

const char* CStringConverter_EMessageId(EMessageId messageId);
const char* CStringConverter_EFaultId(EFaultId faultId);
const char* CStringConverter_EFaultIndicationState(EFaultIndicationState state);
const char* CStringConverter_EUnitId(EUnitId unitId);

// OTHERS

const char* CStringConverter_ELedState(ELedState ledState);
const char* CStringConverter_EExtiType(EExtiType extiType);

const char* CStringConverter_ELoggerLevel(ELoggerLevel loggerLevel);
const char* CStringConverter_ELoggerType(ELoggerType loggerType);

const char* CStringConverter_EEventId(EEventId eventId);
const char* CStringConverter_EThreadId(EThreadId threadId);
const char* CStringConverter_ETimerId(ETimerId timerId);

const char* CStringConverter_ELMP90100Mode(ELMP90100Mode lmp90100Mode);
const char* CStringConverter_EADS1248Mode(EADS1248Mode ads1248Mode);
const char* CStringConverter_EADS1248CallibrationType(EADS1248CallibrationType ads1248CallibrationType);
const char* CStringConverter_EADS1248GainValue(EADS1248GainValue ads1248GainValue);
const char* CStringConverter_EADS1248SamplingSpeed(EADS1248SamplingSpeed ads1248SamplingSpeed);

// CMSIS RTOS

const char* CStringConverter_osStatus(osStatus status);

// HAL

const char* CStringConverter_HAL_StatusTypeDef(HAL_StatusTypeDef halStatus);
const char* CStringConverter_TI2CBusError(TI2CBusError i2cBusError);
const char* CStringConverter_TSpiBusError(TSpiBusError i2cBusError);
    
#endif
