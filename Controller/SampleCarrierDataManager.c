#include "Controller/SampleCarrierDataManager.h"

#include "Defines/CommonDefines.h"

#include "Devices/ADS1248.h"
#include "Devices/LMP90100SignalsMeasurement.h"

#include "System/ThreadMacros.h"

#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"

#include "arm_math.h"

THREAD_DEFINES(SampleCarrierDataManager, SampleCarrierDataManager)
EVENT_HANDLER_PROTOTYPE(NewRTDValueInd)
EVENT_HANDLER_PROTOTYPE(NewThermocoupleVoltageValueInd)

static SSampleCarrierData mSampleCarrierData;

static SRTDPolynomialCoefficients mRTDPolynomialCoefficients =
    {
        -242.02,    /*R0*/
        2.2228,     /*R1*/
        2.5859E-3,  /*R2*/
        4.8260E-6,  /*R3*/
        2.8183E-8,  /*R4*/
        1.5243E-10  /*R5*/
    };
static bool mThermocouplesDataReceived [THERMOCOUPLES_COUNT];
static bool mRTDDataReceived = false;
static void (*mDataReadyCallback)(SSampleCarrierData*) = NULL;

static void processIfSampleCarrierDataIsReady(void);
//static bool isSampleCarrierDataReady(void);
static void storeReceivedThermocoupleData(EUnitId thermocouple, double data);
static void storeReceivedRTDData(double data);
static void copySampleCarrierData(SSampleCarrierData* source, SSampleCarrierData* destination);
static void cleanUpDataReceivedVariables(void);
static double convertRTDResistanceToTemperature(double resistance);
static double convertThermocoupleVoltageValueToMicrovolts(double valueInVolts);

THREAD(SampleCarrierDataManager)
{
    THREAD_SKELETON_START
    
        EVENT_HANDLING(NewRTDValueInd)
        EVENT_HANDLING(NewThermocoupleVoltageValueInd)
    
    THREAD_SKELETON_END
}

EVENT_HANDLER(NewRTDValueInd)
{
    EVENT_MESSAGE(NewRTDValueInd)
    
    Logger_debug("%s: Received new RTD (Pt100) value: %.4f Ohm.", getLoggerPrefix(), event->value);
    
    double rtdTemperature = convertRTDResistanceToTemperature(event->value);
    Logger_debug("%s: RTD temperature: %.4f oC. Storing data.", getLoggerPrefix(), rtdTemperature);
    storeReceivedRTDData(rtdTemperature);
    processIfSampleCarrierDataIsReady();
}

EVENT_HANDLER(NewThermocoupleVoltageValueInd)
{
    EVENT_MESSAGE(NewThermocoupleVoltageValueInd)
    
    Logger_debug("%s: Received new %s value: %.6f V.", getLoggerPrefix(), CStringConverter_EUnitId(event->thermocouple), event->value);
    
    double thermocoupleMicroVoltValue = convertThermocoupleVoltageValueToMicrovolts(event->value);
    Logger_debug("%s: %s value: %.5f uV. Storing data.", getLoggerPrefix(), CStringConverter_EUnitId(event->thermocouple), thermocoupleMicroVoltValue);
    storeReceivedThermocoupleData(event->thermocouple, thermocoupleMicroVoltValue);
    processIfSampleCarrierDataIsReady();
}

void SampleCarrierDataManager_setup(void)
{
    THREAD_INITIALIZE_MUTEX
    cleanUpDataReceivedVariables();
}

void SampleCarrierDataManager_initialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    
    ADS1248_registerNewValueObserver(mThreadId);
    LMP90100SignalsMeasurement_registerNewValueObserver(ELMP90100Rtd_1, mThreadId);
    
    Logger_info("%s: Initialized!", getLoggerPrefix());
    
    osMutexRelease(mMutexId);
    
    Logger_info("%s: Initialized!", getLoggerPrefix());
}

void SampleCarrierDataManager_getData(SSampleCarrierData* data)
{
    osMutexWait(mMutexId, osWaitForever);
    copySampleCarrierData(&mSampleCarrierData, data);
    osMutexRelease(mMutexId);
}

void SampleCarrierManager_setRTDPolynomialCoefficients(SRTDPolynomialCoefficients* coefficients)
{
    osMutexWait(mMutexId, osWaitForever);
    for (u8 iter = 0; RTD_POLYNOMIAL_DEGREE >= iter; ++iter)
    {
        mRTDPolynomialCoefficients.values[iter] = coefficients->values[iter];
    }
    osMutexRelease(mMutexId);
}

void SampleCarrierDataManager_registerDataReadyCallback(void (*dataReadyCallback)(SSampleCarrierData*))
{
    osMutexWait(mMutexId, osWaitForever);
    mDataReadyCallback = dataReadyCallback;
    Logger_debug("%s: Data ready callback registered.", getLoggerPrefix());
    osMutexRelease(mMutexId);
}

void SampleCarrierDataManager_deregisterDataReadyCallback(void)
{
    osMutexWait(mMutexId, osWaitForever);
    mDataReadyCallback = NULL;
    Logger_debug("%s: Data ready callback deregistered.", getLoggerPrefix());
    osMutexRelease(mMutexId);
}

void processIfSampleCarrierDataIsReady(void)
{
    //if (isSampleCarrierDataReady())
    {
        Logger_debug("%s: All sample carrier data is ready. Processing with stored data.", getLoggerPrefix());
        if (mDataReadyCallback)
        {
            Logger_debug("%s: Callback for data ready registered. Processing with stored data...", getLoggerPrefix(), getLoggerPrefix());
            (*mDataReadyCallback)(&mSampleCarrierData);
        }
        else
        {
            Logger_debug("%s: Callback for data ready is not registered. Processing with stored data skipped.");
        }
        //cleanUpDataReceivedVariables();
    }
    //else
    //{
        //Logger_debug("%s: All sample carrier data is not ready yet.", getLoggerPrefix());
    //}
}
/*
bool isSampleCarrierDataReady(void)
{
    for (u8 iter = 0; THERMOCOUPLES_COUNT > iter; ++iter)
    {
        if (!mThermocouplesDataReceived[iter])
        {
            return false;
        }
    }
    
    if (!mRTDDataReceived)
    {
        return false;
    }
    
    return true;
}*/

void storeReceivedThermocoupleData(EUnitId thermocouple, double data)
{
    /*
    switch (thermocouple)
    {
        case EUnitId_ThermocoupleReference :
        {
            mSampleCarrierData.refThermocoupleValue = data;
            mThermocouplesDataReceived[0] = true;
            break;
        }
        
        case EUnitId_Thermocouple1 :
        {
            mSampleCarrierData.thermocouple1Value = data;
            mThermocouplesDataReceived[1] = true;
            break;
        }
        
        case EUnitId_Thermocouple2 :
        {
            mSampleCarrierData.thermocouple2Value = data;
            mThermocouplesDataReceived[2] = true;
            break;
        }
        
        case EUnitId_Thermocouple3 :
        {
            mSampleCarrierData.thermocouple3Value = data;
            mThermocouplesDataReceived[3] = true;
            break;
        }
        
        case EUnitId_Thermocouple4 :
        {
            mSampleCarrierData.thermocouple4Value = data;
            mThermocouplesDataReceived[4] = true;
            break;
        }
        
        default :
            break;
    }*/
    mSampleCarrierData.unitId = thermocouple;
    mSampleCarrierData.value = data;
}

void storeReceivedRTDData(double data)
{
    //mSampleCarrierData.rtdTemperatureValue = data;
    mSampleCarrierData.unitId = EUnitId_Rtd1Pt100;
    mSampleCarrierData.value = data;
    mRTDDataReceived = true;
}

void copySampleCarrierData(SSampleCarrierData* source, SSampleCarrierData* destination)
{
    /*
    destination->rtdTemperatureValue = source->rtdTemperatureValue;
    destination->thermocouple1Value = source->thermocouple1Value;
    destination->thermocouple2Value = source->thermocouple2Value;
    destination->thermocouple3Value = source->thermocouple3Value;
    destination->thermocouple4Value = source->thermocouple4Value;
    destination->refThermocoupleValue = source->refThermocoupleValue;*/
    destination->unitId = source->unitId;
    destination->value = source->value;
}

void cleanUpDataReceivedVariables(void)
{
    mRTDDataReceived = false;
    for (u8 iter = 0; THERMOCOUPLES_COUNT > iter; ++iter)
    {
        mThermocouplesDataReceived[iter] = false;
    }
}

double convertRTDResistanceToTemperature(double resistance)
{
    /*double T = mRTDPolynomialCoefficients.values[0];
    for (u8 iter = 1; RTD_POLYNOMIAL_DEGREE >= iter; ++iter)
    {
        T += mRTDPolynomialCoefficients.values[iter] * resistance;
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
double convertThermocoupleVoltageValueToMicrovolts(double valueInVolts)
{
    return (valueInVolts * 10E6);
}
