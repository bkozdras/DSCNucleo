#include "Utilities/Comparator.h"

#include "Defines/CommonDefines.h"
#include "Utilities/Logger/Logger.h"

static const char* getLoggerPrefix(void);

bool Comparator_u8(const u8* firstVector, const u8* secondVector, const u8 vectorsSize)
{
    bool equalResult = true;
    
    for (u8 iter = 0; vectorsSize > iter; ++iter)
    {
        if ( firstVector[iter] != secondVector[iter] )
        {
            Logger_warning("%s: Values are not equal [Pos. %u]. Elements: 0x%02X, 0x%02X.", getLoggerPrefix(), iter, firstVector[iter], secondVector[iter]);
            equalResult = false;
        }
    }
    
    return equalResult;
}

const char* getLoggerPrefix(void)
{
    return "Comparator";
}
