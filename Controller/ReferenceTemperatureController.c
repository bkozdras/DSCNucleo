#include "Controller/ReferenceTemperatureController.h"

#include "Defines/CommonDefines.h"
#include "Defines/GPIOMacros.h"
#include "FaultManagement/FaultIndication.h"
#include "HardwareSettings/GPIODefines.h"
#include "Peripherals/EXTI.h"
#include "Utilities/Logger/Logger.h"

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

static osMutexDef(mMutex);
static osMutexId mMutexId = NULL;

static void (*mDRV595UnitFaultyCallback)(void) = NULL;

static bool checkIfDRV595UnitNotFaulty(void);
static void initializeGpio(void);
static void internalDRV595UnitFaultyCallback(void);
static void startStabilization(void);
static void stopStabilization(void);
static const char* getLoggerPrefix(void);

void ReferenceTemperatureController_setup(void)
{
    mMutexId = osMutexCreate(osMutex(mMutex));
    initializeGpio();
}

void ReferenceTemperatureController_initialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    EXTI_setCallback(GET_GPIO_PIN(DRV595_FAULT_PIN), internalDRV595UnitFaultyCallback);
    Logger_info("%s: Initialized!", getLoggerPrefix());
    osMutexRelease(mMutexId);
}

bool ReferenceTemperatureController_startStabilization(void)
{
    osMutexWait(mMutexId, osWaitForever);
    Logger_debug("%s: Starting stabilization reference temperature...", getLoggerPrefix());
    startStabilization();
    bool result = checkIfDRV595UnitNotFaulty();
    if (result)
    {
        Logger_info("%s: Reference temperature is stabilized!", getLoggerPrefix());
    }
    else
    {
        Logger_error("%s: Over current detected in Peltier circuit by DRV595 unit. Reference temperature is not stabilized.");
        FaultIndication_start(EFaultId_OverCurrent, EUnitId_Peltier, EUnitId_Empty);
    }
    osMutexRelease(mMutexId);
    return result;
}

void ReferenceTemperatureController_stopStabilization(void)
{
    osMutexWait(mMutexId, osWaitForever);
    stopStabilization();
    Logger_warning("%s: Reference temperature stabilization is stopped.", getLoggerPrefix());
    osMutexRelease(mMutexId);
}

void ReferenceTemperatureController_registerDRV595UnitFaultyCallback(void (*callback)(void))
{
    osMutexWait(mMutexId, osWaitForever);
    mDRV595UnitFaultyCallback = callback;
    osMutexRelease(mMutexId);
}

void ReferenceTemperatureController_deregisterDRV595UnitFaultyCallback(void)
{
    osMutexWait(mMutexId, osWaitForever);
    mDRV595UnitFaultyCallback = NULL;
    osMutexRelease(mMutexId);
}

bool checkIfDRV595UnitNotFaulty(void)
{
    return GPIO_IS_HIGH_LEVEL(DRV595_FAULT_PORT, DRV595_FAULT_PIN);
}

void initializeGpio(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // SHUTDOWN
    
    GPIO_CLOCK_ENABLE(DRV595_SHUTDOWN_PORT);
    
    GPIO_InitStruct.Pin = GET_GPIO_PIN(DRV595_SHUTDOWN_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GET_GPIO_PORT(DRV595_SHUTDOWN_PORT), &GPIO_InitStruct);

    GPIO_SET_HIGH_LEVEL(DRV595_SHUTDOWN_PORT, DRV595_SHUTDOWN_PIN);
    
    // FAULT
    
    GPIO_CLOCK_ENABLE(DRV595_FAULT_PORT);

    GPIO_InitStruct.Pin = GET_GPIO_PIN(DRV595_FAULT_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    //GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GET_GPIO_PORT(DRV595_FAULT_PORT), &GPIO_InitStruct);
}

void internalDRV595UnitFaultyCallback(void)
{
    stopStabilization();
    FaultIndication_start(EFaultId_OverCurrent, EUnitId_Peltier, EUnitId_Empty);
    if (mDRV595UnitFaultyCallback)
    {
        (*mDRV595UnitFaultyCallback)();
    }
}

void startStabilization(void)
{
    GPIO_SET_LOW_LEVEL(DRV595_SHUTDOWN_PORT, DRV595_SHUTDOWN_PIN);
}

void stopStabilization(void)
{
    GPIO_SET_HIGH_LEVEL(DRV595_SHUTDOWN_PORT, DRV595_SHUTDOWN_PIN);
}

const char* getLoggerPrefix(void)
{
    return "ReferenceTemperatureController";
}
