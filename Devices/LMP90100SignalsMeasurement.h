#ifndef _LMP90100_SIGNALS_MEASUREMENT_H_

#define _LMP90100_SIGNALS_MEASUREMENT_H_

#include "stdbool.h"

#include "Defines/CommonDefines.h"
#include "Devices/LMP90100Types.h"
#include "System/ThreadMacros.h"

THREAD_PROTOTYPE(LMP90100SignalsMeasurementController)

void LMP90100SignalsMeasurement_setup(void);
void LMP90100SignalsMeasurement_initialize(void);
bool LMP90100SignalsMeasurement_isInitialized(void);

bool LMP90100SignalsMeasurement_changeMode(ELMP90100Mode newMode);

void LMP90100SignalsMeasurement_registerNewValueObserver(ELMP90100Rtd rtd, EThreadId threadId);
void LMP90100SignalsMeasurement_deregisterNewValueObserver(ELMP90100Rtd rtd);

#endif
