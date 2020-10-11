#if !defined ESCAPE_TST_H
#define ESCAPE_TST_H

#include "compiler.h"

typedef struct {
    uint32_t op;
    uint32_t cb;
} head_t;

extern int STDCALL tst_parser(void *dat, int cb, int *pkt_cb);
extern int STDCALL tst_builder(void *dat, int cb);

#endif
