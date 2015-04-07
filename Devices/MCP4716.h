#ifndef _MCP4716_H_

#define _MCP4716_H_

#include "Defines/CommonDefines.h"

void MCP4716_setup(void);
bool MCP4716_initialize(void);

bool MCP4716_setOutputVoltage(u16 value);
u16 MCP4716_getOutputVoltage(void);
bool MCP4716_readOutputVoltage(u16* value);

float MCP4716_convertOutputVoltageToRealData(u16 outputVoltage);

#endif
