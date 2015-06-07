#ifndef _HEATER_TEMPERATURE_READER_H_

#define _HEATER_TEMPERATURE_READER_H_

#include "System/ThreadMacros.h"
#include "Defines/CommonDefines.h"

THREAD_PROTOTYPE(HeaterTemperatureReader)

void HeaterTemperatureReader_setup(void);
void HeaterTemperatureReader_initialize(void);
float HeaterTemperatureReader_getTemperature(void);
bool HeaterTemperatureReader_registerNewTemperatureValueCallback(void (*newTemperatureValueCallback)(float), u16 period);
bool HeaterTemperatureReader_deregisterNewTemperatureValueCallback(void);

#endif
