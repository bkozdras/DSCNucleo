#include "Peripherals/EXTI.h"

#include "Defines/CommonDefines.h"

#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"

#include "stm32f4xx_hal.h"

#include "cmsis_os.h"

static osMutexDef(mMutex);
static osMutexId mMutexId = NULL;

#define PIN_UNUSED  0x0000

typedef struct _TPairPinCallback
{
    TPin pin;
    void (*callback)(void);
} TPairPinCallback;

typedef struct _SExtiData
{
    EExtiType type;
    IRQn_Type irq;
    bool isInitialized;
} SExtiData;

#define AVAILABLE_CALLBACKS_COUNT   7
#define AVAILABLE_EXTIS             7

static TPairPinCallback mPairPinCallback [AVAILABLE_CALLBACKS_COUNT];
static SExtiData mPairExtiTypeIRQ [AVAILABLE_EXTIS] =
    {
        { EExtiType_EXTI0,      EXTI0_IRQn,     false },
        { EExtiType_EXTI1,      EXTI1_IRQn,     false },
        { EExtiType_EXTI2,      EXTI2_IRQn,     false },
        { EExtiType_EXTI3,      EXTI3_IRQn,     false },
        { EExtiType_EXTI4,      EXTI4_IRQn,     false },
        { EExtiType_EXTI9_5,    EXTI9_5_IRQn,   false },
        { EExtiType_EXTI15_10,  EXTI15_10_IRQn, false }
    };

static TPairPinCallback* getPairPinCallback(TPin pin);
static TPairPinCallback* getNewPairPinCallback(void);
    
void EXTI_setup(void)
{
    for (u8 iter = 0; AVAILABLE_CALLBACKS_COUNT > iter; ++iter)
    {
        mPairPinCallback[iter].pin = PIN_UNUSED;
        mPairPinCallback[iter].callback = NULL;
    }
    
    if (!mMutexId)
    {
        mMutexId = osMutexCreate(osMutex(mMutex));
    }
}

void EXTI_initialize(EExtiType extiType)
{
    osMutexWait(mMutexId, osWaitForever);
    if (!mPairExtiTypeIRQ[extiType].isInitialized)
    {
        Logger_debug("%s: Initializing started.", CStringConverter_EExtiType(extiType));
        HAL_NVIC_SetPriority(mPairExtiTypeIRQ[extiType].irq, 5, 0);
        HAL_NVIC_EnableIRQ(mPairExtiTypeIRQ[extiType].irq);
        mPairExtiTypeIRQ[extiType].isInitialized = true;
        Logger_info("%s: Initialized!", CStringConverter_EExtiType(extiType));
    }
    else
    {
        Logger_warning("%s: Initialized before. Operation skipped.", CStringConverter_EExtiType(extiType));
    }
    osMutexRelease(mMutexId);
}

bool EXTI_isInitialized(EExtiType extiType)
{
    osMutexWait(mMutexId, osWaitForever);
    bool isInitialized = mPairExtiTypeIRQ[extiType].isInitialized;
    osMutexRelease(mMutexId);
    return isInitialized;
}

void EXTI_setCallback(TPin pin, void (*callback)(void))
{
    osMutexWait(mMutexId, osWaitForever);

    TPairPinCallback* pairPinCallback = getPairPinCallback(pin);
    if (!pairPinCallback)
    {
        pairPinCallback = getNewPairPinCallback();
    }
    
    if (pairPinCallback)
    {
        pairPinCallback->pin = pin;
        pairPinCallback->callback = callback;
        Logger_debug("EXTI: Set EXTI callback for Pin: 0x%02X.", pin);
    }
    else
    {
        Logger_warning("EXTI: Setting EXTI callback for Pin: 0x%02X failed. No free places!", pin);
    }
    
    osMutexRelease(mMutexId);
}

void EXTI_unsetCallback(TPin pin)
{
    osMutexWait(mMutexId, osWaitForever);
    
    TPairPinCallback* pairPinCallback = getPairPinCallback(pin);
    if (pairPinCallback)
    {
        pairPinCallback->pin = PIN_UNUSED;
        pairPinCallback->callback = NULL;
        Logger_debug("EXTI: Unset EXTI callback for Pin: 0x%02X.", pin);
    }
    
    osMutexRelease(mMutexId);
}

TPairPinCallback* getPairPinCallback(TPin pin)
{
    for (u8 iter = 0; AVAILABLE_CALLBACKS_COUNT > iter; ++iter)
    {
        if (pin == mPairPinCallback[iter].pin)
        {
            return &(mPairPinCallback[iter]);
        }
    }
    
    return NULL;
}

TPairPinCallback* getNewPairPinCallback(void)
{
    for (u8 iter = 0; AVAILABLE_CALLBACKS_COUNT > iter; ++iter)
    {
        if (PIN_UNUSED == mPairPinCallback[iter].pin)
        {
            return &(mPairPinCallback[iter]);
        }
    }
    
    return NULL;
}

void HAL_GPIO_EXTI_Callback(TPin pin)
{
    for (u8 iter = 0; AVAILABLE_CALLBACKS_COUNT > iter; ++iter)
    {
        TPairPinCallback* pairPinCallback = &(mPairPinCallback[iter]);
        if (pin == pairPinCallback->pin)
        {
            if (pairPinCallback->callback)
            {
                (*(pairPinCallback->callback))();
            }
            break;
        }
    }
}
