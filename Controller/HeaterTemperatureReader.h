#ifndef _HEATER_TEMPERATURE_READER_H_

#define _HEATER_TEMPERATURE_READER_H_

#include "System/ThreadMacros.h"

THREAD_PROTOTYPE(HeaterTemperatureReader)

void HeaterTemperatureReader_setup(void);
void HeaterTemperatureReader_initialize(void);
float HeaterTemperatureReader_getTemperature(void);
void HeaterTemperature_registerNewTemperatureValueCallback(void (*newTemperatureValueCallback)(float));
void HeaterTemperature_deregisterNewTemperatureValueCallback(void);

#endif
