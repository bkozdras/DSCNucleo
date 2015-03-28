#ifndef _LOGGER_H_

#define _LOGGER_H_

#include "Utilities/Logger/TypesLogger.h"
#include "Utilities/Printer/CStringConverter.h"
#include "stdbool.h"

void Logger_setup(void);
void Logger_initialize(ELoggerType loggerType, ELoggerLevel loggerLevel);
void Logger_uninitialize(void);
void Logger_setType(ELoggerType loggerType);
void Logger_setLevel(ELoggerLevel loggerLevel);

bool Logger_isInitialized(void);

void Logger_debug(const char* format, ...);
void Logger_info(const char* format, ...);
void Logger_warning(const char* format, ...);
void Logger_error(const char* format, ...);

#endif
