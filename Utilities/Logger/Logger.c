#include "cmsis_os.h"

#include "Utilities/Logger/TypesLogger.h"
#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"
#include "Peripherals/UART2.h"
#include "Peripherals/UART1.h"
#include "SharedDefines/PrinterPrefixes.h"
#include "stdio.h"
#include "stdarg.h"

osMutexDef(mMutexLogger);
osMutexId mMutexLoggerId = NULL;

static ELoggerType mLoggerType = ELoggerType_Off;
static ELoggerLevel mLoggerLevel = ELoggerLevel_Off;

static void printPrefix(void);
static void printPostfix(void);
static void initialize(ELoggerType loggerType, ELoggerLevel loggerLevel);
static void uninitialize(void);
static void setType(ELoggerType loggerType);
static void setLevel(ELoggerLevel loggerLevel);

void Logger_setup(void)
{
    if (!mMutexLoggerId)
    {
        mMutexLoggerId = osMutexCreate( osMutex(mMutexLogger) );
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
    
    Logger_info("Logger: Changed type from: %s to %s.", CStringConverter_ELoggerLevel(oldLoggerLevel), CStringConverter_ELoggerLevel(mLoggerLevel));
}

bool Logger_isInitialized(void)
{
    return ( ELoggerLevel_Off != mLoggerLevel );
}

void Logger_debugSystem(const char* format, ...)
{
    if (!(Logger_isInitialized() && ( ELoggerLevel_DebugSystem == mLoggerLevel )))
    {
        return;
    }
    
    osMutexWait(mMutexLoggerId, osWaitForever);
    
    if (ELoggerType_EvalCOM1 == mLoggerType)
    {
        printPrefix();
        printf("DBS: ");
    }
    va_list vaList;
    va_start( vaList, format );
    vprintf( format, vaList );
    va_end( vaList );
    if (ELoggerType_EvalCOM1 == mLoggerType)
    {
        printPostfix();
    }
    
    osMutexRelease(mMutexLoggerId);
}

void Logger_debug(const char* format, ...)
{
    if (!(Logger_isInitialized() && ( (ELoggerLevel_Debug == mLoggerLevel) || (ELoggerLevel_DebugSystem == mLoggerLevel) )))
    {
        return;
    }
    
    osMutexWait(mMutexLoggerId, osWaitForever);
    
    if (ELoggerType_EvalCOM1 == mLoggerType)
    {
        printPrefix();
        printf("DBG: ");
    }
    va_list vaList;
    va_start( vaList, format );
    vprintf( format, vaList );
    va_end( vaList );
    if (ELoggerType_EvalCOM1 == mLoggerType)
    {
        printPostfix();
    }
    
    osMutexRelease(mMutexLoggerId);
}

void Logger_info(const char* format, ...)
{
    osMutexWait(mMutexLoggerId, osWaitForever);
    
    if (Logger_isInitialized())
    {
        if (ELoggerType_EvalCOM1 == mLoggerType)
        {
            printPrefix();
            printf("INF: ");
        }
        va_list vaList;
        va_start( vaList, format );
        vprintf( format, vaList );
        va_end( vaList );
        if (ELoggerType_EvalCOM1 == mLoggerType)
        {
            printPostfix();
        }
    } 

    osMutexRelease(mMutexLoggerId);    
}

void Logger_warning(const char* format, ...)
{
    osMutexWait(mMutexLoggerId, osWaitForever);
    
    if (Logger_isInitialized())
    {
        if (ELoggerType_EvalCOM1 == mLoggerType)
        {
            printPrefix();
            printf("WRN: ");
        }
        va_list vaList;
        va_start( vaList, format );
        vprintf( format, vaList );
        va_end( vaList );
        if (ELoggerType_EvalCOM1 == mLoggerType)
        {
            printPostfix();
        }
    }

    osMutexRelease(mMutexLoggerId);    
}

void Logger_error(const char* format, ...)
{
    osMutexWait(mMutexLoggerId, osWaitForever);
    
    if (Logger_isInitialized())
    {
        if (ELoggerType_EvalCOM1 == mLoggerType)
        {
            printPrefix();
            printf("ERR: ");
        }
        va_list vaList;
        va_start( vaList, format );
        vprintf( format, vaList );
        va_end( vaList );
        if (ELoggerType_EvalCOM1 == mLoggerType)
        {
            printPostfix();
        }
    }

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
    if (!Logger_isInitialized())
    {
        mLoggerType = loggerType;
        mLoggerLevel = loggerLevel;
        
        if (ELoggerType_EvalCOM1AndATCommand == mLoggerType || ELoggerType_EvalCOM1 == mLoggerType)
        {
            UART2_initializeDefault();
        }
        
        if (ELoggerType_EvalCOM1AndATCommand == mLoggerType || ELoggerType_ATCommand == mLoggerType)
        {
            //
        }
    }    
}

void uninitialize(void)
{
    if (Logger_isInitialized())
    {
        if (ELoggerType_EvalCOM1AndATCommand == mLoggerType || ELoggerType_EvalCOM1 == mLoggerType)
        {
            UART2_uninitialize();
        }

        if (ELoggerType_EvalCOM1AndATCommand == mLoggerType || ELoggerType_ATCommand == mLoggerType)
        {
            //
        }

        mLoggerType = ELoggerType_Off;
    }    
}

void setType(ELoggerType loggerType)
{
    ELoggerLevel actualLoggerLevel = mLoggerLevel;
    uninitialize();
    initialize(loggerType, actualLoggerLevel);    
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
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
  //while(__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_TXE) == RESET);
  //HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 0xFFFF); 
  UART2_sendByte((uint8_t)ch);
    
  return ch;
}
