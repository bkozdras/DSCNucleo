#include "Utilities/CopyObject.h"

void CopyObject_TMessage(TMessage* source, TMessage* dest)
{
    dest->data = source->data;
    dest->id = source->id;
    dest->length = source->length;
    dest->transactionId = source->transactionId;
}
