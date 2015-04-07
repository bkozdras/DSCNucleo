#ifndef _SYSTEM_MANAGER_H_

#define _SYSTEM_MANAGER_H_

#include "SharedDefines/EUnitId.h"
#include "stdbool.h"

void SystemManager_run(void);
void SystemManager_registerUnitReadyIndCallback(void (*callback)(EUnitId, bool));
void SystemManager_deregisterUnitReadyIndCallback(void);

#endif
