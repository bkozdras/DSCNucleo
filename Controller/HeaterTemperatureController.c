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
#define MAX_HEATER_POWER_F  70.0

static osTimerId mControllerAlgorithmTimerId = NULL;
static osTimerId mNewControllerDataCallbackTimerId = NULL;
static osTimerId mTemperatureControllerTimerId = NULL;
static osMutexDef(mMutex);
static osMutexId mMutexId = NULL;

static bool mIsAlgorithmRunning = false;
static EControlSystemType mControlSystemType = EControlSystemType_OpenLoop;
static SPidTunes mProcessPidTunes;
static SPidTunes mModelPidTunes;
static u16 mAlgorithmExecutionPeriod = 0;
static double mAlgorithmExecutionPeriodInSeconds = 0.0;
static arm_pid_instance_f32 mProcessPid;
static arm_pid_instance_f32 mModelPid;
static float mTemperatureSetPoint = 0.0F;
static u16 mHeaterControlValue = 0.0F;
static float mTemperatureDeviation = 0.0F;
static SControllerData mControllerData;
static bool mIsControllerDataCallbackCallingEnabled = false;
static u16 mNewControllerDataCallbackExecutionPeriod = 0U;
static double mIntState = 0.0;
static double mFilterState = 0.0;
static bool mIsDerivativeElementDisabled = false;

static void (*mNewControllerDataCallback)(EControllerDataType, float) = NULL;

static void controllerAlgorithm(void const* arg);
static void newControllerDataCallback(void const* arg);
static bool startAlgorithmTimer(void);
static bool stopAlgorithmTimer(void);
static bool startNewControllerDataCallbackTimer(void);
static bool stopNewControllerDataCallbackTimer(void);
static bool restartAlgorithmTimer(void);
static double setCVInPercentScope(double cvPercent);
static u16 convertCVPercentToOutputValue(float cvPercent);
static bool setPower(u16 power);
static bool setPowerInPercent(float power);
static void initializePIDs(void);
static u16 linearizePowerOutputValueU16(u16 value);
static float linearizePowerOutputValueFloat(float value);
static const char* getLoggerPrefix(void);
static void setProcessTunesSet(u8 setNumber);
static void startTemperatureController();
static void temperatureController(void const* arg);

void HeaterTemperatureController_setup(void)
{
    mMutexId = osMutexCreate(osMutex(mMutex));
    osTimerDef(controllerAlgorithmTimer, controllerAlgorithm);
    mControllerAlgorithmTimerId = osTimerCreate(osTimer(controllerAlgorithmTimer), osTimerPeriodic, NULL);
    osTimerDef(newControllerDataCallbackTimer, newControllerDataCallback);
    mNewControllerDataCallbackTimerId = osTimerCreate(osTimer(newControllerDataCallbackTimer), osTimerPeriodic, NULL);
    osTimerDef(temperatureControllerTimer, temperatureController);
    mTemperatureControllerTimerId = osTimerCreate(osTimer(temperatureControllerTimer), osTimerPeriodic, NULL);
}

void HeaterTemperatureController_initialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    
    startTemperatureController();
    
    Logger_info("%s: Initialized!", getLoggerPrefix());
    
    setProcessTunesSet(3);
    
    // STILL VALID
    //mProcessPidTunes.kp = 2.31261376723427;
    //mProcessPidTunes.ki = 0.0061997929829211;
    //mProcessPidTunes.kd = -31.7702432308043;
    //mProcessPidTunes.n = 0.0270281069891609;
    
    /*
    mProcessPidTunes.kp = 6.34097113056392;
    mProcessPidTunes.ki = 0.00190910128875694;
    mProcessPidTunes.kd = 2834.13301399286;
    mProcessPidTunes.n = 0.012198925691036;
    */
    
    /*
    mProcessPidTunes.kp = 3.39061357893704;
    mProcessPidTunes.ki = 0.000793951763519721;
    mProcessPidTunes.kd = 1109.89646803049;
    mProcessPidTunes.n = 0.00766779321098387;
    */
    /*
    mProcessPidTunes.kp = 2.90062598618553;
    mProcessPidTunes.ki = 0.00074755344921975;
    mProcessPidTunes.kd = 860.243059516439;
    mProcessPidTunes.n = 0.00652988484158512;
    */
    /*
    mProcessPidTunes.kp = 1.63875499773753;
    mProcessPidTunes.ki = 0.0488092129660641;
    mProcessPidTunes.kd = 0.0;
    mProcessPidTunes.n = 100.0;*/
    /*
    mProcessPidTunes.kp = 0.912861916468649;
    mProcessPidTunes.ki = 0.000227386656665355;
    mProcessPidTunes.kd = 146.112540931923;
    mProcessPidTunes.n = 0.453417120031273;
    */
    osMutexRelease(mMutexId);
}

bool HeaterTemperatureController_setAlgorithmExecutionPeriod(u16 period)
{
    osMutexWait(mMutexId, osWaitForever);
    mAlgorithmExecutionPeriod = period;
    mAlgorithmExecutionPeriodInSeconds = ((double) period) / 1000.0;
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
            mTemperatureSetPoint = HeaterTemperatureReader_getTemperature();
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
        Logger_info("%s: Set %s tunes:", getLoggerPrefix(), CStringConverter_EPid(pid));
        Logger_info("%s: Kp = %.8f.", getLoggerPrefix(), tunes->kp);
        Logger_info("%s: Ki = %.8f 1/s.", getLoggerPrefix(), tunes->ki);
        Logger_info("%s: Kd = %.8f 1/s.", getLoggerPrefix(), tunes->kd);
        Logger_info("%s: N = %.8f.", getLoggerPrefix(), tunes->n);
    }
    else
    {
        Logger_error("%s: Setting %s tunes failed!", getLoggerPrefix(), CStringConverter_EPid(pid));
    }

    osMutexRelease(mMutexId);
    
    return result;
}

void HeaterTemperatureController_resetPidStates(EPid pid)
{
    osMutexWait(mMutexId, osWaitForever);
    
    mIntState = 0.0;
    mFilterState = 0.0;
    
    osMutexRelease(mMutexId);
}

void HeaterTemperatureController_enableDerivativeElement(EPid pid)
{
    osMutexWait(mMutexId, osWaitForever);
    mIsDerivativeElementDisabled = false;
    osMutexRelease(mMutexId);
}

void HeaterTemperatureController_disableDerivativeElement(EPid pid)
{
    osMutexWait(mMutexId, osWaitForever);
    mIsDerivativeElementDisabled = true;
    mFilterState = 0.0;
    osMutexRelease(mMutexId);
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

bool HeaterTemperatureController_setPowerInPercent(float power)
{
    osMutexWait(mMutexId, osWaitForever);

    bool result = false;
    
    if (MAX_HEATER_POWER_F < power)
    {
        power = MAX_HEATER_POWER_F;
    }
    
    if (EControlSystemType_OpenLoop == mControlSystemType)
    {
        result = setPowerInPercent(power);
    }
    else
    {
        Logger_warning("%s: Setting power not allowed. Power is controlled by controller algorithm...", getLoggerPrefix());
        result = false;
    }
    
    if (result)
    {
        Logger_info("%s: Set power: %u (%.2f %%).", getLoggerPrefix(), power);
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
    
    //if (0.0F > mControllerData.ERR)
    //{
    //    mControllerData.CV = 0.0F;
    //}
    //else
    
    {
        double u = mControllerData.ERR;
        double filterCoefficient = 0.0;
        if (!mIsDerivativeElementDisabled)
        {
            filterCoefficient = (mProcessPidTunes.kd * u - mFilterState) * mProcessPidTunes.n;
        }
        double calculatedCV = (mProcessPidTunes.kp * u + mIntState) + filterCoefficient;
        mControllerData.CV = setCVInPercentScope(calculatedCV);
        //if (calculatedCV == mControllerData.CV)
        //if (!(calculatedCV < mControllerData.CV))
        {
            mIntState += (mProcessPidTunes.ki * u) * mAlgorithmExecutionPeriodInSeconds;
            mFilterState += (filterCoefficient) * mAlgorithmExecutionPeriodInSeconds;
        }
    }
    /*
    {
        float calculatedCV = arm_pid_f32(&mProcessPid, mControllerData.ERR);
        mControllerData.CV = setCVInPercentScope(calculatedCV);
    }*/
    
    if (setPowerInPercent(mControllerData.CV))
    {
        mHeaterControlValue = mControllerData.CV;
    }
    else
    {
        Logger_error("%s: Setting new heater power calculated from PID algorithm failed!", getLoggerPrefix());
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
        Logger_warning("%s: Starting algorithm skipped. Algorithm is running...", getLoggerPrefix());
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
            setPower(0);
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

double setCVInPercentScope(double cvPercent)
{
    if (MAX_HEATER_POWER_F < cvPercent)
    {
        return MAX_HEATER_POWER_F;
    }
    else if (0.0 > cvPercent)
    {
        return 0.0;
    }
    
    return cvPercent;
}

u16 convertCVPercentToOutputValue(float cvPercent)
{
    return (u16)(cvPercent * 10.23F);
}

bool setPower(u16 power)
{
    //bool result = MCP4716_setOutputVoltage(linearizePowerOutputValueU16(power));
    bool result = MCP4716_setOutputVoltage(power);
    if (!result)
    {
        Logger_error("%s: New heater power (%u) not set!", getLoggerPrefix(), power);
    }
    return result;
}

bool setPowerInPercent(float power)
{
    //float linearized = linearizePowerOutputValueFloat(power);
    bool result = MCP4716_setOutputVoltage(convertCVPercentToOutputValue(power));
    if (!result)
    {
        Logger_error("%s: New heater power (%u) not set!", getLoggerPrefix(), power);
    }
    return result;
}

void initializePIDs(void)
{
    mModelPid.Kp = mModelPidTunes.kp;
    mModelPid.Ki = mModelPidTunes.ki;
    mModelPid.Kd = mModelPidTunes.kd;
    arm_pid_init_f32(&mModelPid, 1);
    
    mProcessPid.Kp = mProcessPidTunes.kp;
    mProcessPid.Ki = mProcessPidTunes.ki;
    mProcessPid.Kd = mProcessPidTunes.kd;
    arm_pid_init_f32(&mProcessPid, 1);
    
    mFilterState = 0.0;
    mIntState = 0.0;
}

u16 linearizePowerOutputValueU16(u16 value)
{
    float result;
    arm_sqrt_f32((float32_t) value, &result);
    result *= 31.984371183438951579666066332099;
    return ((u16) result);
}

float linearizePowerOutputValueFloat(float value)
{
    float result;
    arm_sqrt_f32(value, &result);
    result *= 10.0;
    return result;
}

void setProcessTunesSet(u8 setNumber)
{
    mProcessPidTunes.kp = 0.0;
    mProcessPidTunes.ki = 0.0;
    mProcessPidTunes.kd = 0.0;
    mProcessPidTunes.n = 0.0;
    
    setNumber = 3;
    
    switch (setNumber)
    {
        case 1 :
        {
            mProcessPidTunes.kp = 2.64410249977534;
            mProcessPidTunes.ki = 0.00870532124758811;
            break;
        }
        
        case 2 :
        {
            mProcessPidTunes.kp = 2.64410249977534;
            mProcessPidTunes.ki = 0.00870532124758811;
            mProcessPidTunes.kd = -10.1375289361798;
            mProcessPidTunes.n = 0.0340144807336751;
            break;
        }
        
        case 3 :
        {
            mProcessPidTunes.kp = 3.04410249977534;
            mProcessPidTunes.ki = 0.00870532124758811;
            mProcessPidTunes.kd = -10.1375289361798;
            mProcessPidTunes.n = 0.0340144807336751;
            break;
        }
        
        case 4 :
        {
            mProcessPidTunes.kp = 2.64410249977534;
            mProcessPidTunes.ki = 0.01670532124758811;
            break;
        }
        
        case 5 :
        {
            mProcessPidTunes.kp = 2.82578817096201;
            mProcessPidTunes.ki = 0.0207239549010401;
            mProcessPidTunes.kd = 13.241622388956;
            mProcessPidTunes.n = 0.211109336726062;
            break;
        }
        
        case 6 :
        {
            mProcessPidTunes.kp = 2.82578817096201;
            mProcessPidTunes.ki = 0.0407239549010401;
            mProcessPidTunes.kd = 13.241622388956;
            mProcessPidTunes.n = 0.211109336726062;
            break;
        }
        
        case 7 :
        {
            mProcessPidTunes.kp = 2.64410249977534;
            mProcessPidTunes.ki = 0.01770532124758811;
            mProcessPidTunes.kd = -10.1375289361798;
            mProcessPidTunes.n = 0.0340144807336751;
            break;
        }
        
        case 8 :
        {
            mProcessPidTunes.kp = 2.74410249977534;
            mProcessPidTunes.ki = 0.01770532124758811;
            mProcessPidTunes.kd = -11.1375289361798;
            mProcessPidTunes.n = 0.0340144807336751;
            break;
        }
        
        case 9 :
        {
            mProcessPidTunes.kp = 2.64410249977534;
            mProcessPidTunes.ki = 0.01770532124758811;
            mProcessPidTunes.kd = -10.1375289361798;
            mProcessPidTunes.n = 0.0340144807336751;
            break;
        }
        
        case 10 :
        {
            mProcessPidTunes.kp = 2.64410249977534;
            mProcessPidTunes.ki = 0.00670532124758811;
            mProcessPidTunes.kd = -10.1375289361798;
            mProcessPidTunes.n = 0.0340144807336751;
            break;
        }
        
        case 11 :
        {
            mProcessPidTunes.kp = 2.64410249977534;
            mProcessPidTunes.ki = 0.01070532124758811;
            mProcessPidTunes.kd = -10.1375289361798;
            mProcessPidTunes.n = 0.0340144807336751;
            break;
        }
        
        case 12 :
        {
            mProcessPidTunes.kp = 2.12695049442543;
            mProcessPidTunes.ki = 0.00635170219471235;
            break;
        }
        
        case 13 :
        {
            mProcessPidTunes.kp = 2.22695049442543;
            mProcessPidTunes.ki = 0.00635170219471235;
            break;
        }
        
        default :
            break;
    }
}

void startTemperatureController()
{
    osStatus osResult = osTimerStart(mTemperatureControllerTimerId, 5000);
    if (osOK != osResult)
    {
        Logger_error("%s: Starting temperature controller FAILED!", getLoggerPrefix());
        Logger_error("%s: RTOS failure: %s.", getLoggerPrefix(), CStringConverter_osStatus(osResult));
        FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
    }
    else
    {
        Logger_info("%s: Temperature range is now controlled...", getLoggerPrefix());
    }
}

void temperatureController(void const* arg)
{
    osMutexWait(mMutexId, osWaitForever);
    
    float heaterTemperature = HeaterTemperatureReader_getTemperature();
    
    if (300.0F < heaterTemperature)
    {
        if (mIsAlgorithmRunning)
        {
            stopAlgorithmTimer();
        }
        
        setPowerInPercent(0.0F);
        
        Logger_error("%s: Temperature is too high: %.2f oC.", getLoggerPrefix(), heaterTemperature);
        Logger_error("%s: Heater power set too zero...", getLoggerPrefix());
        FaultIndication_start(EFaultId_TemperatureTooHigh, EUnitId_Heater, EUnitId_Empty);
    }
    
    osMutexRelease(mMutexId);
}

const char* getLoggerPrefix(void)
{
    return "HeaterTemperatureController";
}

#undef MAX_HEATER_POWER
#undef MAX_HEATER_POWER_F
