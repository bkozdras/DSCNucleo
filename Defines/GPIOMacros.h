#ifndef _GPIO_MACROS_H_
#define _GPIO_MACROS_H_

#include "stm32f4xx_hal.h"

#define GET_GPIO_PORT(port) SGET_GPIO_PORT(port)
#define SGET_GPIO_PORT(port) ( GPIO##port )

#define GET_GPIO_PIN(pin) SGET_GPIO_PIN(pin)
#define SGET_GPIO_PIN(pin) ( GPIO_PIN_##pin )

#define GPIO_CLOCK_ENABLE(port) SGPIO_CLOCK_ENABLE(port)
#define SGPIO_CLOCK_ENABLE(port) ( __HAL_RCC_GPIO##port##_CLK_ENABLE() )

#define GPIO_CLOCK_DISABLE(port) SGPIO_CLOCK_DISABLE(port)
#define SGPIO_CLOCK_DISABLE(port) ( __HAL_RCC_GPIO##port##_CLK_DISABLE() )

#define GPIO_TOGGLE(port, pin)  ( GET_GPIO_PORT(port), GET_GPIO_PIN(pin)) )
#define GPIO_SET_HIGH_LEVEL(port, pin)  ( HAL_GPIO_WritePin(GET_GPIO_PORT(port), GET_GPIO_PIN(pin), GPIO_PIN_SET) )
#define GPIO_SET_LOW_LEVEL(port, pin)  ( HAL_GPIO_WritePin(GET_GPIO_PORT(port), GET_GPIO_PIN(pin), GPIO_PIN_RESET) )

#define GPIO_IS_HIGH_LEVEL(port, pin) ( GPIO_PIN_SET == HAL_GPIO_ReadPin(GET_GPIO_PORT(port), GET_GPIO_PIN(pin)) )
#define GPIO_IS_LOW_LEVEL(port, pin) ( GPIO_PIN_RESET == HAL_GPIO_ReadPin(GET_GPIO_PORT(port), GET_GPIO_PIN(pin)) )

#endif
