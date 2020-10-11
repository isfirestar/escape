#include "tst.h"

static const unsigned char NSPDEF_OPCODE[4] = {'N', 's', 'p', 'd'};

int STDCALL tst_parser(void *dat, int cb, int *pkt_cb)
{
    head_t *head = (head_t *) dat;

    if (!head) return -1;

    if (0 != memcmp(NSPDEF_OPCODE, &head->op, sizeof (NSPDEF_OPCODE))) {
        return -1;
    }

    *pkt_cb = head->cb;
    return 0;
}

int STDCALL tst_builder(void *dat, int cb)
{
    head_t *head = (head_t *) dat;

    if (!dat || cb <= 0) {
        return -1;
    }

    memcpy(&head->op, NSPDEF_OPCODE, sizeof (NSPDEF_OPCODE));
    head->cb = cb;
    return 0;
}
