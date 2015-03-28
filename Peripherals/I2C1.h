#ifndef _I2C1_H_

#define _I2C1_H_

#include "Defines/CommonDefines.h"
#include "Peripherals/TypesI2C.h"

void I2C1_setup(void);
void I2C1_initialize(void);
void I2C1_uninitialize(void);

bool I2C1_isDeviceReady(const TI2CDeviceAddress address, const TTimeMs timeout);
bool I2C1_transmit(const TI2CDeviceAddress address, const TByte* data, const u8 dataLength, const TTimeMs timeout);
bool I2C1_receive(const TI2CDeviceAddress address, TByte* dataBuffer, const u8 dataLength, const TTimeMs timeout);

bool I2C1_isInitialized(void);

#endif
