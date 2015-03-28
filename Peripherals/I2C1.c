#include "Peripherals/I2C1.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_msp.h"
#include "stm32f4xx_hal_i2c.h"

#include "cmsis_os.h"

#include "Defines/GPIOMacros.h"
#include "HardwareSettings/GPIODefines.h"
#include "FaultManagement/FaultIndication.h"
#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"

static osMutexDef(mMutex);
static osMutexId mMutexId = NULL;

static bool mIsInitialized = false;
static I2C_HandleTypeDef mI2CHandle;

static void mspInit(I2C_HandleTypeDef* i2cHandle);
static void mspDeInit(I2C_HandleTypeDef* i2cHandle);
static const char* getLoggerPrefix(void);

void I2C1_setup(void)
{
    if (!mMutexId)
    {
        mMutexId = osMutexCreate(osMutex(mMutex));
    }
}

void I2C1_initialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    
    MSP_setHAL_I2C_MspInitCallback(mspInit);
    
    mI2CHandle.Instance = I2C1;
    mI2CHandle.Init.ClockSpeed = 100000;
    mI2CHandle.Init.DutyCycle = I2C_DUTYCYCLE_2;
    mI2CHandle.Init.OwnAddress1 = 0;
    mI2CHandle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    mI2CHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
    mI2CHandle.Init.OwnAddress2 = 0;
    mI2CHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
    mI2CHandle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
    HAL_StatusTypeDef halStatus = HAL_I2C_Init(&mI2CHandle);
    
    Logger_debug("%s: Initialization: HAL status: %s.", getLoggerPrefix(), CStringConverter_HAL_StatusTypeDef(halStatus));
    
    if(HAL_OK != halStatus)
    {
        Logger_error("%s: Initialization failed (Reason: %s)!", getLoggerPrefix(), CStringConverter_HAL_StatusTypeDef(halStatus));
        FaultIndication_start(EFaultId_I2C, EUnitId_Nucleo, EUnitId_Empty);
        mIsInitialized = false;
    }
    else
    {
        Logger_info("%s: Initialized!", getLoggerPrefix());
        mIsInitialized = true;
    }
    
    osMutexRelease(mMutexId);
}

void I2C1_uninitialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    
    MSP_setHAL_I2C_MspDeInitCallback(mspDeInit);
    
    HAL_StatusTypeDef halStatus = HAL_I2C_DeInit(&mI2CHandle);
    
    Logger_debug("%s: Deinitialization: HAL status: %s.", getLoggerPrefix(), CStringConverter_HAL_StatusTypeDef(halStatus));
    
    if(HAL_OK != halStatus)
    {
        Logger_error("%s: Deinitialization failed (Reason: %s)!", getLoggerPrefix(), CStringConverter_HAL_StatusTypeDef(halStatus));
        FaultIndication_start(EFaultId_I2C, EUnitId_Nucleo, EUnitId_Empty);
    }
    else
    {
        Logger_warning("%s: Deinitialized!", getLoggerPrefix());
        mIsInitialized = false;
    }
    
    osMutexRelease(mMutexId);
}

bool I2C1_isDeviceReady(const TI2CDeviceAddress address, const TTimeMs timeout)
{
    osMutexWait(mMutexId, osWaitForever);
    
    bool deviceStatus = false;
    if (mIsInitialized)
    {
        HAL_StatusTypeDef halStatus = HAL_I2C_IsDeviceReady(&mI2CHandle, address, 1, timeout);
        if (HAL_OK == halStatus)
        {
            Logger_debug("%s: Device 0x%02X is initialized.", getLoggerPrefix(), address);
            deviceStatus = true;
        }
        else
        {
            Logger_warning("%s: Device 0x%02X is not initialized.", getLoggerPrefix(), address);
        }
    }
    
    osMutexRelease(mMutexId);
    
    return deviceStatus;
}

bool I2C1_transmit(const TI2CDeviceAddress address, const TByte* data, const u8 dataLength, const TTimeMs timeout)
{
    bool result = false;
    
    osMutexWait(mMutexId, osWaitForever);
    
    if (mIsInitialized)
    {
        HAL_StatusTypeDef halStatus = HAL_I2C_Master_Transmit(&mI2CHandle, address, (uint8_t*)data, dataLength, timeout);
        TI2CBusError halI2CError = HAL_I2C_GetError(&mI2CHandle);
        
        if (HAL_OK == halStatus && HAL_I2C_ERROR_NONE == halI2CError)
        {
            Logger_debug("%s: Transmitted %u bytes to device: 0x%02X.", getLoggerPrefix(), dataLength, address);
            for (u8 iter = 0; dataLength > iter; ++iter)
            {
                Logger_debug("%s: Byte[%u]: 0x%02X.", getLoggerPrefix(), iter, data[iter]);
            }
            result = true;
        }
        else
        {
            Logger_error
            (
                "%s: Transmitting %u bytes to device: 0x%02X failed (Reason: %s, %s)!",
                getLoggerPrefix(),
                dataLength,
                address,
                CStringConverter_HAL_StatusTypeDef(halStatus),
                CStringConverter_TI2CBusError(halI2CError)
            );
            FaultIndication_start(EFaultId_I2C, EUnitId_Nucleo, EUnitId_Empty);
        }
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

bool I2C1_receive(const TI2CDeviceAddress address, TByte* dataBuffer, const u8 dataLength, const TTimeMs timeout)
{
    bool result = false;
    
    osMutexWait(mMutexId, osWaitForever);
    
    if (mIsInitialized)
    {
        HAL_StatusTypeDef halStatus = HAL_I2C_Master_Receive(&mI2CHandle, address, (uint8_t*)dataBuffer, dataLength, timeout);
        TI2CBusError halI2CError = HAL_I2C_GetError(&mI2CHandle);
        
        if (HAL_OK == halStatus && HAL_I2C_ERROR_NONE == halI2CError)
        {
            Logger_debug("%s: Received %u bytes from device: 0x%02X.", getLoggerPrefix(), dataLength, address);
            for (u8 iter = 0; dataLength > iter; ++iter)
            {
                Logger_debug("%s: Byte[%u]: 0x%02X.", getLoggerPrefix(), iter, dataBuffer[iter]);
            }
            result = true;
        }
        else
        {
            Logger_error
            (
                "%s: Receiving %u bytes from device: %u failed (Reason: %s, %s)!",
                getLoggerPrefix(),
                dataLength,
                address,
                CStringConverter_HAL_StatusTypeDef(halStatus),
                CStringConverter_TI2CBusError(halI2CError)
            );
            FaultIndication_start(EFaultId_I2C, EUnitId_Nucleo, EUnitId_Empty);
        }
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

bool I2C1_isInitialized(void)
{
    bool isInitialized;
    
    osMutexWait(mMutexId, osWaitForever);
    isInitialized = mIsInitialized;
    osMutexRelease(mMutexId);
    
    return isInitialized;
}

void mspInit(I2C_HandleTypeDef* i2cHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_CLOCK_ENABLE(I2C1_SCL_PORT);
    GPIO_CLOCK_ENABLE(I2C1_SDA_PORT);
    
    __I2C1_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GET_GPIO_PIN(I2C1_SCL_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    //GPIO_InitStruct.Alternate = I2Cx_SCL_AF;
    HAL_GPIO_Init(GET_GPIO_PORT(I2C1_SCL_PORT), &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GET_GPIO_PIN(I2C1_SDA_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    //GPIO_InitStruct.Alternate = I2Cx_SDA_AF;
    HAL_GPIO_Init(GET_GPIO_PORT(I2C1_SDA_PORT), &GPIO_InitStruct);
}

void mspDeInit(I2C_HandleTypeDef* i2cHandle)
{
    __I2C1_CLK_DISABLE();
    HAL_GPIO_DeInit(GET_GPIO_PORT(I2C1_SCL_PORT), GET_GPIO_PIN(I2C1_SCL_PIN));
    HAL_GPIO_DeInit(GET_GPIO_PORT(I2C1_SDA_PORT), GET_GPIO_PIN(I2C1_SDA_PIN));
}

const char* getLoggerPrefix(void)
{
    return "I2C1";
}
