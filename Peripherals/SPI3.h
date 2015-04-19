#ifndef _SPI3_H_

#define _SPI3_H_

#include "Defines/CommonDefines.h"
#include "Peripherals/TypesSpi.h"

typedef enum _ESpi3ClkPolarity
{
    ESpi3ClkPolarity_Low    = 0,
    ESpi3ClkPolarity_High   = 1
} ESpi3ClkPolarity;

void SPI3_setup(void);
bool SPI3_initialize(void);
void SPI3_uninitialize(void);

void SPI3_block(ESpi3ClkPolarity clkPolarity);
void SPI3_unblock(void);

bool SPI3_transmit(const TByte* data, const u8 dataLength, const TTimeMs timeout);
bool SPI3_receive(TByte* dataBuffer, const u8 dataLength, const TTimeMs timeout);
bool SPI3_transmitReceive(const TByte* dataToSend, TByte* dataToReceive, const u8 dataLength, const TTimeMs timeout);

bool SPI3_isInitialized(void);

#endif
