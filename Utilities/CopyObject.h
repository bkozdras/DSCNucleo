#ifndef _COPY_OBJECT_H_

#define _COPY_OBJECT_H_

#include "SharedDefines/TMessage.h"
#include "SharedDefines/SFaultIndication.h"
#include "SharedDefines/SSampleCarrierData.h"
#include "SharedDefines/SPidTunes.h"
#include "SharedDefines/SControllerData.h"
#include "SharedDefines/SSegmentData.h"

void CopyObject_TMessage(TMessage* source, TMessage* dest);
void CopyObject_SFaultIndication(SFaultIndication* source, SFaultIndication* dest);
void CopyObject_SSampleCarrierData(SSampleCarrierData* source, SSampleCarrierData* dest);
void CopyObject_SPidTunes(SPidTunes* source, SPidTunes* dest);
void CopyObject_SControllerData(SControllerData* source, SControllerData* dest);
void CopyObject_SSegmentData(SSegmentData* source, SSegmentData* dest);

#endif
