#ifndef _LMP90100_CONTROL_SYSTEM_H_

#define _LMP90100_CONTROL_SYSTEM_H_

#include "stdbool.h"

#include "Defines/CommonDefines.h"
#include "SharedDefines/LMP90100Types.h"
#include "System/ThreadMacros.h"

THREAD_PROTOTYPE(LMP90100ControlSystemController)

void LMP90100ControlSystem_setup(void);
void LMP90100ControlSystem_initialize(void);
bool LMP90100ControlSystem_isInitialized(void);

bool LMP90100ControlSystem_changeMode(ELMP90100Mode newMode);

void LMP90100ControlSystem_registerNewValueObserver(EThreadId threadId);
void LMP90100ControlSystem_deregisterNewValueObserver(void);

#endif
