#include "Testing/SampleThread.h"

#include "System/ThreadStarter.h"

#include "System/EventManagement/Event.h"
#include "System/EventManagement/EEventId.h"
#include "System/EventManagement/TEvent.h"

#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"

#include "Devices/LMP90100ControlSystem.h"
#include "Devices/MCP4716.h"

#include "Peripherals/UART1.h"
#include "MasterCommunication/MasterUartGateway.h"
#include "MasterCommunication/MasterDataMemoryManager.h"
#include "SharedDefines/TMessage.h"
#include "SharedDefines/EMessageId.h"
#include "SharedDefines/SFaultIndication.h"
#include "SharedDefines/MessagesDefines.h"

#include "cmsis_os.h"

static const EThreadId mThreadId = EThreadId_SampleThread;
static bool mIsThreadNotTerminated = true;

//static void testMCP4716(void);
static const char* getLoggerPrefix(void);

void SampleThread_thread(void const* arg)
{
    ThreadStarter_run(mThreadId);
    
    //LMP90100ControlSystem_changeMode(ELMP90100Mode_On_1_6775_SPS); 
    
    /*
    osDelay(5000);
    
    LMP90100_changeMode(ELMP90100Mode_Off);
    
    osDelay(5000);
    
    LMP90100_changeMode(ELMP90100Mode_On_3_355_SPS);
    
    osDelay(5000);
    
    LMP90100_changeMode(ELMP90100Mode_On_1_6775_SPS);
    */
    
    Logger_debug("%s: Sending message.", getLoggerPrefix());
    TPollingResponse* pol = MasterDataMemoryManager_allocate(EMessageId_PollingResponse);
    pol->success = true;
    TFaultInd* faultInd = MasterDataMemoryManager_allocate(EMessageId_FaultInd);
    faultInd->indication.faultId = EFaultId_System;
    faultInd->indication.faultySubUnitId = EUnitId_Empty;
    faultInd->indication.faultyUnitId = EUnitId_Nucleo;
    faultInd->indication.state = EFaultIndicationState_Start;
    Logger_debug("%s: Sending message2.", getLoggerPrefix());
    MasterUartGateway_sendMessage(EMessageId_FaultInd, faultInd);
    MasterUartGateway_sendMessage(EMessageId_PollingResponse, pol);
    
    while (mIsThreadNotTerminated)
    {
        TEvent* event = Event_wait(mThreadId, osWaitForever);
    }
    
    Logger_warning("%s: Thread TERMINATED!", getLoggerPrefix());
    osDelay(osWaitForever);
}

/*
void testMCP4716(void)
{
    Logger_debug("%s: Testing MCP4716: Started!", getLoggerPrefix());
    
    u16 readValue = 0x00;
    u16 valueToSet = 0x00;
    
    MCP4716_setOutputVoltage(valueToSet);
    readValue = MCP4716_readOutputVoltage();
    Logger_debug("%s: Set Value: (%u)(%f V).", getLoggerPrefix(), valueToSet, MCP4716_convertOutputVoltageToRealData(valueToSet));
    Logger_debug("%s: Get Value: (%u)(%f V).", getLoggerPrefix(), readValue, MCP4716_convertOutputVoltageToRealData(readValue));
    
    for (u8 iter = 0; 10 > iter; ++iter)
    {
        osDelay(2000);
        
        valueToSet += 100U;
        
        MCP4716_setOutputVoltage(valueToSet);
        readValue = MCP4716_readOutputVoltage();
        
        Logger_debug("%s: Set Value: (%u)(%f V).", getLoggerPrefix(), valueToSet, MCP4716_convertOutputVoltageToRealData(valueToSet));
        Logger_debug("%s: Get Value: (%u)(%f V).", getLoggerPrefix(), readValue, MCP4716_convertOutputVoltageToRealData(readValue));
        
        if (valueToSet == readValue)
        {
            Logger_debug("%s: Testing MCP4716: Partial test PASSED!", getLoggerPrefix());
        }
        else
        {
            Logger_warning("%s: Testing MCP4716: Partial test FAILED!", getLoggerPrefix());
        }
    }
    
    valueToSet = 1023U;
    
    MCP4716_setOutputVoltage(valueToSet);
    readValue = MCP4716_getOutputVoltage();
    
    Logger_debug("%s: Set Value: (%u)(%f V).", getLoggerPrefix(), valueToSet, MCP4716_convertOutputVoltageToRealData(valueToSet));
    Logger_debug("%s: Get Value: (%u)(%f V).", getLoggerPrefix(), readValue, MCP4716_convertOutputVoltageToRealData(readValue));
    
    if (valueToSet == readValue)
    {
        Logger_debug("%s: Testing MCP4716: Partial test PASSED!", getLoggerPrefix());
    }
    else
    {
        Logger_warning("%s: Testing MCP4716: Partial test FAILED!", getLoggerPrefix());
    }
    
    Logger_debug("%s: Testing MCP4716: DONE!", getLoggerPrefix());
}*/

const char* getLoggerPrefix(void)
{
    static char* prefix = NULL;
    if (!prefix)
    {
        prefix = (char*)(CStringConverter_EThreadId(mThreadId));
    }
    return prefix;
}
