#ifndef _E_LOG_SEVERITY_H_

#define _E_LOG_SEVERITY_H_

typedef enum _ELogSeverity
{
    ELogSeverity_DebugSystem                    = 0,
    ELogSeverity_DebugSystemMasterDataExtended  = 1,
    ELogSeverity_Debug                          = 2,
    ELogSeverity_Info                           = 3,
    ELogSeverity_Warning                        = 4,
    ELogSeverity_Error                          = 5
} ELogSeverity;

#endif
