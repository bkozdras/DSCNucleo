#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

#include "Defines/GPIOMacros.h"
#include "HardwareSettings/GPIODefines.h"

#include "Peripherals/LED.h"
#include "Utilities/Logger/Logger.h"

#include "stdbool.h"

static osMutexDef(mMutexLed);
static osMutexId mMutexLedId = NULL;

static bool mIsInitialized = false;
static ELedState mLedState = ELedState_Off;

void LED_setup(void)
{
    if ( !mMutexLedId )
    {
        mMutexLedId = osMutexCreate( osMutex(mMutexLed) );
    }
}

void LED_initialize(void)
{
    osMutexWait(mMutexLedId, osWaitForever);
    
    if (!mIsInitialized)
    {
        GPIO_InitTypeDef  GPIO_InitStruct;
        
        GPIO_CLOCK_ENABLE(LED_PORT);
        
        GPIO_InitStruct.Pin = GET_GPIO_PIN(LED_PIN);
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

        HAL_GPIO_Init(GET_GPIO_PORT(LED_PORT), &GPIO_InitStruct);

        GPIO_SET_LOW_LEVEL(LED_PORT, LED_PIN);

        mIsInitialized = true;
        mLedState = ELedState_Off;
        
        Logger_debug("LED: Initialized (State: %s)!", CStringConverter_ELedState(mLedState));
    }
    
    osMutexRelease(mMutexLedId);   
}

void LED_changeState(ELedState ledState)
{
    osMutexWait(mMutexLedId, osWaitForever);
    
    if ( mIsInitialized && (mLedState != ledState) )
    {
        if (ELedState_On == ledState)
        {
            GPIO_SET_HIGH_LEVEL(LED_PORT, LED_PIN);   
        }
        else
        {
            GPIO_SET_LOW_LEVEL(LED_PORT, LED_PIN);
        }
        
        mLedState = ledState;
        
        Logger_debug("LED: New state: %s.", CStringConverter_ELedState(ledState));
    }
    
    osMutexRelease(mMutexLedId); 
}
