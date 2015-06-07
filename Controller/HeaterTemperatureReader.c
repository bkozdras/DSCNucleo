#include "Controller/HeaterTemperatureReader.h"

#include "Defines/CommonDefines.h"

#include "Devices/LMP90100ControlSystem.h"

#include "FaultManagement/FaultIndication.h"
#include "System/ThreadMacros.h"

#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"

#include "arm_math.h"

THREAD_DEFINES(HeaterTemperatureReader, HeaterTemperatureReader)
EVENT_HANDLER_PROTOTYPE(NewRTDValueInd)

static float mTemperature = 0.0F;
static void (*mNewTemperatureValueCallback)(float) = NULL;
static u16 mIndCallbackPeriod = 0U;
static osTimerId mIndCallbackTimerId = NULL;
static bool mIsTimerStarted = false;

static float convertRTDResistanceToTemperature(float sensorData);
static void indCallback(const void* arg);
static bool startIndCallbackTimer(void);
static bool stopIndCallbackTimer(void);

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
}

void HeaterTemperatureReader_setup(void)
{
    THREAD_INITIALIZE_MUTEX
    
    osTimerDef(indCallbackTimer, indCallback);
    mIndCallbackTimerId = osTimerCreate(osTimer(indCallbackTimer), osTimerPeriodic, NULL);
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

bool HeaterTemperatureReader_registerNewTemperatureValueCallback(void (*newTemperatureValueCallback)(float), u16 period)
{
    osMutexWait(mMutexId, osWaitForever);
    mNewTemperatureValueCallback = newTemperatureValueCallback;
    mIndCallbackPeriod = period;
    
    bool result = true;
    
    if (!mIsTimerStarted)
    {
        result = startIndCallbackTimer();
    }
    
    osMutexRelease(mMutexId);
    Logger_debug("%s: Registered callback for new RTD temperature value.", getLoggerPrefix());
    
    return result;
}

bool HeaterTemperatureReader_deregisterNewTemperatureValueCallback(void)
{
    osMutexWait(mMutexId, osWaitForever);
    mNewTemperatureValueCallback = NULL;
    
    bool result = true;
    if (mIsTimerStarted)
    {
        result = stopIndCallbackTimer();
    }
    
    osMutexRelease(mMutexId);
    Logger_debug("%s: Deregistered callback for new RTD temperature value.", getLoggerPrefix());
    
    return result;
}

float convertRTDResistanceToTemperature(float resistance)
{ 
    /*
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
    }*/
    
    const float z1 = -3.9083E-3;
    const float z2 = 1.758480889E-5;
    const float z3 = -2.310E-9;
    const float z4 = -1.155E-6;
    
    float sqrtVal = 0.0F;
    arm_sqrt_f32(z2 + z3 * resistance, &sqrtVal);
    return ( (z1 + sqrtVal) / (z4) );
}

void indCallback(const void* arg)
{
    osMutexWait(mMutexId, osWaitForever);
    
    if (mNewTemperatureValueCallback)
    {
        (*mNewTemperatureValueCallback)(mTemperature);
    }
    
    osMutexRelease(mMutexId);
}

bool startIndCallbackTimer(void)
{
    if (!mIsTimerStarted)
    {
        osStatus osResult = osTimerStart(mIndCallbackTimerId, mIndCallbackPeriod);
        if (osOK != osResult)
        {
            Logger_error("%s: Forcing callback calling state to ENABLED failed.", getLoggerPrefix());
            Logger_error("%s: RTOS failure: %s.", getLoggerPrefix(), CStringConverter_osStatus(osResult));
            FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
            return false;
        }
        else
        {
            mIsTimerStarted = true;
            return true;
        }
    }
    
    return true;
}

bool stopIndCallbackTimer(void)
{
    if (mIsTimerStarted)
    {
        osStatus osResult = osTimerStop(mIndCallbackTimerId);
        if (osOK != osResult)
        {
            Logger_error("%s: Forcing callback calling state to ENABLED failed.", getLoggerPrefix());
            Logger_error("%s: RTOS failure: %s.", getLoggerPrefix(), CStringConverter_osStatus(osResult));
            FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
            return false;
        }
        else
        {
            mIsTimerStarted = false;
            return true;
        }
    }
    
    return true;
}
