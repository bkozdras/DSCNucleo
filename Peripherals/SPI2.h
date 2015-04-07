#ifndef _SPI2_H_

#define _SPI2_H_

#include "Defines/CommonDefines.h"
#include "Peripherals/TypesSpi.h"

void SPI2_setup(void);
bool SPI2_initialize(void);
void SPI2_uninitialize(void);

bool SPI2_transmit(const TByte* data, const u8 dataLength, const TTimeMs timeout);
bool SPI2_receive(TByte* dataBuffer, const u8 dataLength, const TTimeMs timeout);
bool SPI2_transmitReceive(const TByte* dataToSend, TByte* dataToReceive, const u8 dataLength, const TTimeMs timeout);

bool SPI2_isInitialized(void);

#endif
