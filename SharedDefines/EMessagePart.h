#ifndef _E_MESSAGE_PART_H_

#define _E_MESSAGE_PART_H_

typedef enum _EMessagePart
{
    EMessagePart_Header         = 0,
    EMessagePart_Data           = 1,
    EMessagePart_End            = 2,
    EMessagePart_Unknown        = 3
} EMessagePart;

#endif
