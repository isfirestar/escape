#if !defined ESCAPE_PROTO_H
#define ESCAPE_PROTO_H

#include "compiler.h"

#pragma pack(push, 1)

struct escape_head
{
    int seq;
    int64_t tx_timestamp;
    unsigned char context[0];
};

struct nsp_head {
    uint32_t op;
    uint32_t cb;
};

#pragma pack(pop)

extern int STDCALL tst_parser(void *dat, int cb, int *pkt_cb);
extern int STDCALL tst_builder(void *dat, int cb);

#endif
