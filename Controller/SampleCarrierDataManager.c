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

static SSampleCarrierData mSampleCarrierData =
    {
        {
            { EUnitId_ThermocoupleReference, 0.0 },
            { EUnitId_Thermocouple1, 0.0 },
            { EUnitId_Thermocouple2, 0.0 },
            { EUnitId_Thermocouple3, 0.0 },
            { EUnitId_Thermocouple4, 0.0 }
        },
        0.0F
    };
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
static bool isSampleCarrierDataReady(void);
static void storeReceivedThermocoupleData(EUnitId thermocouple, double data);
static void storeReceivedRTDData(double data);
static void copySampleCarrierData(SSampleCarrierData* source, SSampleCarrierData* destination);
static void cleanUpDataReceivedVariables(void);
static double convertRTDResistanceToTemperature(double resistance);
static double convertThermocoupleVoltageValueToMillivolts(double valueInVolts);

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
    
    double thermocoupleNanoVoltValue = convertThermocoupleVoltageValueToMillivolts(event->value);
    Logger_debug("%s: %s value: %.5f mV. Storing data.", getLoggerPrefix(), CStringConverter_EUnitId(event->thermocouple), thermocoupleNanoVoltValue);
    storeReceivedThermocoupleData(event->thermocouple, thermocoupleNanoVoltValue);
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
    if (isSampleCarrierDataReady())
    {
        Logger_debug("%s: All sample carrier data is ready. Processing with stored data.");
        if (mDataReadyCallback)
        {
            Logger_debug("%s: Callback for data ready registered. Processing with stored data...", getLoggerPrefix());
            (*mDataReadyCallback)(&mSampleCarrierData);
        }
        else
        {
            Logger_debug("%s: Callback for data ready is not registered. Processing with stored data skipped.");
        }
        cleanUpDataReceivedVariables();
    }
    else
    {
        Logger_debug("%s: All sample carrier data is not ready yet.", getLoggerPrefix());
    }
}

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
}

void storeReceivedThermocoupleData(EUnitId thermocouple, double data)
{
    for (u8 iter = 0; THERMOCOUPLES_COUNT > iter; ++iter)
    {
        if (mSampleCarrierData.data[iter].thermocouple == thermocouple)
        {
            mSampleCarrierData.data[iter].milliVoltVoltage = data;
            mThermocouplesDataReceived[iter] = true;
        }
    }
}

void storeReceivedRTDData(double data)
{
    mSampleCarrierData.rtdTemperatureValue = data;
    mRTDDataReceived = true;
}

void copySampleCarrierData(SSampleCarrierData* source, SSampleCarrierData* destination)
{
    destination->rtdTemperatureValue = source->rtdTemperatureValue;
    for (u8 iter = 0; THERMOCOUPLES_COUNT > iter; ++iter)
    {
        destination->data[iter].thermocouple = source->data[iter].thermocouple;
        destination->data[iter].milliVoltVoltage = source->data[iter].milliVoltVoltage;
    }
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
    double T = mRTDPolynomialCoefficients.values[0];
    for (u8 iter = 1; RTD_POLYNOMIAL_DEGREE >= iter; ++iter)
    {
        T += mRTDPolynomialCoefficients.values[iter] * resistance;
        resistance *= resistance;
    }
    return T;
}
double convertThermocoupleVoltageValueToMillivolts(double valueInVolts)
{
    return (valueInVolts * 1000.0);
}
