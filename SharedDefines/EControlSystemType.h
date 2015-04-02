#ifndef _E_CONTROL_SYSTEM_TYPE_H_

#define _E_CONTROL_SYSTEM_TYPE_H_

typedef enum _EControlSystemType
{
    EControlSystemType_OpenLoop         = 0,
    EControlSystemType_SimpleFeedback   = 1,
    EControlSystemType_MFCFeedback      = 2
} EControlSystemType;

#endif
