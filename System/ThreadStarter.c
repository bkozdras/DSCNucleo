#include "System/ThreadStarter.h"

#include "System/EventManagement/Event.h"
#include "System/EventManagement/EEventId.h"
#include "System/EventManagement/TEvent.h"

#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"

#include "cmsis_os.h"

void ThreadStarter_run(EThreadId threadId)
{
    const char* loggerPrefix = CStringConverter_EThreadId(threadId);
    
    Logger_debug("%s: Waiting for starting...", loggerPrefix);
    
    TEvent* event = Event_wait(threadId, osWaitForever);
    if (event)
    {
        EThreadId client = event->sender;
        EEventId eventId = event->id;
        Event_free(threadId, event);
        
        if (EEventId_Start == eventId)
        {
            Logger_debug("%s: Received event: %s from thread: %s.", loggerPrefix, CStringConverter_EEventId(eventId), CStringConverter_EThreadId(client));
        }
        else
        {
            Logger_error("%s: Wrong event type (%s) received from %s! Thread not started.", loggerPrefix, CStringConverter_EThreadId(client), CStringConverter_EEventId(eventId));
            osDelay(osWaitForever);
        }
        
        TEvent* response = Event_calloc(client, EEventId_StartAck);
        response->sender = threadId;
        Event_send(client, response);
        
        Logger_info("%s: THREAD STARTED!", loggerPrefix);
    }
    else
    {
        Logger_error("%s: Starting event not received! Thread not started.", loggerPrefix);
        osDelay(osWaitForever);
    }
}
