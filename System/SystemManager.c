#include "System/SystemManager.h"
#include "System/KernelManager.h"
#include "System/EventManagement/Event.h"
#include "System/EventManagement/EEventId.h"
#include "System/EventManagement/TEvent.h"
#include "System/ThreadMacros.h"

#include "FaultManagement/FaultIndication.h"

#include "stm32f4xx_hal.h"

#include "Utilities/Logger/Logger.h"
#include "Peripherals/EXTI.h"
#include "Peripherals/LED.h"
#include "Peripherals/I2C1.h"
#include "Peripherals/SPI2.h"
#include "Peripherals/SPI3.h"
#include "Peripherals/TIM3.h"
#include "Peripherals/UART1.h"
#include "Peripherals/UART2.h"

#include "Devices/ADS1248.h"
#include "Devices/LMP90100ControlSystem.h"
#include "Devices/LMP90100SignalsMeasurement.h"
#include "Devices/MCP4716.h"

#include "Controller/HeaterTemperatureReader.h"
#include "Controller/HeaterTemperatureController.h"
#include "Controller/SampleCarrierDataManager.h"
#include "Controller/ReferenceTemperatureReader.h"
#include "Controller/ReferenceTemperatureController.h"
#include "Controller/InertialModel.h"
#include "Controller/SegmentsManager.h"

#include "MasterCommunication/MasterDataManager.h"
#include "MasterCommunication/MasterDataMemoryManager.h"
#include "MasterCommunication/MasterDataReceiver.h"
#include "MasterCommunication/MasterDataTransmitter.h"
#include "MasterCommunication/MasterUartGateway.h"

#include "Testing/SampleThread.h"

#include "cmsis_os.h"

static const EThreadId mThreadId = EThreadId_SystemManager;
static void (*mUnitReadyIndCallback)(EUnitId, bool) = NULL;

static void SystemManager_thread(void const* arg);
static void setup(void);
static void createThreads(void);
static void checkKernelStatus(void);
static void conditionalExecutor(bool (*callback)(void), bool* result);
static bool configurePeripheralsPhaseOne(void);
static bool configurePeripheralsPhaseTwo(void);
static bool configureDevices(void);
static bool configureControllers(void);
static void initializeCommunicationWithMaster(void);
static void notifyObserverAboutUnitReadyStatus(EUnitId unitId, bool result);
static void startThreads(void);
static const char* getLoggerPrefix(void);

void SystemManager_run(void)
{
    setup();
    createThreads();
    osKernelStart();
}

void SystemManager_registerUnitReadyIndCallback(void (*callback)(EUnitId, bool))
{
    mUnitReadyIndCallback = callback;
}

void SystemManager_deregisterUnitReadyIndCallback(void)
{
    mUnitReadyIndCallback = NULL;
}

void setup(void)
{
    Event_setup();
    KernelManager_setup();
    
    EXTI_setup();
    I2C1_setup();
    LED_setup();
    Logger_setup();
    SPI2_setup();
    SPI3_setup();
    
    ADS1248_setup();
    LMP90100ControlSystem_setup();
    LMP90100SignalsMeasurement_setup();
    MCP4716_setup();
    
    HeaterTemperatureController_setup();
    HeaterTemperatureReader_setup();
    SampleCarrierDataManager_setup();
    SegmentsManager_setup();
    ReferenceTemperatureReader_setup();
    ReferenceTemperatureController_setup();
    
    MasterDataManager_setup();
    MasterDataMemoryManager_setup();
    MasterDataReceiver_setup();
    MasterDataTransmitter_setup();
    MasterUartGateway_setup();
}

void createThreads(void)
{
    THREAD_ID
    
    THREAD_CREATE(ADS1248Controller, Realtime, configMINIMAL_STACK_SIZE);
    THREAD_CREATE(LMP90100ControlSystemController, High, configMINIMAL_STACK_SIZE);
    THREAD_CREATE(LMP90100SignalsMeasurementController, Realtime, configMINIMAL_STACK_SIZE);
    THREAD_CREATE(SystemManager, Low, configMINIMAL_STACK_SIZE);
    THREAD_CREATE(HeaterTemperatureReader, Normal, configMINIMAL_STACK_SIZE);
    THREAD_CREATE(SampleCarrierDataManager, Normal, configMINIMAL_STACK_SIZE);
    THREAD_CREATE(ReferenceTemperatureReader, Low, configMINIMAL_STACK_SIZE);
    THREAD_CREATE(MasterDataManager, Low, configNORMAL_STACK_SIZE);
    THREAD_CREATE(MasterDataReceiver, Normal, configNORMAL_STACK_SIZE);
    THREAD_CREATE(MasterDataTransmitter, Low, configMAXIMUM_STACK_SIZE);
    THREAD_CREATE(SampleThread, Normal, configMINIMAL_STACK_SIZE);
}

void SystemManager_thread(void const* arg)
{
    Logger_initialize(ELoggerType_EvalCOM1, ELoggerLevel_DebugSystem);
    initializeCommunicationWithMaster();
    //Logger_setLevel(ELoggerLevel_Debug);
    Logger_setType(ELoggerType_EvalCOM1AndMasterMessage);
    
    Logger_info("%s: THREAD STARTED!", getLoggerPrefix());

    checkKernelStatus();
    
    Logger_info("%s: Started configuring device!", getLoggerPrefix());
    
    bool result = true;
    
    conditionalExecutor(configurePeripheralsPhaseOne, &result);
    conditionalExecutor(configureDevices, &result);
    conditionalExecutor(configureControllers, &result);
    startThreads();
    conditionalExecutor(configurePeripheralsPhaseTwo, &result);
    
    if (result)
    {
        LED_changeState(ELedState_On);
        Logger_info("%s: Device CONFIGURED!", getLoggerPrefix());
    }
    else
    {
        LED_changeState(ELedState_Off);
        Logger_error("%s: Device configuring FAILED!", getLoggerPrefix());
    }
    
    osDelay(osWaitForever);
}

void checkKernelStatus(void)
{
    if (osKernelRunning())
    {
        Logger_info("%s: Kernel started.", getLoggerPrefix());
    }
    else
    {
        Logger_error("%s: Kernel has not started!", getLoggerPrefix());
        FaultIndication_start(EFaultId_System, EUnitId_Nucleo, EUnitId_Empty);
        assert_param(false);
    }    
}

void conditionalExecutor(bool (*callback)(void), bool* result)
{
    if (callback && result && *result)
    {
        *result = (*callback)();
    }
}

bool configurePeripheralsPhaseOne(void)
{
    Logger_debug("%s: Configuring peripherals (Phase: 1/2): START.", getLoggerPrefix());
    
    bool result = true;
    
    LED_initialize();
    conditionalExecutor(I2C1_initialize, &result);
    conditionalExecutor(SPI2_initialize, &result);
    conditionalExecutor(SPI3_initialize, &result);
    
    if (UART2_isInitialized())
    {
        Logger_info("UART2: Initialized!");
    }
    else
    {
        Logger_info("UART2: Not initialized!");
    }
    
    if (result)
    {
        Logger_debug("%s: Configuring peripherals (Phase: 1/2): DONE!", getLoggerPrefix());
    }
    else
    {
        Logger_error("%s: Configuring peripherals (Phase: 1/2): FAILURE!", getLoggerPrefix());
    }
    
    return result;
}

bool configurePeripheralsPhaseTwo(void)
{
    Logger_debug("%s: Configuring peripherals (Phase: 2/2): START.", getLoggerPrefix());
    
    //EXTI_initialize(EExtiType_EXTI0);
    //EXTI_initialize(EExtiType_EXTI4);
    //EXTI_initialize(EExtiType_EXTI9_5);
    //EXTI_initialize(EExtiType_EXTI15_10);
    
    Logger_debug("%s: Configuring peripherals (Phase: 2/2): DONE!", getLoggerPrefix());
    return true;
}

bool configureDevices(void)
{
    Logger_debug("%s: Configuring devices: START.", getLoggerPrefix());
    
    bool mainResult = true;
    bool result = false;
    
    result = ADS1248_initialize();
    mainResult = mainResult && result;
    notifyObserverAboutUnitReadyStatus(EUnitId_ADS1248, result);
    
    result = LMP90100ControlSystem_initialize();
    mainResult = mainResult && result;
    notifyObserverAboutUnitReadyStatus(EUnitId_LMP90100ControlSystem, result);
    
    result = LMP90100SignalsMeasurement_initialize();
    mainResult = mainResult && result;
    notifyObserverAboutUnitReadyStatus(EUnitId_LMP90100SignalsMeasurement, result);
    
    result = MCP4716_initialize();
    mainResult = mainResult && result;
    notifyObserverAboutUnitReadyStatus(EUnitId_MCP4716, result);
    
    if (mainResult)
    {
        Logger_debug("%s: Configuring devices: DONE!", getLoggerPrefix());
    }
    else
    {
        Logger_error("%s: Configuring devices: FAILURE!", getLoggerPrefix());
    }
    
    return mainResult;
}

void initializeCommunicationWithMaster(void)
{
    UART1_initializeDefault();
    
    MasterDataManager_initialize();
    MasterDataReceiver_initialize();
    MasterDataTransmitter_initialize();
    MasterUartGateway_initialize();
    
    KernelManager_startThread(mThreadId, EThreadId_MasterDataManager);
    KernelManager_startThread(mThreadId, EThreadId_MasterDataReceiver);
    KernelManager_startThread(mThreadId, EThreadId_MasterDataTransmitter);
    
    CREATE_EVENT(StartReceivingData, EThreadId_MasterDataReceiver);
    SEND_EVENT();
}

void notifyObserverAboutUnitReadyStatus(EUnitId unitId, bool result)
{
    if (mUnitReadyIndCallback)
    {
        Logger_debug("%s: Notifying observer about unit %s ready status (%s).", getLoggerPrefix(), CStringConverter_EUnitId(unitId), result ? "SUCCESS" : "FAILURE");
        (*mUnitReadyIndCallback)(unitId, result);
    }
}

bool configureControllers(void)
{
    Logger_debug("%s: Configuring controllers: START.", getLoggerPrefix());\
    
    HeaterTemperatureController_initialize();
    HeaterTemperatureReader_initialize();
    SampleCarrierDataManager_initialize();
    SegmentsManager_initialize();
    ReferenceTemperatureReader_initialize();
    ReferenceTemperatureController_initialize();
    
    Logger_debug("%s: Configuring controllers: DONE!", getLoggerPrefix());
    
    return true;
}

void startThreads(void)
{
    Logger_debug("%s: Starting threads: START.", getLoggerPrefix());
    
    KernelManager_startThread(mThreadId, EThreadId_ADS1248Controller);
    KernelManager_startThread(mThreadId, EThreadId_HeaterTemperatureReader);
    KernelManager_startThread(mThreadId, EThreadId_LMP90100ControlSystemController);
    KernelManager_startThread(mThreadId, EThreadId_LMP90100SignalsMeasurementController);
    KernelManager_startThread(mThreadId, EThreadId_ReferenceTemperatureReader);
    KernelManager_startThread(mThreadId, EThreadId_SampleCarrierDataManager);
    KernelManager_startThread(mThreadId, EThreadId_SampleThread);
    KernelManager_startThread(mThreadId, EThreadId_StaticSegmentProgramExecutor);
    
    Logger_debug("%s: Starting threads: DONE!", getLoggerPrefix());
}

const char* getLoggerPrefix(void)
{
    return "SystemManager";
}
