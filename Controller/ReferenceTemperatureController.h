#ifndef _REFERENCE_TEMPERATURE_CONTROLLER_H_

#define _REFERENCE_TEMPERATURE_CONTROLLER_H_

#include "stdbool.h"

void ReferenceTemperatureController_setup(void);
void ReferenceTemperatureController_initialize(void);
bool ReferenceTemperatureController_startStabilization(void);
void ReferenceTemperatureController_stopStabilization(void);
void ReferenceTemperatureController_registerDRV595UnitFaultyCallback(void (*callback)(void));
void ReferenceTemperatureController_deregisterDRV595UnitFaultyCallback(void);

#endif
