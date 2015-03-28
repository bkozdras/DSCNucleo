#ifndef _MASTER_UART_GATEWAY_H_

#define _MASTER_UART_GATEWAY_H_

#include "stdbool.h"
#include "SharedDefines/EMessageId.h"
#include "SharedDefines/TMessage.h"

void MasterUartGateway_setup(void);
void MasterUartGateway_initialize(void);

void MasterUartGateway_sendMessage(EMessageId messageType, void* message);
void MasterUartGateway_verifyReceivedMessage(TMessage message);

#endif
