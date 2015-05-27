#include "Controller/HeaterTemperatureController.h"

#include "Controller/HeaterTemperatureReader.h"
#include "Devices/MCP4716.h"
#include "FaultManagement/FaultIndication.h"
#include "Utilities/Printer/CStringConverter.h"
#include "Utilities/Logger/Logger.h"
#include "Utilities/CopyObject.h"

#include "arm_math.h"
#include "cmsis_os.h"

#define MAX_HEATER_POWER    1023
#define MAX_HEATER_POWER_F  1023.0F

static osTimerId mControllerAlgorithmTimerId = NULL;
static osTimerId mNewControllerDataCallbackTimerId = NULL;
static osMutexDef(mMutex);
static osMutexId mMutexId = NULL;

static bool mIsAlgorithmRunning = false;
static EControlSystemType mControlSystemType = EControlSystemType_OpenLoop;
static SPidTunes mProcessPidTunes;
static SPidTunes mModelPidTunes;
static u16 mAlgorithmExecutionPeriod = 0;
static arm_pid_instance_f32 mProcessPid;
static arm_pid_instance_f32 mModelPid;
static float mTemperatureSetPoint = 0.0F;
static u16 mHeaterControlValue = 0.0F;
static float mTemperatureDeviation = 0.0F;
static SControllerData mControllerData;
static bool mIsControllerDataCallbackCallingEnabled = false;
static u16 mNewControllerDataCallbackExecutionPeriod = 0U;

static void (*mNewControllerDataCallback)(EControllerDataType, float) = NULL;

static void controllerAlgorithm(void const* arg);
static void newControllerDataCallback(void const* arg);
static bool startAlgorithmTimer(void);
static bool stopAlgorithmTimer(void);
static bool startNewControllerDataCallbackTimer(void);
static bool stopNewControllerDataCallbackTimer(void);
static bool restartAlgorithmTimer(void);
static u16 setCVInScope(float cv);
static bool setPower(u16 power);
static void initializePIDs(void);
static const char* getLoggerPrefix(void);

void HeaterTemperatureController_setup(void)
{
    mMutexId = osMutexCreate(osMutex(mMutex));
    osTimerDef(controllerAlgorithmTimer, controllerAlgorithm);
    mControllerAlgorithmTimerId = osTimerCreate(osTimer(controllerAlgorithmTimer), osTimerPeriodic, NULL);
    osTimerDef(newControllerDataCallbackTimer, newControllerDataCallback);
    mNewControllerDataCallbackTimerId = osTimerCreate(osTimer(newControllerDataCallbackTimer), osTimerPeriodic, NULL);
}

void HeaterTemperatureController_initialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    Logger_info("%s: Initialized!", getLoggerPrefix());
    osMutexRelease(mMutexId);
}

bool HeaterTemperatureController_setAlgorithmExecutionPeriod(u16 period)
{
    osMutexWait(mMutexId, osWaitForever);
    mAlgorithmExecutionPeriod = period;
    Logger_debug("%s: Setting controller algorithm execution period to %u ms.", getLoggerPrefix(), mAlgorithmExecutionPeriod);
    
    bool result = true;
    if (mIsAlgorithmRunning)
    {
        result = restartAlgorithmTimer();
    }
    
    if (result)
    {
        Logger_info("%s: Set controller algorithm execution period to %u ms.", getLoggerPrefix(), mAlgorithmExecutionPeriod);
    }
    else
    {
        Logger_error("%s: Setting controller algorithm execution period to %u ms failed.", getLoggerPrefix(), mAlgorithmExecutionPeriod);
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

bool HeaterTemperatureController_setProcessModelParameters(SProcessModelParameters* parameters)
{
    osMutexWait(mMutexId, osWaitForever);
    // Action with Inertial Model
    osMutexRelease(mMutexId);
    return false;
}

bool HeaterTemperatureController_setSystemType(EControlSystemType type)
{
    osMutexWait(mMutexId, osWaitForever);
    
    bool result = true;
    
    if (EControlSystemType_OpenLoop == type && mIsAlgorithmRunning)
    {
        Logger_info("%s: Changing system type from Feedback to Open Loop. Stopping controller algorithm...");
        result = stopAlgorithmTimer();
    }
    
    if (result)
    {
        Logger_info
        (
            "%s: Changed controller system type from %s to %s.",
            getLoggerPrefix(),
            CStringConverter_EControlSystemType(mControlSystemType),
            CStringConverter_EControlSystemType(type)
        );
        mControlSystemType = type;
        
        if (EControlSystemType_OpenLoop != mControlSystemType && !mIsAlgorithmRunning)
        {
            result = startAlgorithmTimer();
        }
    }
    else
    {
        Logger_error
        (
            "%s: Changing controller system type from %s to %s failed!",
            getLoggerPrefix(),
            CStringConverter_EControlSystemType(mControlSystemType),
            CStringConverter_EControlSystemType(type)
        );
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

bool HeaterTemperatureController_setTunes(EPid pid, SPidTunes* tunes)
{
    osMutexWait(mMutexId, osWaitForever);
    
    bool result = true;
    if (EPid_ModelController == pid)
    {
        CopyObject_SPidTunes(tunes, &mModelPidTunes);
    }
    else if (EPid_ProcessController == pid)
    {
        CopyObject_SPidTunes(tunes, &mProcessPidTunes);
    }
    
    if (mIsAlgorithmRunning)
    {
        result = restartAlgorithmTimer();
    }
    
    if (result)
    {
        Logger_info
        (
            "%s: Set %s tunes: Kp = %.2f, Ti = %.2f and Td = %.2f.",
            getLoggerPrefix(),
            tunes->kp,
            tunes->ti,
            tunes->td
        );
    }
    else
    {
        Logger_error("%s: Setting %s tunes failed!", getLoggerPrefix());
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

bool HeaterTemperatureController_setPower(u16 power)
{
    osMutexWait(mMutexId, osWaitForever);

    bool result = false;
    
    if (MAX_HEATER_POWER < power)
    {
        power = MAX_HEATER_POWER;
    }
    
    if (EControlSystemType_OpenLoop == mControlSystemType)
    {
        result = setPower(power);
    }
    else
    {
        Logger_warning("%s: Setting power not allowed. Power is controlled by controller algorithm...", getLoggerPrefix());
        result = false;
    }
    
    if (result)
    {
        Logger_info("%s: Set power: %u (%.2f %%).", getLoggerPrefix(), power, ( ( (float) power ) / MAX_HEATER_POWER_F ) * 100.0F );
    }
    else
    {
        Logger_error("%s: Power not set!", getLoggerPrefix());
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

bool HeaterTemperatureController_setTemperature(float temperature)
{
    osMutexWait(mMutexId, osWaitForever);
    
    mTemperatureSetPoint = temperature;
    
    osMutexRelease(mMutexId);
    
    return true;
}

float HeaterTemperatureController_getControllerError(void)
{
    osMutexWait(mMutexId, osWaitForever);
    float controllerError = mTemperatureDeviation;
    osMutexRelease(mMutexId);
    return controllerError;
}

bool HeaterTemperatureController_start(void)
{
    osMutexWait(mMutexId, osWaitForever);
    bool result = startAlgorithmTimer();
    osMutexRelease(mMutexId);
    return result;
}

bool HeaterTemperatureController_stop(void)
{
    osMutexWait(mMutexId, osWaitForever);
    bool result = stopAlgorithmTimer();
    osMutexRelease(mMutexId);
    return result;
}

bool HeaterTemperatureController_registerNewControllerDataCallback(void (*callback)(EControllerDataType, float), u16 period)
{
    osMutexWait(mMutexId, osWaitForever);
    mNewControllerDataCallback = callback;
    
    bool result = true;
    mNewControllerDataCallbackExecutionPeriod = period;
    
    if (mIsAlgorithmRunning)
    {
        result = startNewControllerDataCallbackTimer();
    }
    
    osMutexRelease(mMutexId);
    Logger_debug("%s: New controller data callback registered.", getLoggerPrefix());
    
    return result;
}

bool HeaterTemperatureController_deregisterNewControllerDataCallback(void)
{
    osMutexWait(mMutexId, osWaitForever);
    mNewControllerDataCallback = NULL;
    bool result = true;
    
    if (mIsAlgorithmRunning)
    {
        result = stopNewControllerDataCallbackTimer();
    }
    
    osMutexRelease(mMutexId);
    Logger_debug("%s: New controller data callback deregistered.", getLoggerPrefix());
    
    return result;
}

void controllerAlgorithm(void const* arg)
{
    osMutexWait(mMutexId, osWaitForever);
    
    mControllerData.SP = mTemperatureSetPoint;
    mControllerData.PV = HeaterTemperatureReader_getTemperature();
    mControllerData.ERR = mControllerData.SP - mControllerData.PV;
    mTemperatureDeviation = mControllerData.ERR;
    
    float calculatedCV = arm_pid_f32(&mProcessPid, mControllerData.ERR);
    mControllerData.CV = setCVInScope(calculatedCV);
    
    if (mHeaterControlValue != mControllerData.CV)
    {
        if (setPower(mControllerData.CV))
        {
            mHeaterControlValue = mControllerData.CV;
        }
        else
        {
            Logger_error("%s: Setting new heater power calculated from PID algorithm failed!", getLoggerPrefix());
        }
    }
    
    osMutexRelease(mMutexId);
}

void newControllerDataCallback(void const* arg)
{
    osMutexWait(mMutexId, osWaitForever);
    
    if (mNewControllerDataCallback)
    {
        (*mNewControllerDataCallback)(EControllerDataType_SP, mControllerData.SP);
        (*mNewControllerDataCallback)(EControllerDataType_CV, (float)(mControllerData.CV));
        (*mNewControllerDataCallback)(EControllerDataType_PV, mControllerData.PV);
        (*mNewControllerDataCallback)(EControllerDataType_ERR, mControllerData.ERR);
    }
    
    osMutexRelease(mMutexId);
}

bool startAlgorithmTimer(void)
{
    if (mIsAlgorithmRunning)
    {
        Logger_warning("%s: Starting algorithm skipped. Algorithm is running...");
        return true;
    }
    else
    {
        initializePIDs();
        osStatus result = osTimerStart(mControllerAlgorithmTimerId, mAlgorithmExecutionPeriod);
        if (osOK == result)
        {
            mIsAlgorithmRunning = true;
            Logger_info("%s: Forcing algorithm state to RUN done (Period: %u ms). Heater temperature is now controlled.", getLoggerPrefix(), mAlgorithmExecutionPeriod);
            return startNewControllerDataCallbackTimer();
        }
        else
        {
            Logger_error("%s: Forcing algorithm state to RUN failed.", getLoggerPrefix());
            Logger_error("%s: RTOS failure: %s.", getLoggerPrefix(), CStringConverter_osStatus(result));
            FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
            return false;
        }
    }
}

bool stopAlgorithmTimer(void)
{
    if (!mIsAlgorithmRunning)
    {
        Logger_warning("%s: Stopping algorithm skipped. Algorithm is not running...");
        return true;
    }
    else
    {
        osStatus result = osTimerStop(mControllerAlgorithmTimerId);
        if (osOK == result)
        {
            mIsAlgorithmRunning = false;
            Logger_info("%s: Forcing algorithm state to IDLE done. Heater temperature is not controlled now.", getLoggerPrefix());
            return stopNewControllerDataCallbackTimer();
        }
        else
        {
            Logger_error("%s: Forcing algorithm state to IDLE failed.", getLoggerPrefix());
            Logger_error("%s: RTOS failure: %s.", getLoggerPrefix(), CStringConverter_osStatus(result));
            FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
            return false;
        }
    }
}

bool startNewControllerDataCallbackTimer(void)
{
    if (!mIsControllerDataCallbackCallingEnabled)
    {
        osStatus osResult = osTimerStart(mNewControllerDataCallbackTimerId, mNewControllerDataCallbackExecutionPeriod);
        if (osOK != osResult)
        {
            Logger_error("%s: Forcing callback calling state to ENABLED failed.", getLoggerPrefix());
            Logger_error("%s: RTOS failure: %s.", getLoggerPrefix(), CStringConverter_osStatus(osResult));
            FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
            return false;
        }
        else
        {
            mIsControllerDataCallbackCallingEnabled = true;
            return true;
        }
    }
    
    return true;
}

bool stopNewControllerDataCallbackTimer(void)
{
    if (mIsControllerDataCallbackCallingEnabled)
    {
        osStatus osResult = osTimerStop(mNewControllerDataCallbackTimerId);
        if (osOK != osResult)
        {
            Logger_error("%s: Forcing callback calling state to DISABLED failed.", getLoggerPrefix());
            Logger_error("%s: RTOS failure: %s.", getLoggerPrefix(), CStringConverter_osStatus(osResult));
            FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
            return false;
        }
        else
        {
            mIsControllerDataCallbackCallingEnabled = false;
            return true;
        }
    }
    
    return true;
}

bool restartAlgorithmTimer(void)
{
    if (!mIsAlgorithmRunning)
    {
        Logger_warning("%s: Restarting algorithm timer not needed. Algorithm is not running...", getLoggerPrefix());
        return true;
    }
    else
    {
        bool result = stopAlgorithmTimer();
        if (result)
        {
            result = startAlgorithmTimer();
        }
        return result;
    }
}

u16 setCVInScope(float cv)
{
    if (MAX_HEATER_POWER_F < cv)
    {
        return MAX_HEATER_POWER;
    }
    else if (0.0F > cv)
    {
        return 0;
    }
    else
    {
        return (u16) cv;
    }
}

bool setPower(u16 power)
{
    bool result = MCP4716_setOutputVoltage(power);
    if (!result)
    {
        Logger_error("%s: New heater power (%u) not set!", getLoggerPrefix(), power);
    }
    return result;
}

void initializePIDs(void)
{
    mModelPid.Kp = mModelPidTunes.kp;
    mModelPid.Ki = (0.0F != mModelPidTunes.ti) ? (1.0F / mModelPidTunes.ti) : 0.0F;
    mModelPid.Kd = (0.0F != mModelPidTunes.td) ? (1.0F / mModelPidTunes.td) : 0.0F;
    arm_pid_init_f32(&mModelPid, 1);
    
    mProcessPid.Kp = mProcessPidTunes.kp;
    mProcessPid.Ki = (0.0F != mProcessPidTunes.ti) ? (1.0F / mProcessPidTunes.ti) : 0.0F;
    mProcessPid.Kd = (0.0F != mProcessPidTunes.td) ? (1.0F / mProcessPidTunes.td) : 0.0F;
    arm_pid_init_f32(&mProcessPid, 1);
}

const char* getLoggerPrefix(void)
{
    return "HeaterTemperatureController";
}

#undef MAX_HEATER_POWER
#undef MAX_HEATER_POWER_F
