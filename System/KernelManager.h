#ifndef _KERNEL_MANAGER_H_

#define _KERNEL_MANAGER_H_

#include "cmsis_os.h"
#include "stdbool.h"

#include "System/TypesKernel.h"

void KernelManager_setup(void);
void KernelManager_registerThread(EThreadId threadId, osThreadId osSpecifiedThreadId);
void KernelManager_startThread(EThreadId client, EThreadId threadId);
void KernelManager_stopThread(EThreadId client, EThreadId threadId);
osThreadId KernelManager_getOsThreadId(EThreadId threadId);
bool KernelManager_isThreadRunning(EThreadId threadId);

#endif
