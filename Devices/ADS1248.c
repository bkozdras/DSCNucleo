#include "Devices/ADS1248.h"
#include "Devices/ADS1248Registers.h"
#include "Devices/ADS1248Types.h"

#include "Peripherals/EXTI.h"
#include "Peripherals/SPI3.h"

#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"
#include "Utilities/Comparator.h"

#include "Defines/CommonDefines.h"
#include "Defines/CommonMacros.h"
#include "Defines/GPIOMacros.h"
#include "HardwareSettings/GPIODefines.h"
#include "SharedDefines/EUnitId.h"

#include "FaultManagement/FaultIndication.h"

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "string.h"

THREAD_DEFINES(ADS1248Controller, ADS1248)
EVENT_HANDLER_PROTOTYPE(CallibrationDoneInd)
EVENT_HANDLER_PROTOTYPE(DeviceDataReadyInd)

#define USED_CHANNELS_COUNT 5

#define MASK(byte, mask)    ( byte &= mask )
#define SET(byte, data)     ( byte |= data )

typedef struct _SChannelData
{
    EUnitId thermocouple;
    double voltage;
    u8 positiveInputNumber;
    u8 negativeInputNumber;
} ChannelData;

static bool mIsInitialized = false;
static EThreadId mObserverThreadId = EThreadId_Unknown;
static double mGainValue = 0.0;
static EADS1248GainValue mGain = EADS1248GainValue_1;
static EADS1248SamplingSpeed mSamplingSpeed = EADS1248SamplingSpeed_5SPS;
static EUnitId mActiveThermocouple;
static EADS1248Mode mActualDeviceMode = EADS1248Mode_Off;
static EADS1248CallibrationType mActiveCallibration = EADS1248CallibrationType_Idle;
static void (*mCallibrationDoneNotifyCallback)(EADS1248CallibrationType, bool) = NULL;
static bool mIsDeviceTurnedOff = false;

static ChannelData mChannelsData [USED_CHANNELS_COUNT] =
    {
        { EUnitId_ThermocoupleReference, 0.0, 5, 0 },
        { EUnitId_Thermocouple1, 0.0, 0, 3 },
        { EUnitId_Thermocouple2, 0.0, 0, 2 },
        { EUnitId_Thermocouple3, 0.0, 0, 7 },
        { EUnitId_Thermocouple4, 0.0, 0, 6 }
    };

static void initializeGpio(void);
static bool configureRegisters(void);
static bool turnOnDevice(void);
static bool turnOffDevice(void);
static void callibrationDoneCallback(void);
static void dataReadyCallback(void);
//static bool readAdcData(u32* adcData);
static bool readAdcDataAndChangeActiveThermocouple(u32* adcData);
static EUnitId getNextThermocouple(void);
static TByte getMux0RegisterContentForThermocoupleMeasurement(EUnitId thermocouple);
static TByte getSys0RegisterContent(EADS1248GainValue gainValue, EADS1248SamplingSpeed samplingSpeed, double* gain);
static TByte getMux1RegisterContent(EADS1248CallibrationType callibrationType);
static TByte getCallibrationCommmand(EADS1248CallibrationType callibrationType);
static ChannelData* getChannelData(EUnitId thermocouple);
static double convertAdcDataToVoltage(u32 adcData);
static void storeReadData(EUnitId thermocouple, double adcData);
static void notifyObserverAboutNewADCValue(EUnitId thermocouple);
static double getStoredThermocoupleVoltageValue(EUnitId thermocouple);

static bool startCallibration(EADS1248CallibrationType callibrationType, void (*callibrationDoneNotifyCallback)(EADS1248CallibrationType, bool));
static bool startReading(void);
static bool stopReading(void);

static bool sendCommand(TByte command);
static bool writeDataToRegister(TByte registerAddress, TByte data);
static bool readDataFromRegister(TByte registerAddress, TByte* data);
static bool writeDataToRegisterAndValidateIt(TByte registerAddress, TByte data);

static void assertCSB(void);
static void deassertCSB(void);
static void assertSTART(void);
static void deassertSTART(void);
static void assertRESET(void);
static void deassertRESET(void);

THREAD(ADS1248Controller)
{
    THREAD_SKELETON_START
    
        EVENT_HANDLING(CallibrationDoneInd)
        EVENT_HANDLING(DeviceDataReadyInd)
    
    THREAD_SKELETON_END
}

EVENT_HANDLER(CallibrationDoneInd)
{
    if (EADS1248CallibrationType_Idle != mActiveCallibration)
    {
        Logger_debug("%s: Callibration %s done.", getLoggerPrefix(), CStringConverter_EADS1248CallibrationType(mActiveCallibration));
        if (mCallibrationDoneNotifyCallback)
        {
            (*mCallibrationDoneNotifyCallback)(mActiveCallibration, true);
        }
        EXTI_unsetCallback(GET_GPIO_PIN(ADS1248_DRDY_PIN));
        mActiveCallibration = EADS1248CallibrationType_Idle;
        mCallibrationDoneNotifyCallback = NULL;
        
        if (!writeDataToRegisterAndValidateIt(ADS1248_REGISTER_MUX1, getMux1RegisterContent(mActiveCallibration)))
        {
            Logger_error("%s: Failure in setting measurment type to Normal Operation in MUX1 register!", getLoggerPrefix());
        }
    }
    else
    {
        Logger_warning("%s: Callibration is not ongoing. DRDY signal from device skipped.", getLoggerPrefix());
    }
}

EVENT_HANDLER(DeviceDataReadyInd)
{
    static u8 wrongDataCounter = 0;
    
    Logger_debug("%s: Reading ADC Data (Active thermocouple: %s).", getLoggerPrefix(), CStringConverter_EUnitId(mActiveThermocouple));
    EUnitId readThermocouple = mActiveThermocouple;
    u32 readData;
    bool status = readAdcDataAndChangeActiveThermocouple(&readData);
    
    if (!status)
    {
        Logger_debug("%s: Received wrong data from device. New data skipped.", getLoggerPrefix());
        ++wrongDataCounter;
        
        if (5 == wrongDataCounter)
        {
            Logger_error("%s: Wrong data counter from device happened 5 times in a row! Permanent error...", getLoggerPrefix());
            FaultIndication_start(EFaultId_WrongData, EUnitId_Nucleo, EUnitId_ADS1248);
        }
    }
    else
    {
        wrongDataCounter = 0;
        
        Logger_debug("%s: Read ADC value: %u.", getLoggerPrefix(), readData);
        double voltageValue = convertAdcDataToVoltage(readData);
        Logger_debug("%s: Actual %s value: %.5f V.", getLoggerPrefix(), CStringConverter_EUnitId(readThermocouple), voltageValue);
        storeReadData(readThermocouple, voltageValue);
        notifyObserverAboutNewADCValue(readThermocouple);
    }
}

void ADS1248_setup(void)
{
    THREAD_INITIALIZE_MUTEX
    initializeGpio();
}

void ADS1248_initialize(void)
{
    Logger_debug("%s: Device initialization.", getLoggerPrefix());
    
    if (!SPI3_isInitialized())
    {
        SPI3_initialize();
    }
    
    EXTI_setCallback(GET_GPIO_PIN(ADS1248_DRDY_PIN), dataReadyCallback);
    assertRESET();
    deassertSTART();
    configureRegisters();
    turnOffDevice();
    mIsInitialized = true;
    
    osMutexRelease(mMutexId);
    
    Logger_info("%s: Device initialized!", getLoggerPrefix());
}

bool ADS1248_isInitialized(void)
{
    bool isInitialized;
    osMutexWait(mMutexId, osWaitForever);
    isInitialized = mIsInitialized;
    osMutexRelease(mMutexId);
    return isInitialized;
}

bool ADS1248_changeMode(EADS1248Mode newMode)
{
    osMutexWait(mMutexId, osWaitForever);
    
    Logger_info("%s: Setting new device mode: %s.", getLoggerPrefix(), CStringConverter_EADS1248Mode(newMode));
    
    bool status = false;
    
    if (mActualDeviceMode != newMode)
    {
        if ( EADS1248Mode_On == newMode )
        {
            status = configureRegisters();
        }
        
        if ( EADS1248Mode_Off == newMode )
        {
            status = turnOffDevice();
        }
        
        if (status)
        {
            mActualDeviceMode = newMode;
            Logger_info("%s: Device new mode: %s.", getLoggerPrefix(), CStringConverter_EADS1248Mode(newMode));
        }
        else
        {
            Logger_error("%s: Device mode changing to %s failed!", getLoggerPrefix(), CStringConverter_EADS1248Mode(newMode));
        }
    }
    else
    {
        status = false;
        Logger_warning("%s: Operation skipped. New mode is the same as actual!", getLoggerPrefix());
    }
    
    osMutexRelease(mMutexId);
    
    return status;
}

bool ADS1248_startCallibration(EADS1248CallibrationType callibrationType, void (*callibrationDoneNotifyCallback)(EADS1248CallibrationType, bool))
{
    osMutexWait(mMutexId, osWaitForever);
    bool result = startCallibration(callibrationType, callibrationDoneNotifyCallback);
    osMutexRelease(mMutexId);
    return result;
}

bool ADS1248_startReading(void)
{
    osMutexWait(mMutexId, osWaitForever);
    bool result = startReading();
    osMutexRelease(mMutexId);
    return result;
}

bool ADS1248_stopReading(void)
{
    osMutexWait(mMutexId, osWaitForever);
    bool result = stopReading();
    osMutexRelease(mMutexId);
    return result;
}

bool ADS1248_getThermocoupleVoltageValue(EUnitId thermocouple, double* value)
{
    osMutexWait(mMutexId, osWaitForever);
    *value = getStoredThermocoupleVoltageValue(thermocouple);
    osMutexRelease(mMutexId);
    return true;
}

bool ADS1248_setChannelGain(EADS1248GainValue gainValue)
{
    osMutexWait(mMutexId, osWaitForever);
    
    Logger_debug("%s: Setting new channels PGA / gain value: %s.", getLoggerPrefix(), CStringConverter_EADS1248GainValue(gainValue));
    
    double gain;
    TByte sys0RegisterContent = getSys0RegisterContent(gainValue, mSamplingSpeed, &gain);
    bool result = writeDataToRegisterAndValidateIt(ADS1248_REGISTER_SYS0, sys0RegisterContent);
    
    if (result)
    {
        Logger_debug("%s: Set new channels PGA / gain value: %s with success.", getLoggerPrefix(), CStringConverter_EADS1248GainValue(gainValue));
        mGainValue = gain;
        mGain = gainValue;
    }
    else
    {
        Logger_error("%s: Setting new channels PGA / gain value: %s failed.", getLoggerPrefix(), CStringConverter_EADS1248GainValue(gainValue));
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

bool ADS1248_setChannelSamplingSpeed(EADS1248SamplingSpeed samplingSpeed)
{
    osMutexWait(mMutexId, osWaitForever);
    
    Logger_debug("%s: Setting new channels sampling speed: %s.", getLoggerPrefix(), CStringConverter_EADS1248SamplingSpeed(samplingSpeed));
    
    double dummy;
    TByte sys0RegisterContent = getSys0RegisterContent(mGain, samplingSpeed, &dummy);
    bool result = writeDataToRegisterAndValidateIt(ADS1248_REGISTER_SYS0, sys0RegisterContent);
    
    if (result)
    {
        Logger_debug("%s: Set new channels sampling speed: %s with success.", getLoggerPrefix(), CStringConverter_EADS1248SamplingSpeed(samplingSpeed));
        mSamplingSpeed = samplingSpeed;
    }
    else
    {
        Logger_error("%s: Setting new channels sampling speed: %s failed.", getLoggerPrefix(), CStringConverter_EADS1248SamplingSpeed(samplingSpeed));
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

void ADS1248_registerNewValueObserver(EThreadId threadId)
{
    osMutexWait(mMutexId, osWaitForever);
    mObserverThreadId = threadId;
    Logger_debug("%s: ADC data new value observer (%s) registered.", getLoggerPrefix(), CStringConverter_EThreadId(mObserverThreadId));
    osMutexRelease(mMutexId);
}

void ADS1248_deregisterNewValueObserver(void)
{
    osMutexWait(mMutexId, osWaitForever);
    Logger_debug("%s: ADC data new value observer (%s) deregistered.", getLoggerPrefix(), CStringConverter_EThreadId(mObserverThreadId));
    mObserverThreadId = EThreadId_Unknown;
    osMutexRelease(mMutexId);
}

void initializeGpio(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // CSB
    
    GPIO_CLOCK_ENABLE(ADS1248_CSB_PORT);
    
    GPIO_InitStruct.Pin = GET_GPIO_PIN(ADS1248_CSB_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GET_GPIO_PORT(ADS1248_CSB_PORT), &GPIO_InitStruct);
    
    deassertCSB();
    
    // START
    
    GPIO_CLOCK_ENABLE(ADS1248_START_PORT);
    
    GPIO_InitStruct.Pin = GET_GPIO_PIN(ADS1248_START_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GET_GPIO_PORT(ADS1248_START_PORT), &GPIO_InitStruct);
    
    // RESET
    
    GPIO_CLOCK_ENABLE(ADS1248_RESET_PORT);
    
    GPIO_InitStruct.Pin = GET_GPIO_PIN(ADS1248_RESET_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GET_GPIO_PORT(ADS1248_RESET_PORT), &GPIO_InitStruct);
    
    // DRDYB
    
    GPIO_CLOCK_ENABLE(ADS1248_DRDY_PORT);

    GPIO_InitStruct.Pin = GET_GPIO_PIN(ADS1248_DRDY_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    //GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GET_GPIO_PORT(ADS1248_DRDY_PORT), &GPIO_InitStruct);
}

bool configureRegisters(void)
{
    deassertSTART();
    
    TByte writeData [13];
    TByte registerAddresses [13];
    u8 counter = 0;
    
    // Register MUX0
    
    MASK(writeData[counter], ADS1248_REGISTER_MUX0_BCS_MASK);
    SET(writeData[counter], ADS1248_REGISTER_MUX0_BCS_OFF);
    MASK(writeData[counter], ADS1248_REGISTER_MUX0_MUX_SP_MASK);
    SET(writeData[counter], ADS1248_REGISTER_MUX0_MUX_SP_AIN5);
    MASK(writeData[counter], ADS1248_REGISTER_MUX0_MUX_SN_MASK);
    SET(writeData[counter], ADS1248_REGISTER_MUX0_MUX_SN_AIN0);
    
    registerAddresses[counter++] = ADS1248_REGISTER_MUX0;
    
    // Register VBIAS

    MASK(writeData[counter], ADS1248_REGISTER_VBIAS_VBIAS_MASK);
    
    registerAddresses[counter++] = ADS1248_REGISTER_VBIAS;
    
    // Register MUX1
    
    MASK(writeData[counter], ADS1248_REGISTER_MUX1_CLKSTAT_MASK);
    SET(writeData[counter], ADS1248_REGISTER_MUX1_CLKSTAT_INTERNAL_OSCILLATOR);
    MASK(writeData[counter], ADS1248_REGISTER_MUX1_VREFCON_MASK);
    SET(writeData[counter], ADS1248_REGISTER_MUX1_VREFCON_INTERNAL_REFERENCE_ON_CONVERSION_IN_PROGRESS);
    MASK(writeData[counter], ADS1248_REGISTER_MUX1_REFSELT_MASK);
    SET(writeData[counter], ADS1248_REGISTER_MUX1_REFSELT_ONBOARD_REFERENCE);
    MASK(writeData[counter], ADS1248_REGISTER_MUX1_MUXCAL_MASK);
    SET(writeData[counter], ADS1248_REGISTER_MUX1_MUXCAL_NORMAL_OPERATION);
    
    registerAddresses[counter++] = ADS1248_REGISTER_MUX1;
    
    // Register SYS0
    
    MASK(writeData[counter], ADS1248_REGISTER_SYS0_MASK);
    MASK(writeData[counter], ADS1248_REGISTER_SYS0_PGA_MASK);
    SET(writeData[counter], ADS1248_REGISTER_SYS0_PGA_1);
    MASK(writeData[counter], ADS1248_REGISTER_SYS0_DOR_MASK);
    SET(writeData[counter], ADS1248_REGISTER_SYS0_DOR_5_SPS);
    
    registerAddresses[counter++] = ADS1248_REGISTER_SYS0;
    
    // Registers OFC0 - OFC2
    
    MASK(writeData[counter], ADS1248_REGISTER_OFC0_MASK);
    registerAddresses[counter++] = ADS1248_REGISTER_OFC0;
    
    MASK(writeData[counter], ADS1248_REGISTER_OFC1_MASK);
    registerAddresses[counter++] = ADS1248_REGISTER_OFC1;
    
    MASK(writeData[counter], ADS1248_REGISTER_OFC2_MASK);
    registerAddresses[counter++] = ADS1248_REGISTER_OFC2;
    
    // Registers FSC0 - FSC2
    
    MASK(writeData[counter], ADS1248_REGISTER_FSC0_MASK);
    registerAddresses[counter++] = ADS1248_REGISTER_FSC0;
    
    MASK(writeData[counter], ADS1248_REGISTER_FSC1_MASK);
    registerAddresses[counter++] = ADS1248_REGISTER_FSC1;
    
    MASK(writeData[counter], ADS1248_REGISTER_FSC2_MASK);
    registerAddresses[counter++] = ADS1248_REGISTER_FSC2;
    
    // Register IDAC0
    
    MASK(writeData[counter], ADS1248_REGISTER_IDAC0_DRDY_MODE_MASK);
    SET(writeData[counter], ADS1248_REGISTER_IDAC0_DRDY_MODE_ONLY_DATA_OUT);
    MASK(writeData[counter], ADS1248_REGISTER_IDAC0_IMAG_MASK);
    SET(writeData[counter], ADS1248_REGISTER_IDAC0_IMAG_OFF);
    
    registerAddresses[counter++] = ADS1248_REGISTER_IDAC0;
    
    // Register IDAC1
    
    MASK(writeData[counter], ADS1248_REGISTER_IDAC1_I1DIR_MASK);
    SET(writeData[counter], ADS1248_REGISTER_IDAC1_I1DIR_DISCONNECTED);
    MASK(writeData[counter], ADS1248_REGISTER_IDAC1_I2DIR_MASK);
    SET(writeData[counter], ADS1248_REGISTER_IDAC1_I2DIR_DISCONNECTED);
    
    registerAddresses[counter++] = ADS1248_REGISTER_IDAC1;
    
    // Register GPIOCFG
    
    MASK(writeData[counter], ADS1248_REGISTER_GPIOCFG_MASK);
    registerAddresses[counter++] = ADS1248_REGISTER_GPIOCFG;
    
    for (u8 iter = 0; counter > iter; ++iter)
    {
        if (!writeDataToRegisterAndValidateIt(registerAddresses[iter], writeData[iter]))
        {
            Logger_error("%s: Configuring registers: Failure!", getLoggerPrefix());
            return false;
        }
    }
    mGainValue = 1.0F;
    mGain = EADS1248GainValue_1;
    mSamplingSpeed = EADS1248SamplingSpeed_5SPS;
    Logger_debug("%s: Configuring registers: DONE!", getLoggerPrefix());
    return true;
}

bool turnOnDevice(void)
{
    deassertRESET();
    if (sendCommand(ADS1248_COMMAND_WAKEUP))
    {
        mIsDeviceTurnedOff = false;
        Logger_debug("%s: Device is turned on!", getLoggerPrefix());
        return true;
    }
    else
    {
        Logger_error("%s: Device turning on failed!", getLoggerPrefix());
        return false;
    }
}

bool turnOffDevice(void)
{
    stopReading();
    if (sendCommand(ADS1248_COMMAND_SLEEP))
    {
        mIsDeviceTurnedOff = true;
        Logger_debug("%s: Device is turned off!", getLoggerPrefix());
        return true;
    }
    else
    {
        Logger_error("%s: Device turning off failed!", getLoggerPrefix());
        return false;
    }
}

void callibrationDoneCallback(void)
{
    CREATE_EVENT_ISR(CallibrationDoneInd, mThreadId);
    SEND_EVENT();
}

void dataReadyCallback(void)
{
    CREATE_EVENT_ISR(DeviceDataReadyInd, mThreadId);
    SEND_EVENT();
}
/*
bool readAdcData(u32* adcData)
{
    TByte writeData;
    TByte readData [3];
    
    writeData = ADS1248_COMMAND_RDATA;
    
    SPI3_block();
    assertCSB();
    
    bool result = SPI3_transmit(&writeData, 1, 100);
    if (result)
    {
        result = SPI3_receive(readData, 3, 100);
    }
    
    deassertCSB();
    SPI3_unblock();
    
    if (result)
    {
        u32 newValue = 0;
        
        newValue |= (((u32)(readData[2])) & 0xFF);
        newValue |= ((((u32) (readData[1]) ) << 8) & 0xFF00);
        newValue |= ((((u32) (readData[0]) ) << 16) & 0xFF0000);
        
        *adcData = newValue;
    }
    else
    {
        Logger_error("%s: Reading data failed (channel: %s).", getLoggerPrefix(), CStringConverter_EUnitId(mActiveThermocouple));
        FaultIndication_start(EFaultId_Communication, EUnitId_Nucleo, EUnitId_ADS1248);
    }
    
    return result;
}
*/
bool readAdcDataAndChangeActiveThermocouple(u32* adcData)
{
    EUnitId nextThermocouple = getNextThermocouple();
    
    TByte writeData [3];
    TByte readData [3];
    
    writeData[0] = ADS1248_COMMAND_WREG;
    writeData[0] |= (ADS1248_REGISTER_MUX0 & ADS1248_COMMAND_WREG_REG_ADDRESS_MASK);
    writeData[1] = 0x01;
    writeData[2] = getMux0RegisterContentForThermocoupleMeasurement(nextThermocouple);
    
    SPI3_block();
    assertCSB();
    bool result = SPI3_transmitReceive(writeData, readData, 3, 100);
    deassertCSB();
    SPI3_unblock();
    
    if (!result)
    {
        Logger_error("%s: Reading ADC data failed for %s.", getLoggerPrefix(), CStringConverter_EUnitId(mActiveThermocouple));
        return false;
    }
    
    mActiveThermocouple = nextThermocouple;
    
    u32 newValue = 0;
    
    newValue |= (((u32)(readData[2])) & 0xFF);
    newValue |= ((((u32) (readData[1]) ) << 8) & 0xFF00);
    newValue |= ((((u32) (readData[0]) ) << 16) & 0xFF0000);
    
    *adcData = newValue;
    
    return true;
}
EUnitId getNextThermocouple(void)
{
    typedef struct _ThermocouplesPair
    {
        EUnitId first;
        EUnitId second;
    } ThermocouplesPair;

    static ThermocouplesPair thermocouplesPairs [USED_CHANNELS_COUNT] =
        {
            { EUnitId_ThermocoupleReference, EUnitId_Thermocouple1 },
            { EUnitId_Thermocouple1, EUnitId_Thermocouple2 },
            { EUnitId_Thermocouple2, EUnitId_Thermocouple3 },
            { EUnitId_Thermocouple3, EUnitId_Thermocouple4 },
            { EUnitId_Thermocouple4, EUnitId_Thermocouple1 }
        };
        
    for (u8 iter = 0; USED_CHANNELS_COUNT > iter; ++iter)
    {
        if (mActiveThermocouple == thermocouplesPairs[iter].first)
        {
            return thermocouplesPairs[iter].second;
        }
    }
    
    return EUnitId_Unknown;
}

TByte getMux0RegisterContentForThermocoupleMeasurement(EUnitId thermocouple)
{
    TByte mux0Register = 0x00;
    MASK(mux0Register, ADS1248_REGISTER_MUX0_BCS_MASK);
    SET(mux0Register, ADS1248_REGISTER_MUX0_BCS_OFF);
    MASK(mux0Register, ADS1248_REGISTER_MUX0_MUX_SP_MASK);
    MASK(mux0Register, ADS1248_REGISTER_MUX0_MUX_SN_MASK);
    
    ChannelData* channelData = getChannelData(thermocouple);
    
    switch (channelData->positiveInputNumber)
    {
        case 0 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SP_AIN0);
            break;
        }
        
        case 1 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SP_AIN1);
            break;
        }
        
        case 2 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SP_AIN2);
            break;
        }
        
        case 3 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SP_AIN3);
            break;
        }
        
        case 4 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SP_AIN4);
            break;
        }
        
        case 5 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SP_AIN5);
            break;
        }
        
        case 6 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SP_AIN6);
            break;
        }
        
        case 7 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SP_AIN7);
            break;
        }
        
        default :
            break;
    }
    
    switch (channelData->negativeInputNumber)
    {
        case 0 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SN_AIN0);
            break;
        }
        
        case 1 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SN_AIN1);
            break;
        }
        
        case 2 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SN_AIN2);
            break;
        }
        
        case 3 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SN_AIN3);
            break;
        }
        
        case 4 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SN_AIN4);
            break;
        }
        
        case 5 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SN_AIN5);
            break;
        }
        
        case 6 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SN_AIN6);
            break;
        }
        
        case 7 :
        {
            SET(mux0Register, ADS1248_REGISTER_MUX0_MUX_SN_AIN7);
            break;
        }
        
        default :
            break;
    }
    
    return mux0Register;
}

TByte getSys0RegisterContent(EADS1248GainValue gainValue, EADS1248SamplingSpeed samplingSpeed, double* gain)
{
    TByte sys0Register = 0x00;
    MASK(sys0Register, ADS1248_REGISTER_SYS0_PGA_MASK);
    MASK(sys0Register, ADS1248_REGISTER_SYS0_DOR_MASK);
    
    switch (gainValue)
    {
        case EADS1248GainValue_1 :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_PGA_1);
            *gain = 1.0F;
            break;
        }
        
        case EADS1248GainValue_2 :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_PGA_2);
            *gain = 2.0F;
            break;
        }
                
        case EADS1248GainValue_4 :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_PGA_4);
            *gain = 4.0F;
            break;
        }
                        
        case EADS1248GainValue_8 :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_PGA_8);
            *gain = 8.0F;
            break;
        }
                                
        case EADS1248GainValue_16 :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_PGA_16);
            *gain = 16.0F;
            break;
        }
                                        
        case EADS1248GainValue_32 :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_PGA_32);
            *gain = 32.0F;
            break;
        }
                                                
        case EADS1248GainValue_64 :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_PGA_64);
            *gain = 64.0F;
            break;
        }
        
        case EADS1248GainValue_128 :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_PGA_128);
            *gain = 128.0F;
            break;
        }
        
        default :
            break;
    }
    
    switch (samplingSpeed)
    {
        case EADS1248SamplingSpeed_5SPS :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_DOR_5_SPS);
            break;
        }
        
        case EADS1248SamplingSpeed_10SPS :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_DOR_10_SPS);
            break;
        }
                
        case EADS1248SamplingSpeed_20SPS :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_DOR_20_SPS);
            break;
        }
                        
        case EADS1248SamplingSpeed_40SPS :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_DOR_40_SPS);
            break;
        }
                                
        case EADS1248SamplingSpeed_80SPS :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_DOR_80_SPS);
            break;
        }
                                        
        case EADS1248SamplingSpeed_160SPS :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_DOR_160_SPS);
            break;
        }
                                                
        case EADS1248SamplingSpeed_320SPS :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_DOR_320_SPS);
            break;
        }
                                                        
        case EADS1248SamplingSpeed_640SPS :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_DOR_640_SPS);
            break;
        }
                                                                
        case EADS1248SamplingSpeed_1000SPS :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_DOR_1000_SPS);
            break;
        }
        
        case EADS1248SamplingSpeed_2000SPS :
        {
            SET(sys0Register, ADS1248_REGISTER_SYS0_DOR_2000_SPS);
            break;
        }
        
        default :
            break;
    }
    
    return sys0Register;
}

TByte getMux1RegisterContent(EADS1248CallibrationType callibrationType)
{
    TByte mux1Register = 0x00;
    
    MASK(mux1Register, ADS1248_REGISTER_MUX1_CLKSTAT_MASK);
    SET(mux1Register, ADS1248_REGISTER_MUX1_CLKSTAT_INTERNAL_OSCILLATOR);
    MASK(mux1Register, ADS1248_REGISTER_MUX1_VREFCON_MASK);
    SET(mux1Register, ADS1248_REGISTER_MUX1_VREFCON_INTERNAL_REFERENCE_ON_CONVERSION_IN_PROGRESS);
    MASK(mux1Register, ADS1248_REGISTER_MUX1_REFSELT_MASK);
    SET(mux1Register, ADS1248_REGISTER_MUX1_REFSELT_ONBOARD_REFERENCE);
    MASK(mux1Register, ADS1248_REGISTER_MUX1_MUXCAL_MASK);
    
    switch(callibrationType)
    {
        case EADS1248CallibrationType_SelfOffset :
        {
            SET(mux1Register, ADS1248_REGISTER_MUX1_MUXCAL_OFFSET_MEASUREMENT);
            break;
        }
        
        case EADS1248CallibrationType_SystemGain :
        {
            SET(mux1Register, ADS1248_REGISTER_MUX1_MUXCAL_GAIN_MEASUREMENT);
            break;
        }
        
        default :
        {
            SET(mux1Register, ADS1248_REGISTER_MUX1_MUXCAL_NORMAL_OPERATION);
            break;
        }
    }
    
    return mux1Register;
}

TByte getCallibrationCommmand(EADS1248CallibrationType callibrationType)
{
    switch (callibrationType)
    {
        case EADS1248CallibrationType_OffsetSystem :
            return ADS1248_COMMAND_SYSOCAL;
        
        case EADS1248CallibrationType_SystemGain :
            return ADS1248_COMMAND_SYSGCAL;
        
        case EADS1248CallibrationType_SelfOffset :
            return ADS1248_COMMAND_SELFOCAL;
        
        default :
        {
            assert_param(0 && "Forbidden callibration type in switch statement!");
            return 0;
        }
    }
}

ChannelData* getChannelData(EUnitId thermocouple)
{
    for (u8 iter = 0; USED_CHANNELS_COUNT > iter; ++iter)
    {
        if (mChannelsData[iter].thermocouple == thermocouple)
        {
            return &(mChannelsData[iter]);
        }
    }
    
    return NULL;
}

double convertAdcDataToVoltage(u32 adcData)
{
    double doubleAdcData = (double) adcData;
    return ( ( ( (doubleAdcData * 2.048) ) / mGainValue) / 8388607.0 );
}

void storeReadData(EUnitId thermocouple, double adcData)
{
    for (u8 iter = 0; USED_CHANNELS_COUNT > iter; ++iter)
    {
        if (mChannelsData[iter].thermocouple == thermocouple)
        {
            mChannelsData[iter].voltage = adcData;
            return;
        }
    }
}

void notifyObserverAboutNewADCValue(EUnitId thermocouple)
{
    if (EThreadId_Unknown != mObserverThreadId)
    {
        CREATE_EVENT(NewThermocoupleVoltageValueInd, mObserverThreadId);
        CREATE_EVENT_MESSAGE(NewThermocoupleVoltageValueInd);
        
        eventMessage->thermocouple = thermocouple;
        eventMessage->value = getStoredThermocoupleVoltageValue(thermocouple);
        
        Logger_debug
        (
            "%s: Sending notification %s to %s about new %s voltage value: %.6f V.",
            getLoggerPrefix(),
            CStringConverter_EEventId(event->id),
            CStringConverter_EThreadId(mObserverThreadId),
            CStringConverter_EUnitId(eventMessage->thermocouple),
            eventMessage->value
        );
        
        SEND_EVENT();
    }
}

bool startCallibration(EADS1248CallibrationType callibrationType, void (*callibrationDoneNotifyCallback)(EADS1248CallibrationType, bool))
{
    if (EADS1248CallibrationType_Idle == callibrationType)
    {
        Logger_warning("%s: Wrong callibration type to start!", getLoggerPrefix());
        return false;
    }
    else if (EADS1248CallibrationType_Idle == mActiveCallibration)
    {
        stopReading();
        if (mIsDeviceTurnedOff)
        {
            if (!turnOnDevice())
            {
                Logger_error("%s: Callibration %s not started. Device turning on failed!", getLoggerPrefix(), CStringConverter_EADS1248CallibrationType(mActiveCallibration));
            }
        }
        EXTI_setCallback(GET_GPIO_PIN(ADS1248_DRDY_PIN), callibrationDoneCallback);
        mActiveCallibration = callibrationType;
        mCallibrationDoneNotifyCallback = callibrationDoneNotifyCallback;
        TByte mux1RegisterContent = getMux1RegisterContent(callibrationType);
        if (writeDataToRegisterAndValidateIt(ADS1248_REGISTER_MUX1, mux1RegisterContent) && sendCommand(getCallibrationCommmand(callibrationType)))
        {
            Logger_debug("%s: Callibration %s started.", getLoggerPrefix(), CStringConverter_EADS1248CallibrationType(mActiveCallibration));
        }
        else
        {
            Logger_error("%s: Callibration %s not started. Communication failed!", getLoggerPrefix(), CStringConverter_EADS1248CallibrationType(mActiveCallibration));
            return false;
        }
    }
    else
    {
        Logger_warning
        (
            "%s: Callibration %s in ongoing. Demanded callibration %s skipped.",
            getLoggerPrefix(),
            CStringConverter_EADS1248CallibrationType(mActiveCallibration),
            CStringConverter_EADS1248CallibrationType(callibrationType)
        );
        return false;
    }
    
    return false;
}

bool startReading(void)
{
    mActiveThermocouple = EUnitId_Thermocouple1;
    TByte mux0RegisterContent = getMux0RegisterContentForThermocoupleMeasurement(EUnitId_ThermocoupleReference);
    
    if (mIsDeviceTurnedOff)
    {
        turnOnDevice();
    }
    
    bool result = writeDataToRegisterAndValidateIt(ADS1248_REGISTER_MUX0, mux0RegisterContent);
    if (result)
    {
        result = sendCommand(ADS1248_COMMAND_RDATAC);
    }
    
    if (result)
    {
        EXTI_setCallback(GET_GPIO_PIN(ADS1248_DRDY_PIN), dataReadyCallback);
        assertSTART();
        Logger_debug("%s: Starting reading ADC data from thermocouples.", getLoggerPrefix());
    }
    else
    {
        Logger_error("%s: Starting reading failed!", getLoggerPrefix());
    }
    
    return result;
}

bool stopReading(void)
{
    deassertSTART();
    EXTI_unsetCallback(GET_GPIO_PIN(ADS1248_DRDY_PIN));
    bool result = sendCommand(ADS1248_COMMAND_SDATAC);
    if (result)
    {
        result = turnOffDevice();
        if (result)
        {
            Logger_debug("%s: Data reading stopped.", getLoggerPrefix());
            return true;
        }
        else
        {
            Logger_error("%s: Data reading stopped but device is not turning off. Operation failed", getLoggerPrefix());
            return false;
        }
    }
    else
    {
        Logger_error("%s: Failure in stopping data reading operation. Operation failed", getLoggerPrefix());
        return false;
    }
}

double getStoredThermocoupleVoltageValue(EUnitId thermocouple)
{
    for (u8 iter = 0; USED_CHANNELS_COUNT > iter; ++iter)
    {
        if (mChannelsData[iter].thermocouple == thermocouple)
        {
            return mChannelsData[iter].voltage;
        }
    }
    
    return 0.0F;
}

bool sendCommand(TByte command)
{
    SPI3_block();
    assertCSB();
    
    bool result = SPI3_transmit(&command, 1, 100);
    
    deassertCSB();
    SPI3_unblock();
    
    if (!result)
    {
        Logger_error("%s: Command 0x%02X sending to device failed.", getLoggerPrefix(), command);
        FaultIndication_start(EFaultId_Communication, EUnitId_Nucleo, EUnitId_ADS1248);
    }
    
    return result;
}

bool writeDataToRegister(TByte registerAddress, TByte data)
{
    TByte writeData [3];
    
    // 1st Command Byte
    writeData[0] = ADS1248_COMMAND_WREG;
    writeData[0] |= (registerAddress & ADS1248_COMMAND_WREG_REG_ADDRESS_MASK);
    
    // 2nd Command Byte
    writeData[1] = 0x01; // one byte
    
    // Data written to register
    writeData[2] = data;
    
    SPI3_block();
    assertCSB();
    
    bool result = SPI3_transmit(writeData, 3, 100);
        
    deassertCSB();
    SPI3_unblock();
    
    if (result)
    {
        Logger_debug("%s: Byte 0x%02X written to the register 0x%02X.", getLoggerPrefix(), data, registerAddress);
    }
    else
    {
        Logger_error("%s: Byte 0x%02X was not written to the register 0x%02X.", getLoggerPrefix(), data, registerAddress);
        FaultIndication_start(EFaultId_Communication, EUnitId_Nucleo, EUnitId_ADS1248);
    }
    
    return result;
}

bool readDataFromRegister(TByte registerAddress, TByte* data)
{
    TByte writeData [2];
    
    // 1st Command Byte
    writeData[0] = ADS1248_COMMAND_RREG;
    writeData[0] |= (registerAddress & ADS1248_COMMAND_RREG_REG_ADDRESS_MASK);
    
    // 2nd Command Byte
    writeData[1] = 0x01;
    
    SPI3_block();
    assertCSB();
    
    bool result = SPI3_transmit(writeData, 2, 100);
    
    if (result)
    {
        result = SPI3_receive(data, 1, 100);
    }
    
    deassertCSB();
    SPI3_unblock();
    
    if (result)
    {
        Logger_debug("%s: Byte 0x%02X read from the register 0x%02X.", getLoggerPrefix(), *data, registerAddress);
    }
    else
    {
        Logger_error("%s: Reading register 0x%02X content failed.", getLoggerPrefix(), registerAddress);
        *data = 0x00;
        FaultIndication_start(EFaultId_Communication, EUnitId_Nucleo, EUnitId_ADS1248);
    }
    
    return result;
}

bool writeDataToRegisterAndValidateIt(TByte registerAddress, TByte data)
{
    bool result = writeDataToRegister(registerAddress, data);
    if (result)
    {
        TByte dataToCompare;
        result = readDataFromRegister(registerAddress, &dataToCompare);
        if (result)
        {
            if (data != dataToCompare)
            {
                result = false;
                Logger_error
                (
                    "%s: Comparing written and read data from register 0x%02X failed. (0x%02X != 0x%02X)",
                    getLoggerPrefix(),
                    registerAddress,
                    data,
                    dataToCompare
                );
            }
        }
    }
    
    return result;
}

void assertCSB(void)
{
    GPIO_SET_LOW_LEVEL(ADS1248_CSB_PORT, ADS1248_CSB_PIN);
}

void deassertCSB(void)
{
    GPIO_SET_HIGH_LEVEL(ADS1248_CSB_PORT, ADS1248_CSB_PIN);
}

void assertSTART(void)
{
    GPIO_SET_HIGH_LEVEL(ADS1248_START_PORT, ADS1248_START_PIN);
}

void deassertSTART(void)
{
    GPIO_SET_HIGH_LEVEL(ADS1248_RESET_PORT, ADS1248_RESET_PIN);
}

void assertRESET(void)
{
    GPIO_SET_HIGH_LEVEL(ADS1248_RESET_PORT, ADS1248_RESET_PIN);
}

void deassertRESET(void)
{
    GPIO_SET_LOW_LEVEL(ADS1248_RESET_PORT, ADS1248_RESET_PIN);
}

#undef MASK
#undef SET
