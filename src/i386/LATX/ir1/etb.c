#include "common.h"
#include "env.h"
#include "etb.h"

/* global etb_qht defined here */
QHT *etb_qht;

static bool etb_lookup_custom(const void *ap, const void *bp)
{
    ETB *etb = (ETB*)ap;
    ADDRX pc = *(ADDRX *)bp;
    return etb->pc == pc;
}

static bool etb_cmp(const void *ap, const void *bp)
{
    ETB *p = (ETB*)ap;
    ETB *q = (ETB*)bp;
    return p->pc == q->pc;
}

void etb_qht_init(void)
{
    unsigned int mode = QHT_MODE_AUTO_RESIZE;

    qht_init(etb_qht, etb_cmp, 1 << 10, mode);
}

static void etb_init(ETB *etb)
{
    memset(etb, 0, sizeof(ETB));
}

static ETB *fast_table[1 << 10];

ETB *etb_find(ADDRX pc)
{
    uint32_t hash = pc & 0x3ff;
    if (fast_table[hash] && fast_table[hash]->pc == pc)
        return fast_table[hash];

    ETB *etb = (ETB*)qht_lookup_custom(etb_qht, &pc, hash, etb_lookup_custom);
    if (etb == NULL) {
        etb = (ETB*)mm_malloc(sizeof(ETB));
        etb_init(etb);
        etb->pc = pc;
        lsassertm(etb,"memory is full\n");
        qht_insert(etb_qht, etb, hash, NULL);
    }
    fast_table[hash] = etb;
    return etb;
}
