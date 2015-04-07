#ifndef _REFERENCE_TEMPERATURE_READER_H_

#define _REFERENCE_TEMPERATURE_READER_H_

#include "System/ThreadMacros.h"

THREAD_PROTOTYPE(ReferenceTemperatureReader)

void ReferenceTemperatureReader_setup(void);
void ReferenceTemperatureReader_initialize(void);
float ReferenceTemperatureReader_getTemperature(void);
void ReferenceTemperatureReader_registerDataReadyCallback(void (*dataReadyCallback)(float));
void ReferenceTemperatureReader_deregisterDataReadyCallback(void);

#endif
