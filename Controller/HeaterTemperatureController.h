#ifndef _HEATER_TEMPERATURE_CONTROLLER_H_

#define _HEATER_TEMPERATURE_CONTROLLER_H_

#include "Defines/CommonDefines.h"

#include "SharedDefines/EControlSystemType.h"
#include "SharedDefines/EPid.h"
#include "SharedDefines/SPidTunes.h"
#include "SharedDefines/SProcessModelParameters.h"
#include "SharedDefines/SControllerData.h"

void HeaterTemperatureController_setup(void);
void HeaterTemperatureController_initialize(void);

bool HeaterTemperatureController_setAlgorithmExecutionPeriod(u16 period);
bool HeaterTemperatureController_setProcessModelParameters(SProcessModelParameters* parameters);
bool HeaterTemperatureController_setSystemType(EControlSystemType type);
bool HeaterTemperatureController_setTunes(EPid pid, SPidTunes* tunes);

bool HeaterTemperatureController_setPower(u16 power);
bool HeaterTemperatureController_setTemperature(float temperature);

float HeaterTemperatureController_getControllerError(void);

bool HeaterTemperatureController_start(void);
bool HeaterTemperatureController_stop(void);

void HeaterTemperatureController_registerNewControllerDataCallback(void (*callback)(SControllerData*));
void HeaterTemperatureController_deregisterNewControllerDataCallback(void);

#endif
