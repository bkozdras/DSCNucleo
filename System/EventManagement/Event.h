#ifndef _EVENT_H_

#define _EVENT_H_

#include "cmsis_os.h"
#include "System/EventManagement/TEvent.h"
#include "System/EventManagement/EEventId.h"
#include "System/EventManagement/TEventMessage.h"
#include "System/EThreadId.h"
#include "Defines/CommonDefines.h"

#define OsEventId                                   osMessageQId

// Create Event From Thread
#define CREATE_EVENT(eventName, receiver)           TEvent* event = Event_calloc(receiver, EEventId_##eventName);       \
                                                    EThreadId _receiverThread = receiver;                               \
                                                    event->sender = mThreadId;
                                                    
// Create Event From ISR
#define CREATE_EVENT_ISR(eventName, receiver)       TEvent* event = Event_calloc(receiver, EEventId_##eventName);       \
                                                    EThreadId _receiverThread = receiver;                               \
                                                    event->sender = EThreadId_ISR;
                                                    
#define CREATE_EVENT_MESSAGE(eventName)             TEventMessage##eventName *eventMessage = event->data;
                                                   
#define SEND_EVENT()                                Event_send(_receiverThread, event)

void Event_setup(void);
OsEventId Event_getId(EThreadId threadId);
osStatus Event_send(EThreadId threadId, TEvent* event);
TEvent* Event_malloc(EThreadId threadId, EEventId eventId);
TEvent* Event_calloc(EThreadId threadId, EEventId eventId);
void Event_free(EThreadId threadId, TEvent* event);
TEvent* Event_wait(EThreadId threadId, u32 timeout);

#endif
