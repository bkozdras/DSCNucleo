#include "Devices/MCP4716.h"
#include "Devices/MCP4716Registers.h"

#include "Defines/CommonMacros.h"

#include "Peripherals/I2C1.h"

#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"

#include "FaultManagement/FaultIndication.h"

#include "cmsis_os.h"
#include "stdbool.h"

static osMutexDef(mMutex);
static osMutexId mMutexId = NULL;

static u16 mActualValue = 0;
static u8 mGain = 0;

static bool isDeviceReady(u8 maxCheckAttempts, TTimeMs intervalBetweenAttempts);
static bool configureRegisters(void);
static bool validateMainRegister(TByte transmittedRegister, TByte receivedRegister);
static bool checkDeviceStatusBits(TByte controlByte);
static float convertOutputVoltageToRealData(u16 outputVoltage);
static const char* getLoggerPrefix(void);

void MCP4716_setup(void)
{
    if (!mMutexId)
    {
        mMutexId = osMutexCreate(osMutex(mMutex));
    }
}

bool MCP4716_initialize(void)
{
    if (!I2C1_isInitialized())
    {
        I2C1_initialize();
    }
    
    if ( !isDeviceReady(5, 500) )
    {
        Logger_error("%s: Device is not ready. Initialization failed!", getLoggerPrefix());
        return false;
    }
    
    if ( !configureRegisters() )
    {
        Logger_error("%s: Configuring registers failed. Initialization failed!", getLoggerPrefix());
        return false;
    }
    
    Logger_info("%s: Initialized!", getLoggerPrefix());
    return true;
}

bool MCP4716_setOutputVoltage(u16 value)
{
    osMutexWait(mMutexId, osWaitForever);
    
    bool result = true;
    
    if (value != mActualValue)
    {
        TByte data [2] = 
        {
            MCP4716_POWER_DOWN_NORMAL_OPERATION,
            0x00
        };
        
        data[0] |= (TByte) ( ( value >> 6 ) & 0x0F );
        data[1] |= (TByte)( ( value << 2 ) & 0xFC );
        
        if (I2C1_transmit(MCP4716_DEVICE_ADDRESS, data, 2, 100))
        {
            Logger_debug("%s: Output voltage %.2f V (Val. %u).", getLoggerPrefix(), convertOutputVoltageToRealData(value), value);
            mActualValue = value;
        }
        else
        {
            Logger_error("%s: Failure in transmitting I2C data during setting output voltage!", getLoggerPrefix());
            FaultIndication_start(EFaultId_Communication, EUnitId_Nucleo, EUnitId_MCP4716);
            result = false;
        }
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

u16 MCP4716_getOutputVoltage(void)
{
    osMutexWait(mMutexId, osWaitForever);
    u16 value = mActualValue;
    osMutexRelease(mMutexId);
    return value;
}

bool MCP4716_readOutputVoltage(u16* value)
{
    osMutexWait(mMutexId, osWaitForever);
    
    bool result = true;
    
    TByte data [6];
    
    if (!I2C1_receive(MCP4716_DEVICE_ADDRESS, data, 6, 100))
    {
        Logger_error("%s: Failure in receiving I2C data during getting output voltage!", getLoggerPrefix());
        FaultIndication_start(EFaultId_Communication, EUnitId_Nucleo, EUnitId_MCP4716);
        result = false;
    }
    else
    {
        u16 outputVoltage = 0x00;
        outputVoltage |= ( ( ( (u16) (data[1]) ) << 2 ) & 0x3FC);
        outputVoltage |= ( ( ( (u16) (data[2]) ) >> 6 ) & 0x03);
        
        Logger_debug("%s: Output voltage (raw value): %u.", getLoggerPrefix(), outputVoltage);
        
        mActualValue = outputVoltage;
        *value = outputVoltage;
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

float MCP4716_convertOutputVoltageToRealData(u16 outputVoltage)
{
    osMutexWait(mMutexId, osWaitForever);
    
    float value = convertOutputVoltageToRealData(outputVoltage);
    
    osMutexRelease(mMutexId);
    
    return value;
}

bool isDeviceReady(u8 maxCheckAttempts, TTimeMs intervalBetweenAttempts)
{
    Logger_debug("%s: Checking if device is ready...", getLoggerPrefix());
    u8 iter;
    for (iter = 1; maxCheckAttempts >= iter; ++iter)
    {
        Logger_debug("%s: Attempt (%u/%u).", getLoggerPrefix(), iter, maxCheckAttempts);
        if (I2C1_isDeviceReady(MCP4716_DEVICE_ADDRESS, 100))
        {
            Logger_debug("%s: Device is ready after %u attempts.", getLoggerPrefix(), iter);
            return true;
        }
        else
        {
            osDelay(intervalBetweenAttempts);
        }
    }
    
    Logger_error("%s: Device is not ready after %u attempts.", getLoggerPrefix(), iter - 1);
    FaultIndication_start(EFaultId_Communication, EUnitId_Nucleo, EUnitId_MCP4716);
    return false;
}

bool configureRegisters(void)
{
    TByte data [3] =
    {
        0x00, // Command
        0x00, // DAC Value 0
        0x00  // DAC Value 0
    };
    
    data[0] &= MCP4716_COMMAND_MASK;
    data[0] |= MCP4716_COMMAND_ALL_MEMORY;
    
    data[0] &= MCP4716_VOLTAGE_REFERENCE_MASK;
    data[0] |= MCP4716_VOLTAGE_REFERENCE_VREF_BUFFERED;
    
    data[0] &= MCP4716_POWER_DOWN_MASK;
    data[0] |= MCP4716_POWER_DOWN_640_KOHM_TO_GROUND;
    
    data[0] &= MCP4716_GAIN_MASK;
    data[0] |= MCP4716_GAIN_1_X;
    
    mGain = 1;
    
    if (!I2C1_transmit(MCP4716_DEVICE_ADDRESS, data, 3, 100))
    {
        Logger_error("%s: Failure in transmitting I2C data during configuring registers!", getLoggerPrefix());
        FaultIndication_start(EFaultId_Communication, EUnitId_Nucleo, EUnitId_MCP4716);
        return false;
    }
    
    TByte setRegister = data[0];
    
    if (!I2C1_receive(MCP4716_DEVICE_ADDRESS, data, 3, 100))
    {
        Logger_error("%s: Failure in receiving I2C data during configuring registers!", getLoggerPrefix());
        FaultIndication_start(EFaultId_Communication, EUnitId_Nucleo, EUnitId_MCP4716);
        return false;
    }
    
    if (!checkDeviceStatusBits(data[0]))
    {
        Logger_error("%s: Failure in checking device readiness! Device is not ready!", getLoggerPrefix());
        FaultIndication_start(EFaultId_Communication, EUnitId_Nucleo, EUnitId_MCP4716);
        return false;
    }
    
    if (!validateMainRegister(setRegister, data[0]))
    {
        Logger_error("%s: Failure in validating transmitted data!", getLoggerPrefix());
        FaultIndication_start(EFaultId_Communication, EUnitId_Nucleo, EUnitId_MCP4716);
        return false;
    }
    
    Logger_debug("%s: Registers configured!", getLoggerPrefix());
    
    return true;
}

bool validateMainRegister(TByte transmittedRegister, TByte receivedRegister)
{
    receivedRegister &= MCP4716_COMMAND_MASK;
    transmittedRegister &= MCP4716_COMMAND_MASK;
    
    return ( transmittedRegister == receivedRegister );
}

bool checkDeviceStatusBits(TByte controlByte)
{
    if (!IS_BIT_SET(controlByte, MCP4716_POR_BIT))
    {
        Logger_error("%s: POR in voltage status bits is reset! Device is in powered off state...", getLoggerPrefix());
        return false;
    }
    
    return true;
}

float convertOutputVoltageToRealData(u16 outputVoltage)
{
    return ( ( ( 2.5F * ( (float) (outputVoltage) ) ) / 1024.0F ) * ( (float) mGain ) );
}

const char* getLoggerPrefix(void)
{
    return "MCP4716";
}
