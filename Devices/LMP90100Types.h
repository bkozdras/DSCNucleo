#ifndef _LMP90100_TYPES_H_

#define _LMP90100_TYPES_H_

typedef enum _ELMP90100Mode
{
    ELMP90100Mode_Off               = 0,
    ELMP90100Mode_On_1_6775_SPS     = 1,
    ELMP90100Mode_On_3_355_SPS      = 2,
    ELMP90100Mode_On_6_71_SPS       = 3,
    ELMP90100Mode_On_13_42_SPS      = 4,
    ELMP90100Mode_On_26_83125_SPS   = 5,
    ELMP90100Mode_On_53_6625_SPS    = 6,
    ELMP90100Mode_On_107_325_SPS    = 7,
    ELMP90100Mode_On_214_65_SPS     = 8
} ELMP90100Mode;

typedef enum _ELMP90100Rtd
{
    ELMP90100Rtd_1  = 0,
    ELMP90100Rtd_2  = 1
} ELMP90100Rtd;

#endif
