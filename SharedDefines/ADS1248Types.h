#ifndef _ADS1248_TYPES_H_

#define _ADS1248_TYPES_H_

typedef enum _EADS1248Mode
{
    EADS1248Mode_Off    = 0,
    EADS1248Mode_On     = 1
} EADS1248Mode;

typedef enum _EADS1248CallibrationType
{
    EADS1248CallibrationType_Idle           = 0,
    EADS1248CallibrationType_OffsetSystem   = 1,
    EADS1248CallibrationType_SystemGain     = 2,
    EADS1248CallibrationType_SelfOffset     = 3
} EADS1248CallibrationType;

typedef enum _EADS1248GainValue
{
    EADS1248GainValue_1                     = 0,
    EADS1248GainValue_2                     = 1,
    EADS1248GainValue_4                     = 2,
    EADS1248GainValue_8                     = 3,
    EADS1248GainValue_16                    = 4,
    EADS1248GainValue_32                    = 5,
    EADS1248GainValue_64                    = 6,
    EADS1248GainValue_128                   = 7
} EADS1248GainValue;

typedef enum _EADS1248SamplingSpeed
{
    EADS1248SamplingSpeed_5SPS              = 0,
    EADS1248SamplingSpeed_10SPS             = 1,
    EADS1248SamplingSpeed_20SPS             = 2,
    EADS1248SamplingSpeed_40SPS             = 3,
    EADS1248SamplingSpeed_80SPS             = 4,
    EADS1248SamplingSpeed_160SPS            = 5,
    EADS1248SamplingSpeed_320SPS            = 6,
    EADS1248SamplingSpeed_640SPS            = 7,
    EADS1248SamplingSpeed_1000SPS           = 8,
    EADS1248SamplingSpeed_2000SPS           = 9
} EADS1248SamplingSpeed;

#endif
