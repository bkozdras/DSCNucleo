#include "Peripherals/TIM3.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_msp.h"
#include "stm32f4xx_hal_tim.h"

#include "FaultManagement/FaultIndication.h"
#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"

static bool mIsInitialized = false;
static u16 mPeriodMsDiv10 = 0;
static void (*mPeriodElapsedCallback)(void) = NULL;
TIM_HandleTypeDef mTim3Handle;

static void baseMspInit(TIM_HandleTypeDef* timHandle);
static void baseMspDeInit(TIM_HandleTypeDef* timHandle);
static const char* getLoggerPrefix(void);

void TIM3_setup(void)
{
}

void TIM3_setPeriod(u16 periodMs)
{
    mPeriodMsDiv10 = periodMs;// / 10;
    Logger_debug("%s: Set period: %u ms.", getLoggerPrefix(), mPeriodMsDiv10 * 10);
}

bool TIM3_start(void)
{
    if (!mIsInitialized)
    {
        Logger_debug("%s: Starting timer (Period: %u ms).", getLoggerPrefix(), mPeriodMsDiv10 * 10);
        
        MSP_setHAL_TIM_Base_MspInitCallback(baseMspInit);
        
        // 84 MHz 
        
        mTim3Handle.Instance = TIM3;
        //mTim3Handle.Init.Period = 40 - 1; // 4 kHz / 40 = 100 Hz (10 ms IT period)
        //mTim3Handle.Init.Prescaler = 21000; // 84 MHz / 21000 = 4 kHz
        //mTim3Handle.Init.Period = 1000 - 1;
        //mTim3Handle.Init.Prescaler = (u32)( ((SystemCoreClock / 8) / 1000) - 1);
        mTim3Handle.Init.Period = 10000 - 1;
        mTim3Handle.Init.Prescaler = 84 - 1;
        mTim3Handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        mTim3Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
        
        Logger_debug("%s: Period is %u. Prescaler is %u.", getLoggerPrefix(), mTim3Handle.Init.Period, mTim3Handle.Init.Prescaler);
        
        HAL_StatusTypeDef result = HAL_TIM_Base_Init(&mTim3Handle);
        if(HAL_OK == result)
        {
            Logger_debug("%s: Starting timer (TIM base initialization) done.", getLoggerPrefix());
        }
        else
        {
            Logger_error("%s: Starting timer failure (Error code: %s).", getLoggerPrefix(), CStringConverter_HAL_StatusTypeDef(result));
            FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
            return false;
        }

        result = HAL_TIM_Base_Start_IT(&mTim3Handle);
        if(HAL_OK == result)
        {
            Logger_debug("%s: Starting timer (TIM base interrupt initialization) done.", getLoggerPrefix());
        }
        else
        {
            Logger_error("%s: Starting timer failure (Error code: %s).", getLoggerPrefix(), CStringConverter_HAL_StatusTypeDef(result));
            FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
            return false;
        }
        
        mIsInitialized = true;
        Logger_debug("%s: Timer started!", getLoggerPrefix());
        return true;
    }
    else
    {
        Logger_warning("%s: Timer is running. New starting attempt skipped.", getLoggerPrefix());
        return true;
    }
}

bool TIM3_stop(void)
{
    if (mIsInitialized)
    {
        Logger_debug("%s: Stopping timer.", getLoggerPrefix());
        
        MSP_setHAL_TIM_Base_MspDeInitCallback(baseMspDeInit);
        
        HAL_StatusTypeDef result = HAL_TIM_Base_Stop_IT(&mTim3Handle);
        if (HAL_OK == result)
        {
            Logger_debug("%s: Stopping timer (TIM base interrupt deinitialization) done.", getLoggerPrefix());
        }
        else
        {
            Logger_error("%s: Stopping timer failure (Error code: %s).", getLoggerPrefix(), CStringConverter_HAL_StatusTypeDef(result));
            FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
            return false;
        }
        
        result = HAL_TIM_Base_DeInit(&mTim3Handle);
        if (HAL_OK == result)
        {
            Logger_debug("%s: Stopping timer (TIM base deinitialization) done.", getLoggerPrefix());
        }
        else
        {
            Logger_error("%s: Stopping timer failure (Error code: %s).", getLoggerPrefix(), CStringConverter_HAL_StatusTypeDef(result));
            FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
            return false;
        }
        
        mIsInitialized = false;
        Logger_debug("%s: Timer stopped!", getLoggerPrefix());
        return true;
    }
    else
    {
        Logger_warning("%s: Timer is not running. New stopping attempt skipped.", getLoggerPrefix());
        return true;
    }
}

void TIM3_registerPeriodElapsedCallback(void (*periodElapsedCallback)(void))
{
    mPeriodElapsedCallback = periodElapsedCallback;
}

void TIM3_deregisterPeriodElapsedCallback(void)
{
    mPeriodElapsedCallback = NULL;
}

void baseMspInit(TIM_HandleTypeDef* timHandle)
{
    __HAL_RCC_TIM3_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM3_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void baseMspDeInit(TIM_HandleTypeDef* timHandle)
{
    __HAL_RCC_TIM3_FORCE_RESET();
    __HAL_RCC_TIM3_RELEASE_RESET();
    HAL_NVIC_DisableIRQ(TIM3_IRQn);
}

const char* getLoggerPrefix(void)
{
    return "TIM3";
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* timHandler)
{
    if (&mTim3Handle == timHandler)
    {
        static u16 internalMsCounter = 0;
        
        if (mPeriodMsDiv10 == ++internalMsCounter)
        {
            internalMsCounter = 0;
            if (mPeriodElapsedCallback)
            {
                (*mPeriodElapsedCallback)();
            }
        }
    }
}
