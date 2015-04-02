#ifndef _S_SEGMENT_DATA_H_

#define _S_SEGMENT_DATA_H_

typedef struct _ESegmentType
{
    ESegmentType_Static     = 0,
    ESegmentType_Dynamic    = 1
} ESegmentType;

typedef struct _SSegmentData
{
    u16 number;
    ESegmentType type;
    float startTemperature;
    float stopTemperature;
    u16 settingTimeInterval;
} SSegmentData;

#endif
