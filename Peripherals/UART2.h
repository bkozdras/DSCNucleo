#ifndef _UART2_H_
#define _UART2_H_

#include "Defines/CommonDefines.h"
#include "Peripherals/TypesUart.h"

void UART2_initializeDefault(void);
void UART2_initialize(TUartBaudRate baudRate, TUartMode mode);
void UART2_uninitialize(void);

void UART2_send(const TByte* data, const u8 dataLength);
void UART2_sendByte(TByte byte);

bool UART2_isInitialized(void);

#endif
