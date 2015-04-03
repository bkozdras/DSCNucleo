#ifndef _TIM3_H_

#define _TIM3_H_

#include "Defines/CommonDefines.h"
#include "stdbool.h"

void TIM3_setup(void);
void TIM3_setPeriod(u16 periodMs);
bool TIM3_start(void);
bool TIM3_stop(void);
void TIM3_registerPeriodElapsedCallback(void (*periodElapsedCallback)(void));
void TIM3_deregisterPeriodElapsedCallback(void);

#endif
