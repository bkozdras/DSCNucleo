#include "Peripherals/SPI3.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_msp.h"
#include "stm32f4xx_hal_spi.h"

#include "cmsis_os.h"

#include "Defines/GPIOMacros.h"
#include "HardwareSettings/GPIODefines.h"
#include "FaultManagement/FaultIndication.h"
#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"

static osMutexDef(mMutex);
static osMutexId mMutexId = NULL;

static osMutexDef(mBlockingMutex);
static osMutexId mBlockingMutexId = NULL;

static bool mIsInitialized = false;
static SPI_HandleTypeDef mSpiHandle;
static ESpi3ClkPolarity mClkPolarity;

static void mspInit(SPI_HandleTypeDef* spiHandle);
static void mspDeInit(SPI_HandleTypeDef* spiHandle);
static const char* getLoggerPrefix(void);

void SPI3_setup(void)
{
    if (!mMutexId)
    {
        mMutexId = osMutexCreate(osMutex(mMutex));
    }
    
    if (!mBlockingMutexId)
    {
        mBlockingMutexId = osMutexCreate(osMutex(mBlockingMutex));
    }
}

bool SPI3_initialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    
    MSP_setHAL_SPI_MspInitCallback(mspInit);
    
    mSpiHandle.Instance = SPI3;
    mSpiHandle.Init.Mode = SPI_MODE_MASTER;
    mSpiHandle.Init.Direction = SPI_DIRECTION_2LINES;
    mSpiHandle.Init.DataSize = SPI_DATASIZE_8BIT;
    mSpiHandle.Init.CLKPolarity = SPI_POLARITY_HIGH;
    mClkPolarity = ESpi3ClkPolarity_High;
    mSpiHandle.Init.CLKPhase = SPI_PHASE_1EDGE;
    mSpiHandle.Init.NSS = SPI_NSS_SOFT;
    mSpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    mSpiHandle.Init.FirstBit = SPI_FIRSTBIT_MSB;
    mSpiHandle.Init.TIMode = SPI_TIMODE_DISABLED;
    mSpiHandle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    mSpiHandle.Init.CRCPolynomial = 7; // DUMMY VALUE
    HAL_StatusTypeDef halStatus = HAL_SPI_Init(&mSpiHandle);
    
    Logger_debug("%s: Initialization: HAL status: %s.", getLoggerPrefix(), CStringConverter_HAL_StatusTypeDef(halStatus));
    
    if(HAL_OK != halStatus)
    {
        Logger_error("%s: Initialization failed (Reason: %s)!", getLoggerPrefix(), CStringConverter_HAL_StatusTypeDef(halStatus));
        FaultIndication_start(EFaultId_Spi, EUnitId_Nucleo, EUnitId_Empty);
        mIsInitialized = false;
    }
    else
    {
        Logger_info("%s: Initialized!", getLoggerPrefix());
        mIsInitialized = true;
    }
    
    osMutexRelease(mMutexId);
    
    return mIsInitialized;
}

void SPI3_uninitialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    
    MSP_setHAL_SPI_MspDeInitCallback(mspDeInit);
    
    HAL_StatusTypeDef halStatus = HAL_SPI_DeInit(&mSpiHandle);
    
    Logger_debug("%s: Deinitialization: HAL status: %s.", getLoggerPrefix(), CStringConverter_HAL_StatusTypeDef(halStatus));
    
    if(HAL_OK != halStatus)
    {
        Logger_error("%s: Deinitialization failed (Reason: %s)!", getLoggerPrefix(), CStringConverter_HAL_StatusTypeDef(halStatus));
        FaultIndication_start(EFaultId_Spi, EUnitId_Nucleo, EUnitId_Empty);
    }
    else
    {
        Logger_warning("%s: Deinitialized!", getLoggerPrefix());
        mIsInitialized = false;
    }
    
    osMutexRelease(mMutexId);
}

void SPI3_block(ESpi3ClkPolarity clkPolarity)
{
    osMutexWait(mBlockingMutexId, osWaitForever);
    
    if (mClkPolarity != clkPolarity)
    {
        if (ESpi3ClkPolarity_High == clkPolarity)
        {
            mSpiHandle.Init.CLKPolarity = SPI_POLARITY_HIGH;
        }
        else
        {
            mSpiHandle.Init.CLKPolarity = SPI_POLARITY_LOW;
        }
        
        MSP_setHAL_SPI_MspInitCallback(mspInit);
        HAL_StatusTypeDef halStatus = HAL_SPI_Init(&mSpiHandle);
        if(HAL_OK != halStatus)
        {
            Logger_error("%s: Initialization failed (Reason: %s)!", getLoggerPrefix(), CStringConverter_HAL_StatusTypeDef(halStatus));
            FaultIndication_start(EFaultId_Spi, EUnitId_Nucleo, EUnitId_Empty);
            mIsInitialized = false;
            assert_param(false);
        }
    }
}

void SPI3_unblock(void)
{
    osMutexRelease(mBlockingMutexId);
}

bool SPI3_transmit(const TByte* data, const u8 dataLength, const TTimeMs timeout)
{
    bool result = false;
    
    osMutexWait(mMutexId, osWaitForever);
    
    if (mIsInitialized)
    {
        HAL_StatusTypeDef halStatus = HAL_SPI_Transmit(&mSpiHandle, (uint8_t*)data, dataLength, timeout);
        TSpiBusError halSpiError = HAL_SPI_GetError(&mSpiHandle);
        
        if (HAL_OK == halStatus && HAL_SPI_ERROR_NONE == halSpiError)
        {
            Logger_debug("%s: Transmitted %u bytes:", getLoggerPrefix(), dataLength);
            for (u8 iter = 0; dataLength > iter; ++iter)
            {
                Logger_debugSystem("%s: Byte[%u]: 0x%02X.", getLoggerPrefix(), iter, data[iter]);
            }
            result = true;
        }
        else
        {
            Logger_error
            (
                "%s: Transmitting %u bytes: %u failed (Reason: %s, %s)!",
                getLoggerPrefix(),
                dataLength,
                CStringConverter_HAL_StatusTypeDef(halStatus),
                CStringConverter_TSpiBusError(halSpiError)
            );
            FaultIndication_start(EFaultId_Spi, EUnitId_Nucleo, EUnitId_Empty);
        }
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

bool SPI3_receive(TByte* dataBuffer, const u8 dataLength, const TTimeMs timeout)
{
    bool result = false;
    
    osMutexWait(mMutexId, osWaitForever);
    
    if (mIsInitialized)
    {
        HAL_StatusTypeDef halStatus = HAL_SPI_Receive(&mSpiHandle, (uint8_t*)dataBuffer, dataLength, timeout);
        TSpiBusError halSpiError = HAL_SPI_GetError(&mSpiHandle);
        
        if (HAL_OK == halStatus && HAL_SPI_ERROR_NONE == halSpiError)
        {
            Logger_debug("%s: Received %u bytes:", getLoggerPrefix(), dataLength);
            for (u8 iter = 0; dataLength > iter; ++iter)
            {
                Logger_debugSystem("%s: Byte[%u]: 0x%02X.", getLoggerPrefix(), iter, dataBuffer[iter]);
            }
            result = true;
        }
        else
        {
            Logger_error
            (
                "%s: Receiving %u bytes: %u failed (Reason: %s, %s)!",
                getLoggerPrefix(),
                dataLength,
                CStringConverter_HAL_StatusTypeDef(halStatus),
                CStringConverter_TSpiBusError(halSpiError)
            );
            FaultIndication_start(EFaultId_Spi, EUnitId_Nucleo, EUnitId_Empty);
        }
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

bool SPI3_transmitReceive(const TByte* dataToSend, TByte* dataToReceive, const u8 dataLength, const TTimeMs timeout)
{
    bool result = false;
    
    osMutexWait(mMutexId, osWaitForever);
    
    if (mIsInitialized)
    {
        HAL_StatusTypeDef halStatus = HAL_SPI_TransmitReceive(&mSpiHandle, (uint8_t*)dataToSend, (uint8_t*)dataToReceive, dataLength, timeout);
        TSpiBusError halSpiError = HAL_SPI_GetError(&mSpiHandle);
        
        if (HAL_OK == halStatus && HAL_SPI_ERROR_NONE == halSpiError)
        {
            Logger_debug("%s: Sent and received %u bytes:", getLoggerPrefix(), dataLength);
            for (u8 iter = 0; dataLength > iter; ++iter)
            {
                Logger_debugSystem("%s: Byte[%u]: Sent[0x%02X] Received[0x%02X].", getLoggerPrefix(), iter, dataToSend[iter], dataToReceive[iter]);
            }
            result = true;
        }
        else
        {
            Logger_error
            (
                "%s: Sending and receiving %u bytes: %u failed (Reason: %s, %s)!",
                getLoggerPrefix(),
                dataLength,
                CStringConverter_HAL_StatusTypeDef(halStatus),
                CStringConverter_TSpiBusError(halSpiError)
            );
            FaultIndication_start(EFaultId_Spi, EUnitId_Nucleo, EUnitId_Empty);
        }
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

bool SPI3_isInitialized(void)
{
    bool isInitialized;
    
    osMutexWait(mMutexId, osWaitForever);
    isInitialized = mIsInitialized;
    osMutexRelease(mMutexId);
    
    return isInitialized;
}

void mspInit(SPI_HandleTypeDef* spiHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_CLOCK_ENABLE(SPI3_SCK_PORT);
    GPIO_CLOCK_ENABLE(SPI3_MISO_PORT);
    GPIO_CLOCK_ENABLE(SPI3_MOSI_PORT);
    //__SPI2_CLK_ENABLE();
  
    GPIO_InitStruct.Pin = GET_GPIO_PIN(SPI3_MISO_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    //GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GET_GPIO_PORT(SPI3_MISO_PORT), &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GET_GPIO_PIN(SPI3_MOSI_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    //GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GET_GPIO_PORT(SPI3_MOSI_PORT), &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GET_GPIO_PIN(SPI3_SCK_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    //GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GET_GPIO_PORT(SPI3_SCK_PORT), &GPIO_InitStruct);
    
    __HAL_RCC_SPI3_CLK_ENABLE();
}

void mspDeInit(SPI_HandleTypeDef* spiHandle)
{
    __SPI3_CLK_DISABLE();
    HAL_GPIO_DeInit(GET_GPIO_PORT(SPI3_MISO_PORT), GET_GPIO_PIN(SPI3_MISO_PIN));
    HAL_GPIO_DeInit(GET_GPIO_PORT(SPI3_MOSI_PORT), GET_GPIO_PIN(SPI3_MOSI_PIN));
    HAL_GPIO_DeInit(GET_GPIO_PORT(SPI3_SCK_PORT), GET_GPIO_PIN(SPI3_SCK_PIN));
}

const char* getLoggerPrefix(void)
{
    return "SPI3";
}
