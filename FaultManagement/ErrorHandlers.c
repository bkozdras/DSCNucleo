#include "FaultManagement/ErrorHandlers.h"
#include "FaultManagement/FaultIndication.h"
#include "Utilities/Logger/Logger.h"

#include "Peripherals/LED.h"

#include "stdint.h"

static void shutdownNucleoAndWaitForRecovery(void);

#ifdef  USE_FULL_ASSERT

    void assert_failed(uint8_t* file, uint32_t line)
    {
        Logger_error("Assertion failed: File: %s, Line: %u.", file, line);
        FaultIndication_start(EFaultId_Assert, EUnitId_Nucleo, EUnitId_Empty);
        ErrorHandler_simpleHandler();
    }

#endif
   
void ErrorHandler_simpleHandler(void)
{
    LED_changeState(ELedState_On);
    shutdownNucleoAndWaitForRecovery();
}

void shutdownNucleoAndWaitForRecovery(void)
{
    // SHUT DOWN
    // WAITING FOR UNIT RESET
    while(true)
    {
        ;
    }   
}
