#ifndef _COMMON_MACROS_H_

#define _COMMON_MACROS_H_

#include "stdbool.h"
#include "stm32f4xx.h"

#define SIMPLE_SET_BIT(bit)             (1 << bit)

#define SET_BIT_IN_BYTE(byte, bit)      SET_BIT(byte, bit)
#define RESET_BIT_IN_BYTE(byte, bit)    CLEAR_BIT(byte, bit)

#define IS_BIT_SET(byte, bit)           (byte & (1 << bit))
#define IS_BIT_RESET(byte, bit)         !(byte & (1 << bit))

#endif
