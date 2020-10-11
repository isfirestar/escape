#if !defined ESCAPE_PROTO_H
#define ESCAPE_PROTO_H

#include <stdint.h>

#pragma pack(push, 1)

struct escape_head
{
    int seq;
    int64_t tx_timestamp;
    unsigned char context[0];
};

#pragma pack(pop)

#endif
