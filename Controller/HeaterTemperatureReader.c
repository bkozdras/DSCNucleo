#include "Controller/HeaterTemperatureReader.h"

#include "Defines/CommonDefines.h"

#include "Devices/LMP90100ControlSystem.h"

#include "System/ThreadMacros.h"

#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"

#include "arm_math.h"

THREAD_DEFINES(HeaterTemperatureReader, HeaterTemperatureReader)
EVENT_HANDLER_PROTOTYPE(NewRTDValueInd)

static float mTemperature = 0.0F;
static void (*mNewTemperatureValueCallback)(float) = NULL;

static float convertRTDResistanceToTemperature(float sensorData);

THREAD(HeaterTemperatureReader)
{
    THREAD_SKELETON_START
    
        EVENT_HANDLING(NewRTDValueInd)
    
    THREAD_SKELETON_END
}

EVENT_HANDLER(NewRTDValueInd)
{
    EVENT_MESSAGE(NewRTDValueInd)
    
    Logger_debug("%s: New RTD value received! Value: %.4f Ohm.", getLoggerPrefix(), event->value);
    mTemperature = convertRTDResistanceToTemperature(event->value);
    Logger_debug("%s: Temperature: %f oC.", getLoggerPrefix(), mTemperature);
    
    if (mNewTemperatureValueCallback)
    {
        (*mNewTemperatureValueCallback)(mTemperature);
    }
}

void HeaterTemperatureReader_setup(void)
{
    THREAD_INITIALIZE_MUTEX
}

void HeaterTemperatureReader_initialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    
    LMP90100ControlSystem_registerNewValueObserver(mThreadId);
    Logger_info("%s: Initialized!", getLoggerPrefix());
    
    osMutexRelease(mMutexId);
}

float HeaterTemperatureReader_getTemperature(void)
{
    osMutexWait(mMutexId, osWaitForever);
    float temperature = mTemperature;
    osMutexRelease(mMutexId);
    return temperature;
}

void HeaterTemperatureReader_registerNewTemperatureValueCallback(void (*newTemperatureValueCallback)(float))
{
    osMutexWait(mMutexId, osWaitForever);
    mNewTemperatureValueCallback = newTemperatureValueCallback;
    osMutexRelease(mMutexId);
    Logger_debug("%s: Registered callback for new RTD temperature value.", getLoggerPrefix());
}

void HeaterTemperatureReader_deregisterNewTemperatureValueCallback(void)
{
    osMutexWait(mMutexId, osWaitForever);
    mNewTemperatureValueCallback = NULL;
    osMutexRelease(mMutexId);
    Logger_debug("%s: Deregistered callback for new RTD temperature value.", getLoggerPrefix());
}

float convertRTDResistanceToTemperature(float resistance)
{ 
    float A = 3.9083E-3F; 
    float B = -5.775E-7F;
    float R = resistance;
    float T;
    float temp;
   
    R = R / 1000.0F; 
    T = 0.0F - A;
    arm_sqrt_f32((A*A) - 4.0F * B * (1.0F - R), &temp);
    T += temp; 
    T /= (2.0F * B); 

    if( T > 0.0F && T < 200.0F)
    { 
        return T; 
    } 
    else
    { 
        T = 0.0F - A;
        arm_sqrt_f32((A*A) - 4.0F * B * (1.0F - R), &temp);
        T -= temp; 
        T /= (2.0F * B); 
        return T; 
    } 
}
