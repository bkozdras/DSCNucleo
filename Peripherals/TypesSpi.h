#ifndef _TYPES_SPI_H_

#define _TYPES_SPI_H_

#include "Defines/CommonDefines.h"

typedef u32 TSpiBusError;

typedef enum _TSpiOperation
{
    TSpiOperation_Read  = 0,
    TSpiOperation_Write = 1
} TSpiOperation;

#endif
