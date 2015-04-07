#include "Controller/SegmentsManager.h"

#include "Controller/HeaterTemperatureController.h"
#include "FaultManagement/FaultIndication.h"
#include "System/KernelManager.h"
#include "Utilities/Printer/CStringConverter.h"
#include "Utilities/Logger/Logger.h"
#include "Utilities/CopyObject.h"

#include "cmsis_os.h"

#define MAX_REGISTERED_SEGMENTS_COUNT 10

typedef struct _SLocalSegmentData
{
    bool isActive;
    bool isInProgress;
    struct _SLocalSegmentData* nextSegmentInChain;
    SSegmentData data;
} SLocalSegmentData;

typedef enum _EProgramState
{
    EProgramState_Idle                      = 0,
    EProgramState_RealizingSegment          = 1,
    EProgramState_TemperatureStabilization  = 2
} EProgramState;

static osTimerId mDynamicSegmentTimerId = NULL;
static osThreadId mStaticSegmentTimerId = NULL;
static osMutexDef(mMutex);
static osMutexId mMutexId = NULL;
static EThreadId mThreadId = EThreadId_StaticSegmentProgramExecutor;

static SLocalSegmentData mRegisteredSegments [MAX_REGISTERED_SEGMENTS_COUNT];
static u16 mNumberOfRegisteredSegments = 0;
static u16 mNumberOfRealizedSegments = 0;
static SLocalSegmentData* mFirstSegmentInChain = NULL;
static SLocalSegmentData* mLastSegmentInChain = NULL;
static void (*mNewSegmentStartedIndCallback)(u16) = NULL;
static void (*mSegmentsProgramDoneIndCallback)(u16, u16) = NULL;
static bool mIsProgramRunning = false;
static bool mIsSegmentWaitingForFinishing = false;
static float mActualSetTemperature = 0.0F;

static void dynamicSegmentProgramExecutor(void const* arg);
static void staticSegmentProgramExecutor(void const* arg);
static void dynamicSegmentSetter(void);
static bool isTemperatureDeviationTooBig(void);
static bool isTemperatureReachedRequestedValue(void);
static bool isSetTemperatureLastInSegment(void);
static void segmentStartedActionExecutor(void);
static void segmentDoneActionExecutor(void);
static void sendSegmentStartedInd(void);
static void sendProgramDoneInd(void);

static SLocalSegmentData* getRegisteredSegment(u16 number);
static SLocalSegmentData* getPreviousSegment(SLocalSegmentData* segment);
static bool startProgram(void);
static bool stopProgram(void);
static const char* getLoggerPrefix(void);

void SegmentsManager_setup(void)
{
    mMutexId = osMutexCreate(osMutex(mMutex));
    osTimerDef(dynamicSegmentTemperatureSetterTimer, dynamicSegmentProgramExecutor);
    mDynamicSegmentTimerId = osTimerCreate(osTimer(dynamicSegmentTemperatureSetterTimer), osTimerPeriodic, NULL);
    osThreadDef(staticSegmentTemperatureSetterThread, staticSegmentProgramExecutor, osPriorityAboveNormal, 0, configMINIMAL_STACK_SIZE);
    mStaticSegmentTimerId = osThreadCreate(osThread(staticSegmentTemperatureSetterThread), NULL);
    KernelManager_registerThread(EThreadId_StaticSegmentProgramExecutor, mStaticSegmentTimerId);
}

void SegmentsManager_initialize(void)
{
    osMutexWait(mMutexId, osWaitForever);
    
    for (u8 iter = 0; MAX_REGISTERED_SEGMENTS_COUNT > iter; ++iter)
    {
        mRegisteredSegments[iter].isActive = false;
        mRegisteredSegments[iter].isInProgress = false;
        mRegisteredSegments[iter].nextSegmentInChain = NULL;
    }
    
    osMutexRelease(mMutexId);
}

bool SegmentsManager_modifyRegisteredSegment(SSegmentData* data)
{
    osMutexWait(mMutexId, osWaitForever);
    
    bool result = true;
    SLocalSegmentData* modifiedSegment = getRegisteredSegment(data->number);
    
    if (NULL == modifiedSegment || !(modifiedSegment->isActive))
    {
        Logger_error("%s: Modifying segment %u is not possible. Segment not found in program", getLoggerPrefix(), data->number);
        result = false;
    }
    else if (modifiedSegment->isInProgress)
    {
        Logger_error("%s: Modifying segment %u is in progress. Request skipped.", getLoggerPrefix(), data->number);
        result = false;
    }
    else
    {
        CopyObject_SSegmentData(data, &(modifiedSegment->data));
        Logger_info
        (
            "%s: Modified segment %u (Start temp.: %.2f oC, Stop temp. %.2f oC).",
            getLoggerPrefix(),
            modifiedSegment->data.number,
            modifiedSegment->data.startTemperature,
            modifiedSegment->data.stopTemperature
        );
        
        result = true;
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

bool SegmentsManager_registerNewSegment(SSegmentData* data)
{
    osMutexWait(mMutexId, osWaitForever);
    
    bool result = true;
    const u16 segmentNumber = data->number;
    
    if (MAX_REGISTERED_SEGMENTS_COUNT == mNumberOfRegisteredSegments)
    {
        result = false;
        Logger_error("%s: Registering new segment (%u) is not possible. Max number of allocated segments reached.", getLoggerPrefix(), segmentNumber);
    }
    else if (NULL != getRegisteredSegment(segmentNumber))
    {
        result = false;
        Logger_error("%s: Registering new segment (%u) is not possible. Requested segment is exist in program.", getLoggerPrefix(), segmentNumber);
    }
    else
    {
        if (NULL == mFirstSegmentInChain)
        {
            mLastSegmentInChain = &(mRegisteredSegments[0]);
            mFirstSegmentInChain = mLastSegmentInChain;
        }
        else
        {
            for (u8 iter = 0; MAX_REGISTERED_SEGMENTS_COUNT > iter; ++iter)
            {
                if ( !(mRegisteredSegments[iter].isActive) )
                {
                    mLastSegmentInChain->nextSegmentInChain = &(mRegisteredSegments[iter]);
                    mLastSegmentInChain = mLastSegmentInChain->nextSegmentInChain;
                    break;
                }
            }
        }

        result = true;
        mLastSegmentInChain->isActive = true;
        mLastSegmentInChain->isInProgress = false;
        mLastSegmentInChain->nextSegmentInChain = NULL;
        CopyObject_SSegmentData(data, &(mLastSegmentInChain->data));
        ++mNumberOfRegisteredSegments;
        
        Logger_info
        (
            "%s: Segment %u registered (Start temp.: %.2f oC, Stop temp. %.2f oC).",
            getLoggerPrefix(),
            mLastSegmentInChain->data.number,
            mLastSegmentInChain->data.startTemperature,
            mLastSegmentInChain->data.stopTemperature
        );
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

bool SegmentsManager_deregisterSegment(u16 number)
{
    osMutexWait(mMutexId, osWaitForever);
    
    bool result = true;
    SLocalSegmentData* deregisteredSegment = getRegisteredSegment(number);
    
    if (NULL == deregisteredSegment)
    {
        Logger_warning("%s: Cannot find segment %u to deregister. Segment does not exist in program.", getLoggerPrefix(), number);
        result = false;
    }
    else
    {
        if (deregisteredSegment->isInProgress)
        {
            Logger_warning("%s: Segment %u to deregister is in progress. Request skipped.");
            result = false;
        }
        else
        {
            deregisteredSegment->isActive = false;
            deregisteredSegment->isInProgress = false;
            deregisteredSegment->data.number = 0;
            
            SLocalSegmentData* previousSegment = getPreviousSegment(deregisteredSegment);
            if (NULL != previousSegment)
            {
                previousSegment->nextSegmentInChain = deregisteredSegment->nextSegmentInChain;
            }
            
            deregisteredSegment->nextSegmentInChain = NULL;
            
            Logger_info("%s: Deregistered segment %u from program.", getLoggerPrefix(), number);
            
            result = true;
        }
    }
    
    osMutexRelease(mMutexId);
    
    return result;
}

void SegmentsManager_registerSegmentStartedIndCallback(void (*callback)(u16))
{
    osMutexWait(mMutexId, osWaitForever);
    
    mNewSegmentStartedIndCallback = callback;
    Logger_debug("%s: Registered callback for new segment started indication.", getLoggerPrefix());
    
    osMutexRelease(mMutexId);
}

void SegmentsManager_deregisterSegmentStartedIndCallback(void)
{
    osMutexWait(mMutexId, osWaitForever);
    
    mNewSegmentStartedIndCallback = NULL;
    Logger_debug("%s: Deregistered callback for new segment started indication.", getLoggerPrefix());
    
    osMutexRelease(mMutexId);
}

void SegmentsManager_registerSegmentsProgramDoneIndCallback(void (*callback)(u16, u16))
{
    osMutexWait(mMutexId, osWaitForever);
    
    mSegmentsProgramDoneIndCallback = callback;
    Logger_debug("%s: Registered callback for segments program done indication.", getLoggerPrefix());
    
    osMutexRelease(mMutexId);
}

void SegmentsManager_deregisterSegmentsProgramDoneIndCallback(void)
{
    osMutexWait(mMutexId, osWaitForever);
    
    mSegmentsProgramDoneIndCallback = NULL;
    Logger_debug("%s: Deregistered callback for segments program done indication.", getLoggerPrefix());
    
    osMutexRelease(mMutexId);
}

bool SegmentsManager_startProgram(void)
{
    osMutexWait(mMutexId, osWaitForever);
    bool result = startProgram();
    osMutexRelease(mMutexId);
    
    return result;
}

bool SegmentsManager_stopProgram(void)
{
    osMutexWait(mMutexId, osWaitForever);
    
    osMutexRelease(mMutexId);
    
    return false;
}

void SegmentsManager_clearProgram(void)
{
    osMutexWait(mMutexId, osWaitForever);
    
    if (mIsProgramRunning)
    {
        Logger_info("%s: Clearing program. Stopping executing program...", getLoggerPrefix());
        stopProgram();
    }
    
    for (u8 iter = 0; MAX_REGISTERED_SEGMENTS_COUNT > iter; ++iter)
    {
        mRegisteredSegments[iter].isInProgress = false;
        mRegisteredSegments[iter].isActive = false;
    }
    
    Logger_info("%s: Segment program is cleared!", getLoggerPrefix());
    
    osMutexRelease(mMutexId);
}

void dynamicSegmentProgramExecutor(void const* arg)
{
    osMutexWait(mMutexId, osWaitForever);
    
    static bool isFirstEnteringInSegment = false;
    
    if (isFirstEnteringInSegment)
    {
        segmentStartedActionExecutor();
        isFirstEnteringInSegment = false;
    }
    else if (mIsSegmentWaitingForFinishing)
    {
        if (isTemperatureReachedRequestedValue())
        {
            segmentDoneActionExecutor();
        }
    }
    else if (isSetTemperatureLastInSegment())
    {
        Logger_info("%s: Temperature is set to last in segment. Waiting for finishing segment...", getLoggerPrefix());
        mIsSegmentWaitingForFinishing = true;
    }
    else if (!isTemperatureDeviationTooBig())
    {
        dynamicSegmentSetter();
    }
    
    osMutexRelease(mMutexId);
}

void staticSegmentProgramExecutor(void const* arg)
{
    ThreadStarter_run(mThreadId);
    bool mIsThreadTerminated = false;
    while(!mIsThreadTerminated)
    {
        TEvent* event = Event_wait(mThreadId, osWaitForever);
        osMutexWait(mMutexId, osWaitForever);
        if (event)
        {
            switch (event->id)
            {
                case EEventId_StartStaticSegment :
                {
                    Event_free(mThreadId, event);
                    osMutexRelease(mMutexId);
                    segmentStartedActionExecutor();
                    osDelay(mFirstSegmentInChain->data.settingTimeInterval);
                    Logger_info("%s: Timer expired!", getLoggerPrefix());
                    osMutexWait(mMutexId, osWaitForever);
                    if (mIsProgramRunning)
                    {
                        while (!isTemperatureReachedRequestedValue())
                        {
                            osMutexRelease(mMutexId);
                            osDelay(1000);
                            osMutexWait(mMutexId, osWaitForever);
                        }
                        segmentDoneActionExecutor();
                    }
                    osMutexRelease(mMutexId);
                    break;
                }
                
                default :
                    assert_param(0);
                    break;
            }
        }
    }
    
    Logger_warning("%s: Thread TERMINATED!", getLoggerPrefix());
    osDelay(osWaitForever);
}

void dynamicSegmentSetter(void)
{
    mActualSetTemperature += mFirstSegmentInChain->data.temperatureStep;
    HeaterTemperatureController_setTemperature(mActualSetTemperature);
}

bool isTemperatureDeviationTooBig(void)
{
    if (5.0F < HeaterTemperatureController_getControllerError())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool isTemperatureReachedRequestedValue(void)
{
    // TODO: If needed, added in the future few steps to be sure if temperature is proper
    // Example: Counter - if all tests passed temperature reached proper value
    float controllerError = HeaterTemperatureController_getControllerError();
    if ( (0.15F > controllerError) && (-0.15F < controllerError) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool isSetTemperatureLastInSegment(void)
{
    return ( mActualSetTemperature == mFirstSegmentInChain->data.stopTemperature );
}

void segmentStartedActionExecutor(void)
{
    Logger_info("%s: Starting %s segment (Nr: %u).", getLoggerPrefix(), CStringConverter_ESegmentType(mFirstSegmentInChain->data.type), mFirstSegmentInChain->data.number);
    sendSegmentStartedInd();
    HeaterTemperatureController_setTemperature(mFirstSegmentInChain->data.startTemperature);
    mActualSetTemperature = mFirstSegmentInChain->data.startTemperature;
    mIsSegmentWaitingForFinishing = false;
    Logger_info
    (
        "%s: %s segment started. Temperature: &.2f oC. Time interval: %u ms.",
        getLoggerPrefix(),
        CStringConverter_ESegmentType(mFirstSegmentInChain->data.type),
        mFirstSegmentInChain->data.startTemperature,
        mFirstSegmentInChain->data.settingTimeInterval
    );
    mFirstSegmentInChain->isInProgress = true;
}

void segmentDoneActionExecutor(void)
{
    if (ESegmentType_Dynamic == mFirstSegmentInChain->data.type)
    {
        osStatus result = osTimerStop(mDynamicSegmentTimerId);
        if (osOK != result)
        {
            Logger_error("%s: Dynamic segment timer cancelling failed! Reason: %s.", getLoggerPrefix(), CStringConverter_osStatus(result));
            Logger_error("%s: Fatal error. Segment program stopped. Waiting for recovery action from Master unit!", getLoggerPrefix());
            FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
            return;
        }
    }
    
    Logger_info
    (
        "%s: Segment %u successfully finished. Requested temperature (%.2f oC) reached.",
        getLoggerPrefix(),
        mFirstSegmentInChain->data.number,
        mFirstSegmentInChain->data.stopTemperature
    );
    
    ++mNumberOfRealizedSegments;
    mIsSegmentWaitingForFinishing = false;
    
    --mNumberOfRegisteredSegments;
    mFirstSegmentInChain->isInProgress = false;
    mFirstSegmentInChain->isActive = false;
    mFirstSegmentInChain = mFirstSegmentInChain->nextSegmentInChain;
    
    if (NULL != mFirstSegmentInChain)
    {
        Logger_info
        (
            "%s: Starting next segment: %u (%s) in program.",
            getLoggerPrefix(),
            mFirstSegmentInChain->data.number,
            CStringConverter_ESegmentType(mFirstSegmentInChain->data.type)
        );
        
        if (ESegmentType_Dynamic == mFirstSegmentInChain->data.type)
        {
            osStatus result = osTimerStart(mDynamicSegmentTimerId, mFirstSegmentInChain->data.settingTimeInterval);
            if (osOK != result)
            {
                Logger_error("%s: Dynamic segment timer starting failed! Reason: %s.", getLoggerPrefix(), CStringConverter_osStatus(result));
                Logger_error("%s: Fatal error. Segment program stopped. Waiting for recovery action from Master unit!", getLoggerPrefix());
                FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
                return;
            }
        }
        else
        {
            CREATE_EVENT_ISR(StartStaticSegment, mThreadId);
            SEND_EVENT();
        }
        
        HeaterTemperatureController_setSystemType(EControlSystemType_OpenLoop);
    }
    else
    {
        Logger_info("%s: All segments executed. Program is finished!", getLoggerPrefix());
        mIsProgramRunning = false;
        sendProgramDoneInd();
    }
    
}

void sendSegmentStartedInd(void)
{
    if (mNewSegmentStartedIndCallback)
    {
        (*mNewSegmentStartedIndCallback)(mFirstSegmentInChain->data.number);
    }
}

void sendProgramDoneInd(void)
{
    if (mSegmentsProgramDoneIndCallback)
    {
        (*mSegmentsProgramDoneIndCallback)(mNumberOfRealizedSegments, mFirstSegmentInChain->data.number);
    }
}

SLocalSegmentData* getRegisteredSegment(u16 number)
{
    for (u8 iter = 0; MAX_REGISTERED_SEGMENTS_COUNT > iter; ++iter)
    {
        if (mRegisteredSegments[iter].data.number == number)
        {
            return &(mRegisteredSegments[iter]);
        }
    }
    
    return NULL;
}

SLocalSegmentData* getPreviousSegment(SLocalSegmentData* segment)
{
    for (u8 iter = 0; MAX_REGISTERED_SEGMENTS_COUNT > iter; ++iter)
    {
        if (mRegisteredSegments[iter].nextSegmentInChain == segment)
        {
            return &(mRegisteredSegments[iter]);
        }
    }
    
    return NULL;
}

bool startProgram(void)
{
    if (NULL == mFirstSegmentInChain)
    {
        Logger_warning("%s: No registered segments! Program not started...", getLoggerPrefix());
        return false;
    }
    else if (mIsProgramRunning)
    {
        Logger_warning("%s: Program is running. New start request skipped.", getLoggerPrefix());
        return true;
    }
    else
    {
        mIsProgramRunning = true;
        HeaterTemperatureController_setSystemType(EControlSystemType_SimpleFeedback);
        Logger_info("%s: Starting segment program. Number of registered segments: %u.", mNumberOfRegisteredSegments);
        if (ESegmentType_Dynamic == mFirstSegmentInChain->data.type)
        {
            Logger_info("%s: First segment is dynamic.", getLoggerPrefix());
            osStatus result = osTimerStart(mDynamicSegmentTimerId, mFirstSegmentInChain->data.settingTimeInterval);
            if (osOK != result)
            {
                Logger_error("%s: Dynamic segment timer starting failed! Reason: %s.", getLoggerPrefix(), CStringConverter_osStatus(result));
                Logger_error("%s: Fatal error. Segment program stopped. Waiting for recovery action from Master unit!", getLoggerPrefix());
                FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
                return false;
            }
        }
        else
        {
            Logger_info("%s: Fist segment is static.", getLoggerPrefix());
            CREATE_EVENT_ISR(StartStaticSegment, mThreadId);
            SEND_EVENT();
        }
    }
    
    return true;
}

bool stopProgram(void)
{
    if (!mIsProgramRunning)
    {
        Logger_warning("%s: Program is not running. New stop request skipped.", getLoggerPrefix());
        return true;
    }
    else
    {
        mIsProgramRunning = false;
        mFirstSegmentInChain->isInProgress = false;
        HeaterTemperatureController_setSystemType(EControlSystemType_OpenLoop);
        Logger_warning
        (
            "%s: Stopping segment: %u. Segment is NOT removed from program. Waiting for relaunching...",
            getLoggerPrefix(),
            mFirstSegmentInChain->data.number
        );
        
        if (ESegmentType_Dynamic == mFirstSegmentInChain->data.type)
        {
            osStatus result = osTimerStop(mDynamicSegmentTimerId);
            if (osOK != result)
            {
                Logger_error("%s: Dynamic segment timer cancelling failed! Reason: %s.", getLoggerPrefix(), CStringConverter_osStatus(result));
                Logger_error("%s: Fatal error. Segment program stopped. Waiting for recovery action from Master unit!", getLoggerPrefix());
                FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
                return false;
            }
        }
    }
    
    return true;
}

const char* getLoggerPrefix(void)
{
    return "SegmentsManager";
}

#undef MAX_REGISTERED_SEGMENTS_COUNT
