#include "Controller/ReferenceThermocoupleTemperatureReader.h"

#include "Defines/CommonDefines.h"

#include "Devices/LMP90100SignalsMeasurement.h"

#include "System/ThreadMacros.h"

#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"

#include "arm_math.h"

THREAD_DEFINES(ReferenceThermocoupleTemperatureReader, ReferenceThermocoupleTemperatureReader)
EVENT_HANDLER_PROTOTYPE(NewRTDValueInd)

static float mRtdTemperature = 0.0F;
static float mRTDPolynomialCoefficients [3] =
    {
        -242.02,    /*R0*/
        2.2228,     /*R1*/
        2.5859E-3   /*R2*/
    };
static void (*mDataReadyCallback)(float) = NULL;

static float convertRTDResistanceToTemperature(float resistance);
    
THREAD(ReferenceThermocoupleTemperatureReader)
{
    THREAD_SKELETON_START
    
        EVENT_HANDLING(NewRTDValueInd)
    
    THREAD_SKELETON_END
}

EVENT_HANDLER(NewRTDValueInd)
{
    EVENT_MESSAGE(NewRTDValueInd)
    
    Logger_debug("%s: New RTD value received: %.4f Ohm.", getLoggerPrefix(), event->value);
    mRtdTemperature = convertRTDResistanceToTemperature(event->value);
    Logger_debug("%s: RTD temperature: %.4f oC.", getLoggerPrefix(), mRtdTemperature);
    
    if (mDataReadyCallback)
    {
        Logger_debug("%s: Callback for data ready registered. Processing with stored data...", getLoggerPrefix());
        (*mDataReadyCallback)(mRtdTemperature);
    }
    else
    {
        Logger_debug("%s: Callback for data ready is not registered. Processing with stored data skipped.");
    }
}

void ReferenceThermocoupleTemperatureReader_setup(void)
{
    THREAD_INITIALIZE_MUTEX
}

void ReferenceThermocoupleTemperatureReader_initialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    
    LMP90100SignalsMeasurement_registerNewValueObserver(ELMP90100Rtd_2, mThreadId);
    
    osMutexRelease(mMutexId);
    
    Logger_info("%s: Initialized!", getLoggerPrefix());
}

float ReferenceThermocoupleTemperatureReader_getTemperature(void)
{
    osMutexWait(mMutexId, osWaitForever);
    float rtdTemperature = mRtdTemperature;
    osMutexRelease(mMutexId);
    return rtdTemperature;
}

void ReferenceThermocoupleTemperatureReader_registerDataReadyCallback(void (*dataReadyCallback)(float))
{
    osMutexWait(mMutexId, osWaitForever);
    mDataReadyCallback = dataReadyCallback;
    Logger_debug("%s: Data ready callback registered.", getLoggerPrefix());
    osMutexRelease(mMutexId);
}

void ReferenceThermocoupleTemperatureReader_deregisterDataReadyCallback(void)
{
    osMutexWait(mMutexId, osWaitForever);
    mDataReadyCallback = NULL;
    Logger_debug("%s: Data ready callback deregistered.", getLoggerPrefix());
    osMutexRelease(mMutexId);
}

float convertRTDResistanceToTemperature(float resistance)
{
    float T = mRTDPolynomialCoefficients[0];
    for (u8 iter = 1; 3 > iter; ++iter)
    {
        T += mRTDPolynomialCoefficients[iter] * resistance;
        resistance *= resistance;
    }
    return T;
}
