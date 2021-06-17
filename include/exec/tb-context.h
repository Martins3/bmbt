#ifndef TB_CONTEXT_H_GOOTFXVY
#define TB_CONTEXT_H_GOOTFXVY


#define CODE_GEN_HTABLE_BITS     15
#define CODE_GEN_HTABLE_SIZE     (1 << CODE_GEN_HTABLE_BITS)

typedef struct TBContext TBContext;

extern TBContext tb_ctx;

struct TBContext {

    struct qht htable;

    /* statistics */
    unsigned tb_flush_count;
};


#endif /* end of include guard: TB_CONTEXT_H_GOOTFXVY */
