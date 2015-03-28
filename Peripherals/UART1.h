#ifndef _UART1_H_
#define _UART1_H_

#include "Defines/CommonDefines.h"
#include "Peripherals/TypesUart.h"

void UART1_initializeDefault(void);
void UART1_initialize(TUartBaudRate baudRate, TUartMode mode);
void UART1_uninitialize(void);

bool UART1_transmit(TByte* data, const u16 dataLength);
bool UART1_receive(TByte* data, const u16 dataLength);

void UART1_registerDataTransmittingDoneCallback(void (*transmittingDoneCallback)(void));
void UART1_deregisterDataTransmittingDoneCallback(void);
void UART1_registerDataReceivingDoneCallback(void (*receivingDoneCallback)(void));
void UART1_deregisterDataReceivingDoneCallback(void);

bool UART1_isInitialized(void);

#endif
