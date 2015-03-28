#ifndef _LED_H_

#define _LED_H_

#include "Peripherals/TypesLed.h"

void LED_setup(void);
void LED_initialize(void);
void LED_changeState(ELedState ledState);

#endif
