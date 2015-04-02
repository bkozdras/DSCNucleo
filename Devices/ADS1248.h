#ifndef _ADS1248_H_

#define _ADS1248_H_

#include "stdbool.h"

#include "Defines/CommonDefines.h"
#include "System/ThreadMacros.h"
#include "SharedDefines/ADS1248Types.h"

THREAD_PROTOTYPE(ADS1248Controller)

void ADS1248_setup(void);
void ADS1248_initialize(void);
bool ADS1248_isInitialized(void);

bool ADS1248_changeMode(EADS1248Mode newMode);
bool ADS1248_startCallibration(EADS1248CallibrationType callibrationType, void (*callibrationDoneNotifyCallback)(EADS1248CallibrationType, bool));
bool ADS1248_startReading(void);
bool ADS1248_stopReading(void);
bool ADS1248_getThermocoupleVoltageValue(EUnitId thermocouple, double* value);
bool ADS1248_setChannelGain(EADS1248GainValue gainValue);
bool ADS1248_setChannelSamplingSpeed(EADS1248SamplingSpeed samplingSpeed);

void ADS1248_registerNewValueObserver(EThreadId threadId);
void ADS1248_deregisterNewValueObserver(void);

#endif
