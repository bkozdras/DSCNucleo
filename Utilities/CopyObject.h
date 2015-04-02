#ifndef _COPY_OBJECT_H_

#define _COPY_OBJECT_H_

#include "SharedDefines/TMessage.h"
#include "SharedDefines/SFaultIndication.h"
#include "SharedDefines/SSampleCarrierData.h"

void CopyObject_TMessage(TMessage* source, TMessage* dest);
void CopyObject_SFaultIndication(SFaultIndication* source, SFaultIndication* dest);
void CopyObject_SSampleCarrierData(SSampleCarrierData* source, SSampleCarrierData* dest);

#endif
