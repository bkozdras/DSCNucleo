#include "Controller/ReferenceTemperatureReader.h"

#include "Defines/CommonDefines.h"

#include "Devices/LMP90100SignalsMeasurement.h"

#include "System/ThreadMacros.h"

#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"

#include "arm_math.h"

THREAD_DEFINES(ReferenceTemperatureReader, ReferenceTemperatureReader)
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
    
THREAD(ReferenceTemperatureReader)
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
        Logger_debug("%s: Callback for data ready is not registered. Processing with stored data skipped.", getLoggerPrefix());
    }
}

void ReferenceTemperatureReader_setup(void)
{
    THREAD_INITIALIZE_MUTEX
}

void ReferenceTemperatureReader_initialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    
    LMP90100SignalsMeasurement_registerNewValueObserver(ELMP90100Rtd_2, mThreadId);
    
    osMutexRelease(mMutexId);
    
    Logger_info("%s: Initialized!", getLoggerPrefix());
}

float ReferenceTemperatureReader_getTemperature(void)
{
    osMutexWait(mMutexId, osWaitForever);
    float rtdTemperature = mRtdTemperature;
    osMutexRelease(mMutexId);
    return rtdTemperature;
}

void ReferenceTemperatureReader_registerDataReadyCallback(void (*dataReadyCallback)(float))
{
    osMutexWait(mMutexId, osWaitForever);
    mDataReadyCallback = dataReadyCallback;
    Logger_debug("%s: Data ready callback registered.", getLoggerPrefix());
    osMutexRelease(mMutexId);
}

void ReferenceTemperatureReader_deregisterDataReadyCallback(void)
{
    osMutexWait(mMutexId, osWaitForever);
    mDataReadyCallback = NULL;
    Logger_debug("%s: Data ready callback deregistered.", getLoggerPrefix());
    osMutexRelease(mMutexId);
}

float convertRTDResistanceToTemperature(float resistance)
{
    /*
    float T = mRTDPolynomialCoefficients[0];
    for (u8 iter = 1; 3 > iter; ++iter)
    {
        T += mRTDPolynomialCoefficients[iter] * resistance;
        resistance *= resistance;
    }
    return T;*/
    
    //const float R0 = 100;
    //float sqrtVal;
    //arm_sqrt_f32(R0 * R0 * + 3.9083E-3 * 3.9083E-3 - 4 * R0 * -5.775E-7 * (R0 - resistance), &sqrtVal);
    //return ((-R0 * 3.9083E-3 + sqrtVal) / (2 * R0 * -5.775E-7));
    
    const float z1 = -3.9083E-3;
    const float z2 = 17.58480889E-6;
    const float z3 = -23.10E-9;
    const float z4 = -1.155E-6;
    
    float sqrtVal = 0.0F;
    arm_sqrt_f32(z2 + z3 * resistance, &sqrtVal);
    return ( (z1 + sqrtVal) / (z4) );
}
