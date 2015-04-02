#ifndef _THREAD_MACROS_H_

#define _THREAD_MACROS_H_

#include "System/ThreadStarter.h"
#include "System/EventManagement/Event.h"
#include "System/EventManagement/EEventId.h"
#include "System/EventManagement/TEvent.h"
#include "System/EventManagement/TEventMessage.h"

#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"

#include "cmsis_os.h"
#include "stdbool.h"

#define THREAD_ID   osThreadId threadId;
#define THREAD_CREATE(name, priority, stackSize)    osThreadDef                                                                                         \
                                                    (                                                                                                   \
                                                        name##ThreadHandler,                                                                            \
                                                        name##_thread,                                                                                  \
                                                        osPriority##priority,                                                                           \
                                                        0,                                                                                              \
                                                        stackSize                                                                                       \
                                                    );                                                                                                  \
                                                    threadId = osThreadCreate(osThread(name##ThreadHandler), NULL);                                     \
                                                    KernelManager_registerThread(EThreadId_##name, threadId);                                           \
                                                    
#define THREAD(name)   void name##_thread(void const* arg)
#define THREAD_PROTOTYPE(name) THREAD(name);

#define THREAD_DEFINES(threadIdPostfix, loggerPrefix)       static osMutexDef(mMutex);                                                                  \
                                                            static osMutexId mMutexId = NULL;                                                           \
                                                            static EThreadId mThreadId = EThreadId_##threadIdPostfix;                                   \
                                                            static const char* getLoggerPrefix(void)                                                    \
                                                            {                                                                                           \
                                                                return ""#loggerPrefix"";                                                               \
                                                            }

#define THREAD_INITIALIZE_MUTEX             if (!mMutexId)                                                                                              \
                                            {                                                                                                           \
                                                mMutexId = osMutexCreate( osMutex(mMutex) );                                                            \
                                            }

#define THREAD_SKELETON_START   ThreadStarter_run(mThreadId);                                                                                           \
                                bool mIsThreadTerminated = false;                                                                                       \
                                while(!mIsThreadTerminated)                                                                                             \
                                {                                                                                                                       \
                                    TEvent* event = Event_wait(mThreadId, osWaitForever);                                                               \
                                    osMutexWait(mMutexId, osWaitForever);                                                                               \
                                    if (event)                                                                                                          \
                                    {                                                                                                                   \
                                        Logger_debugSystem                                                                                              \
                                        (                                                                                                               \
                                            "%s: Received event: %s from %s.",                                                                          \
                                            getLoggerPrefix(),                                                                                          \
                                            CStringConverter_EEventId(event->id),                                                                       \
                                            CStringConverter_EThreadId(event->sender)                                                                   \
                                        );                                                                                                              \
                                        switch (event->id)                                                                                              \
                                        {

#define THREAD_SKELETON_END                 case (EEventId_Stop) :                                                                                      \
                                            {                                                                                                           \
                                                Logger_warning("%s: Thread will be stopped. Sending ACK event to client.", getLoggerPrefix());          \
                                                TEvent* event = Event_calloc(event->sender, EEventId_StopAck);                                          \
                                                Event_send(event->sender, event);                                                                       \
                                                mIsThreadTerminated = true;                                                                             \
                                                break;                                                                                                  \
                                            }                                                                                                           \
                                            case (EEventId_Terminate) :                                                                                 \
                                            {                                                                                                           \
                                                Logger_error("%s: Thread unexpectly will be terminated without informing anyone!", getLoggerPrefix());  \
                                                mIsThreadTerminated = true;                                                                             \
                                                break;                                                                                                  \
                                            }                                                                                                           \
                                            default :                                                                                                   \
                                            {                                                                                                           \
                                                Logger_warning                                                                                          \
                                                (                                                                                                       \
                                                    "%s: Received unexpected event %s from %s. Event discarded!",                                       \
                                                    getLoggerPrefix(),                                                                                  \
                                                    CStringConverter_EEventId(event->id),                                                               \
                                                    CStringConverter_EThreadId(event->sender)                                                           \
                                                );                                                                                                      \
                                                break;                                                                                                  \
                                            }                                                                                                           \
                                        }                                                                                                               \
                                        Event_free(mThreadId, event);                                                                                   \
                                    }                                                                                                                   \
                                    else                                                                                                                \
                                    {                                                                                                                   \
                                        Logger_error("%s: Null event. Failure", getLoggerPrefix());                                                     \
                                    }                                                                                                                   \
                                    osMutexRelease(mMutexId);                                                                                           \
                                    osThreadYield();                                                                                                    \
                                }                                                                                                                       \
                                Logger_warning("%s: Thread TERMINATED!", getLoggerPrefix());                                                            \
                                osDelay(osWaitForever);

#define EVENT_HANDLER_NAME(eventId)  event##eventId##Handler
#define EVENT_HANDLER(eventId)    void EVENT_HANDLER_NAME(eventId)(TEvent* _event)
#define EVENT_HANDLER_PROTOTYPE(eventId)    static EVENT_HANDLER(eventId);
#define EVENT_MESSAGE(eventId)  TEventMessage##eventId *event = _event->data;
                                
#define EVENT_HANDLING(eventId)             case EEventId_##eventId :                                                                                              \
                                            {                                                                                                           \
                                                EVENT_HANDLER_NAME(eventId)(event);                                                                                             \
                                                break;                                                                                                  \
                                            }

#endif
