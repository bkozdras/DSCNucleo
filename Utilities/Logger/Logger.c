#include "cmsis_os.h"

#include "Utilities/Logger/TypesLogger.h"
#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"
#include "Peripherals/UART2.h"
#include "Peripherals/UART1.h"
#include "SharedDefines/ELogSeverity.h"
#include "SharedDefines/PrinterPrefixes.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"

#include "MasterCommunication/MasterDataMemoryManager.h"
#include "SharedDefines/MessagesDefines.h"

static osMutexDef(mMutexLogger);
static osMutexId mMutexLoggerId = NULL;
static osMutexDef(mMutexLoggerEvalCom1);
static osMutexId mMutexLoggerEvalCom1Id = NULL;
static osMutexDef(mMutexLoggerMasterMessage);
static osMutexId mMutexLoggerMasterMessageId = NULL;

static ELoggerType mLoggerType = ELoggerType_Off;
static ELoggerLevel mLoggerLevel = ELoggerLevel_Off;
static void (*mMasterMesssageLogIndCallback)(TLogInd*) = NULL;

static void printPrefix(void);
static void printPostfix(void);
static void initialize(ELoggerType loggerType, ELoggerLevel loggerLevel);
static bool isLogShouldBePrinted(ELogSeverity severity);
static void uninitialize(void);
static void setType(ELoggerType loggerType);
static void setLevel(ELoggerLevel loggerLevel);

void Logger_setup(void)
{
    if (!mMutexLoggerId)
    {
        mMutexLoggerId = osMutexCreate( osMutex(mMutexLogger) );
    }
    if (!mMutexLoggerEvalCom1Id)
    {
        mMutexLoggerEvalCom1Id = osMutexCreate( osMutex(mMutexLoggerEvalCom1) );
    }
    if (!mMutexLoggerMasterMessageId)
    {
        mMutexLoggerMasterMessageId = osMutexCreate( osMutex(mMutexLoggerMasterMessage) );
    }
}

void Logger_initialize(ELoggerType loggerType, ELoggerLevel loggerLevel)
{
    osMutexWait(mMutexLoggerId, osWaitForever);
    
    initialize(loggerType, loggerLevel);
    
    osMutexRelease(mMutexLoggerId);
    
    Logger_info("Logger: Initialized with: Logger Type: %s and Logger Level: %s.", CStringConverter_ELoggerType(mLoggerType), CStringConverter_ELoggerLevel(mLoggerLevel));
}

void Logger_uninitialize(void)
{
    Logger_warning("Logger: Unitializing!");
    
    osMutexWait(mMutexLoggerId, osWaitForever);
    
    uninitialize();
    
    osMutexRelease(mMutexLoggerId);
}

void Logger_setType(ELoggerType loggerType)
{
    osMutexWait(mMutexLoggerId, osWaitForever);
    
    ELoggerType oldLoggerType = mLoggerType;
    setType(loggerType);
    
    osMutexRelease(mMutexLoggerId);
    
    Logger_info("Logger: Changed type from: %s to %s.", CStringConverter_ELoggerType(oldLoggerType), CStringConverter_ELoggerType(mLoggerType));
}

void Logger_setLevel(ELoggerLevel loggerLevel)
{
    osMutexWait(mMutexLoggerId, osWaitForever);
    
    ELoggerLevel oldLoggerLevel = mLoggerLevel;
    setLevel(loggerLevel);
    
    osMutexRelease(mMutexLoggerId);
    
    Logger_info("Logger: Changed level from: %s to %s.", CStringConverter_ELoggerLevel(oldLoggerLevel), CStringConverter_ELoggerLevel(mLoggerLevel));
}

bool Logger_isInitialized(void)
{
    return ( ELoggerLevel_Off != mLoggerLevel );
}

void Logger_registerMasterMessageLogIndCallback(void (*callback)(TLogInd*))
{
    mMasterMesssageLogIndCallback = callback;
}

void Logger_deregisterMasterMessageLogIndCallback(void)
{
    mMasterMesssageLogIndCallback = NULL;
}

void Logger_debugSystem(const char* format, ...)
{
    if (!isLogShouldBePrinted(ELogSeverity_DebugSystem))
    {
        return;
    }
    
    osMutexWait(mMutexLoggerEvalCom1Id, osWaitForever);
    
    if (ELoggerType_EvalCOM1 == mLoggerType || ELoggerType_EvalCOM1AndMasterMessage == mLoggerType)
    {
        printPrefix();
        printf("DBS: ");
        va_list vaList;
        va_start( vaList, format );
        vprintf( format, vaList );
        va_end( vaList );
        printPostfix();
    }
    
    osMutexRelease(mMutexLoggerEvalCom1Id);
}

void Logger_debugSystemMasterDataExtended(const char* format, ...)
{
    if (!isLogShouldBePrinted(ELogSeverity_DebugSystemMasterDataExtended))
    {
        return;
    }
    
    osMutexWait(mMutexLoggerEvalCom1Id, osWaitForever);
    
    if (ELoggerType_EvalCOM1 == mLoggerType || ELoggerType_EvalCOM1AndMasterMessage == mLoggerType)
    {
        printPrefix();
        printf("DME: ");
        va_list vaList;
        va_start( vaList, format );
        vprintf( format, vaList );
        va_end( vaList );
        printPostfix();
    }
    
    osMutexRelease(mMutexLoggerEvalCom1Id);
}

void Logger_debug(const char* format, ...)
{
    if (!isLogShouldBePrinted(ELogSeverity_Debug))
    {
        return;
    }
    
    osMutexWait(mMutexLoggerId, osWaitForever);
    TLogInd* logInd = NULL;
    logInd = MasterDataMemoryManager_allocate(EMessageId_LogInd);
    logInd->severity = ELogSeverity_Debug;
    va_list vaList;
    va_start( vaList, format );
    logInd->length = vsprintf( logInd->data, format, vaList );
    va_end( vaList );
    osMutexRelease(mMutexLoggerId);
    
    osMutexWait(mMutexLoggerEvalCom1Id, osWaitForever);
    if (ELoggerType_EvalCOM1 == mLoggerType || ELoggerType_EvalCOM1AndMasterMessage == mLoggerType)
    {
        printPrefix();
        printf("DBG: ");
        printf("%s", logInd->data);
        printPostfix();
    }
    osMutexRelease(mMutexLoggerEvalCom1Id);
    
    osMutexWait(mMutexLoggerMasterMessageId, osWaitForever);
    if (ELoggerType_MasterMessage == mLoggerType || ELoggerType_EvalCOM1AndMasterMessage == mLoggerType)
    {
        if (mMasterMesssageLogIndCallback)
        {
            (*mMasterMesssageLogIndCallback)(logInd);
        }
    }
    osMutexRelease(mMutexLoggerMasterMessageId);
    
    osMutexWait(mMutexLoggerId, osWaitForever);
    MasterDataMemoryManager_free(EMessageId_LogInd, logInd);
    osMutexRelease(mMutexLoggerId);
}

void Logger_info(const char* format, ...)
{
    if (!isLogShouldBePrinted(ELogSeverity_Info))
    {
        return;
    }
    
    osMutexWait(mMutexLoggerId, osWaitForever);
    TLogInd* logInd = NULL;
    logInd = MasterDataMemoryManager_allocate(EMessageId_LogInd);
    logInd->severity = ELogSeverity_Info;
    va_list vaList;
    va_start( vaList, format );
    logInd->length = vsprintf( logInd->data, format, vaList );
    va_end( vaList );
    osMutexRelease(mMutexLoggerId);
    
    osMutexWait(mMutexLoggerEvalCom1Id, osWaitForever);
    if (ELoggerType_EvalCOM1 == mLoggerType || ELoggerType_EvalCOM1AndMasterMessage == mLoggerType)
    {
        printPrefix();
        printf("INF: ");
        printf("%s", logInd->data);
        printPostfix();
    }
    osMutexRelease(mMutexLoggerEvalCom1Id);
    
    osMutexWait(mMutexLoggerMasterMessageId, osWaitForever);
    if (ELoggerType_MasterMessage == mLoggerType || ELoggerType_EvalCOM1AndMasterMessage == mLoggerType)
    {
        if (mMasterMesssageLogIndCallback)
        {
            (*mMasterMesssageLogIndCallback)(logInd);
        }
    }
    osMutexRelease(mMutexLoggerMasterMessageId);
    
    osMutexWait(mMutexLoggerId, osWaitForever);
    MasterDataMemoryManager_free(EMessageId_LogInd, logInd);
    osMutexRelease(mMutexLoggerId);
}

void Logger_warning(const char* format, ...)
{
    if (!isLogShouldBePrinted(ELogSeverity_Warning))
    {
        return;
    }
    
    osMutexWait(mMutexLoggerId, osWaitForever);
    TLogInd* logInd = NULL;
    logInd = MasterDataMemoryManager_allocate(EMessageId_LogInd);
    logInd->severity = ELogSeverity_Warning;
    va_list vaList;
    va_start( vaList, format );
    logInd->length = vsprintf( logInd->data, format, vaList );
    va_end( vaList );
    osMutexRelease(mMutexLoggerId);
    
    osMutexWait(mMutexLoggerEvalCom1Id, osWaitForever);
    if (ELoggerType_EvalCOM1 == mLoggerType || ELoggerType_EvalCOM1AndMasterMessage == mLoggerType)
    {
        printPrefix();
        printf("WRN: ");
        printf("%s", logInd->data);
        printPostfix();
    }
    osMutexRelease(mMutexLoggerEvalCom1Id);
    
    osMutexWait(mMutexLoggerMasterMessageId, osWaitForever);
    if (ELoggerType_MasterMessage == mLoggerType || ELoggerType_EvalCOM1AndMasterMessage == mLoggerType)
    {
        if (mMasterMesssageLogIndCallback)
        {
            (*mMasterMesssageLogIndCallback)(logInd);
        }
    }
    osMutexRelease(mMutexLoggerMasterMessageId);
    
    osMutexWait(mMutexLoggerId, osWaitForever);
    MasterDataMemoryManager_free(EMessageId_LogInd, logInd);
    osMutexRelease(mMutexLoggerId);
}

void Logger_error(const char* format, ...)
{
    if (!isLogShouldBePrinted(ELogSeverity_Error))
    {
        return;
    }
    
    osMutexWait(mMutexLoggerId, osWaitForever);
    TLogInd* logInd = NULL;
    logInd = MasterDataMemoryManager_allocate(EMessageId_LogInd);
    logInd->severity = ELogSeverity_Error;
    va_list vaList;
    va_start( vaList, format );
    logInd->length = vsprintf( logInd->data, format, vaList );
    va_end( vaList );
    osMutexRelease(mMutexLoggerId);
    
    osMutexWait(mMutexLoggerEvalCom1Id, osWaitForever);
    if (ELoggerType_EvalCOM1 == mLoggerType || ELoggerType_EvalCOM1AndMasterMessage == mLoggerType)
    {
        printPrefix();
        printf("ERR: ");
        printf("%s", logInd->data);
        printPostfix();
    }
    osMutexRelease(mMutexLoggerEvalCom1Id);
    
    osMutexWait(mMutexLoggerMasterMessageId, osWaitForever);
    if (ELoggerType_MasterMessage == mLoggerType || ELoggerType_EvalCOM1AndMasterMessage == mLoggerType)
    {
        if (mMasterMesssageLogIndCallback)
        {
            (*mMasterMesssageLogIndCallback)(logInd);
        }
    }
    osMutexRelease(mMutexLoggerMasterMessageId);
    
    osMutexWait(mMutexLoggerId, osWaitForever);
    MasterDataMemoryManager_free(EMessageId_LogInd, logInd);
    osMutexRelease(mMutexLoggerId);
}

void printPrefix(void)
{
    printf(NUCLEO_PRINT_PREFIX);
    printf("/");
}

void printPostfix(void)
{
    printf("\r");   
}

void initialize(ELoggerType loggerType, ELoggerLevel loggerLevel)
{
    mLoggerType = loggerType;
    mLoggerLevel = loggerLevel;
    
    if (ELoggerType_EvalCOM1AndMasterMessage == mLoggerType || ELoggerType_EvalCOM1 == mLoggerType)
    {
        if (!UART2_isInitialized())
        {
            UART2_initializeDefault();
        }
    }
}

bool isLogShouldBePrinted(ELogSeverity severity)
{
    switch (mLoggerLevel)
    {
        case ELoggerLevel_Off :
        {
            return false;
        }
        
        case ELoggerLevel_DebugSystemMasterDataExtended :
        {
            return true;
        }
        
        case ELoggerLevel_DebugSystem :
        {
            return ( ELogSeverity_DebugSystemMasterDataExtended != severity );
        }
        
        case ELoggerLevel_Debug :
        {
            return ( ( ELogSeverity_DebugSystemMasterDataExtended != severity ) && ( ELogSeverity_DebugSystem != severity ) );
        }
        
        case ELoggerLevel_Info :
        {
            return ( ( ELogSeverity_DebugSystemMasterDataExtended != severity ) && ( ELogSeverity_DebugSystem != severity ) && ( ELogSeverity_Debug != severity ) );
        }
    }
    
    return false;
}

void uninitialize(void)
{
    if (Logger_isInitialized())
    {
        if (ELoggerType_EvalCOM1AndMasterMessage == mLoggerType || ELoggerType_EvalCOM1 == mLoggerType)
        {
            UART2_uninitialize();
        }

        if (ELoggerType_EvalCOM1AndMasterMessage == mLoggerType || ELoggerType_MasterMessage == mLoggerType)
        {
            //
        }

        mLoggerType = ELoggerType_Off;
    }    
}

void setType(ELoggerType loggerType)
{
    if (Logger_isInitialized())
    {
        mLoggerType = loggerType;
    }
}

void setLevel(ELoggerLevel loggerLevel)
{
    if (Logger_isInitialized())
    {
        mLoggerLevel = loggerLevel;
    }    
}

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
  
PUTCHAR_PROTOTYPE
{
    UART2_sendByte((uint8_t)ch);
    return ch;
}
