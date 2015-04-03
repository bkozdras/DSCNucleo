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
#include "Controller/SampleCarrierDataManager.h"
#include "Controller/ReferenceThermocoupleTemperatureReader.h"

#include "MasterCommunication/MasterDataManager.h"
#include "MasterCommunication/MasterDataMemoryManager.h"
#include "MasterCommunication/MasterDataReceiver.h"
#include "MasterCommunication/MasterDataTransmitter.h"
#include "MasterCommunication/MasterUartGateway.h"

#include "Testing/SampleThread.h"

#include "cmsis_os.h"

static const EThreadId mThreadId = EThreadId_SystemManager;

static void SystemManager_thread(void const* arg);
static void setup(void);
static void createThreads(void);
static void checkKernelStatus(void);
static void configurePeripheralsPhaseOne(void);
static void configurePeripheralsPhaseTwo(void);
static void configureDevices(void);
static void configureControllers(void);
static void initializeCommunicationWithMaster(void);
static void startThreads(void);
static const char* getLoggerPrefix(void);

void SystemManager_run(void)
{
    setup();
    createThreads();
    osKernelStart();
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
    TIM3_setup();
    
    ADS1248_setup();
    LMP90100ControlSystem_setup();
    LMP90100SignalsMeasurement_setup();
    MCP4716_setup();
    
    HeaterTemperatureReader_setup();
    SampleCarrierDataManager_setup();
    ReferenceThermocoupleTemperatureReader_setup();
    
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
    THREAD_CREATE(ReferenceThermocoupleTemperatureReader, Low, configMINIMAL_STACK_SIZE);
    THREAD_CREATE(MasterDataManager, Low, configNORMAL_STACK_SIZE);
    THREAD_CREATE(MasterDataReceiver, Low, configNORMAL_STACK_SIZE);
    THREAD_CREATE(MasterDataTransmitter, Low, configMAXIMUM_STACK_SIZE);
    THREAD_CREATE(SampleThread, Normal, configMINIMAL_STACK_SIZE);
}

void SystemManager_thread(void const* arg)
{
    Logger_initialize(ELoggerType_EvalCOM1, ELoggerLevel_DebugSystem);
    initializeCommunicationWithMaster();
    //Logger_initialize(ELoggerType_EvalCOM1, ELoggerLevel_Debug);
    
    Logger_info("%s: THREAD STARTED!", getLoggerPrefix());
    
    checkKernelStatus();
    
    Logger_info("%s: Started configuring device!", getLoggerPrefix());
    
    configurePeripheralsPhaseOne();
    configureDevices();
    configureControllers();
    startThreads();
    configurePeripheralsPhaseTwo();
    
    LED_changeState(ELedState_On);
    Logger_info("%s: Device CONFIGURED!", getLoggerPrefix());  
    
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

void configurePeripheralsPhaseOne(void)
{
    Logger_debug("%s: Configuring peripherals (Phase: 1/2): START.", getLoggerPrefix());
    
    LED_initialize();
    I2C1_initialize();
    SPI2_initialize();
    SPI3_initialize();
    
    if (UART2_isInitialized())
    {
        Logger_info("UART2: Initialized!");
    }
    else
    {
        Logger_info("UART2: Not initialized!");
    }
    
    Logger_debug("%s: Configuring peripherals (Phase: 1/2): DONE!", getLoggerPrefix());
}

void configurePeripheralsPhaseTwo(void)
{
    Logger_debug("%s: Configuring peripherals (Phase: 2/2): START.", getLoggerPrefix());
    
    //EXTI_initialize(EExtiType_EXTI4);
    //EXTI_initialize(EExtiType_EXTI9_5);
    //EXTI_initialize(EExtiType_EXTI15_10);
    
    Logger_debug("%s: Configuring peripherals (Phase: 2/2): DONE!", getLoggerPrefix());
}

void configureDevices(void)
{
    Logger_debug("%s: Configuring devices: START.", getLoggerPrefix());
    
    //ADS1248_initialize();
    //LMP90100ControlSystem_initialize();
    //LMP90100SignalsMeasurement_initialize();
    //MCP4716_initialize();
    
    Logger_debug("%s: Configuring devices: DONE!", getLoggerPrefix());
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

void configureControllers(void)
{
    Logger_debug("%s: Configuring controllers: START.", getLoggerPrefix());\
    
    HeaterTemperatureReader_initialize();
    SampleCarrierDataManager_initialize();
    ReferenceThermocoupleTemperatureReader_initialize();
    
    Logger_debug("%s: Configuring controllers: DONE!", getLoggerPrefix());
}

void startThreads(void)
{
    Logger_debug("%s: Starting threads: START.", getLoggerPrefix());
    
    KernelManager_startThread(mThreadId, EThreadId_ADS1248Controller);
    KernelManager_startThread(mThreadId, EThreadId_HeaterTemperatureReader);
    KernelManager_startThread(mThreadId, EThreadId_LMP90100ControlSystemController);
    KernelManager_startThread(mThreadId, EThreadId_LMP90100SignalsMeasurementController);
    KernelManager_startThread(mThreadId, EThreadId_ReferenceThermocoupleTemperatureReader);
    KernelManager_startThread(mThreadId, EThreadId_SampleCarrierDataManager);
    KernelManager_startThread(mThreadId, EThreadId_SampleThread);
    
    Logger_debug("%s: Starting threads: DONE!", getLoggerPrefix());
}

const char* getLoggerPrefix(void)
{
    return "SystemManager";
}
