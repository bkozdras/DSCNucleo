#ifndef _EXTI_H_

#define _EXTI_H_

#include "Defines/CommonDefines.h"
#include "Peripherals/TypesExti.h"
#include "stdbool.h"

void EXTI_setup(void);
void EXTI_initialize(EExtiType extiType);
bool EXTI_isInitialized(EExtiType extiType);
void EXTI_setCallback(TPin pin, void (*callback)(void));
void EXTI_unsetCallback(TPin pin);

#endif
