#ifndef _MCP4716_REGISTERS_H_

#define _MCP4716_REGISTERS_H_

#include "Defines/CommonMacros.h"

#define MCP4716_DEVICE_ADDRESS                      0xC0

#define MCP4716_C2                                  7
#define MCP4716_C1                                  6
#define MCP4716_C0                                  5
#define MCP4716_VREF1                               4
#define MCP4716_VREF0                               3
#define MCP4716_PD1                                 2
#define MCP4716_PD0                                 1
#define MCP4716_GAIN                                0

#define MCP4716_COMMAND_MASK                        0x1F
#define MCP4716_COMMAND_VOLATILE_DAC_REGISTER       0x00
#define MCP4716_COMMAND_VOLATILE_MEMORY             SIMPLE_SET_BIT( MCP4716_C1 )
#define MCP4716_COMMAND_ALL_MEMORY                  ( SIMPLE_SET_BIT( MCP4716_C1 ) | SIMPLE_SET_BIT( MCP4716_C0 ) )
#define MCP4716_COMMAND_VOLATILE_CONFIGURATION      SIMPLE_SET_BIT( MCP4716_C2 )

#define MCP4716_RDY_BIT                             7
#define MCP4716_POR_BIT                             6

#define MCP4716_VOLTAGE_REFERENCE_MASK              0xE7
#define MCP4716_VOLTAGE_REFERENCE_VDD               SIMPLE_SET_BIT( MCP4716_VREF0 )
#define MCP4716_VOLTAGE_REFERENCE_VREF_UNBUFFERED   SIMPLE_SET_BIT( MCP4716_VREF1 )
#define MCP4716_VOLTAGE_REFERENCE_VREF_BUFFERED     ( SIMPLE_SET_BIT( MCP4716_VREF1 ) | SIMPLE_SET_BIT( MCP4716_VREF0 ) )

#define MCP4716_POWER_DOWN_MASK                     0xF9
#define MCP4716_POWER_DOWN_NORMAL_OPERATION         0x00
#define MCP4716_POWER_DOWN_1_KOHM_TO_GROUND         SIMPLE_SET_BIT( MCP4716_PD0 )
#define MCP4716_POWER_DOWN_125_KOHM_TO_GROUND       SIMPLE_SET_BIT( MCP4716_PD1 )
#define MCP4716_POWER_DOWN_640_KOHM_TO_GROUND       ( SIMPLE_SET_BIT( MCP4716_PD1 ) | SIMPLE_SET_BIT( MCP4716_PD0 ) )

#define MCP4716_GAIN_MASK                           0xFE
#define MCP4716_GAIN_1_X                            0x00
#define MCP4716_GAIN_2_X                            SIMPLE_SET_BIT( MCP4716_GAIN )

#endif
