#if 0
#include "include/profile.h"
#include "ir1/ir1.h"
#include "include/etb.h"
#include "include/common.h"

#define PROFILE_HELPER_NUM 7

static ETB **unique_tb = NULL;
static IR1_OPCODE *unique_opcode = NULL;
static int unique_tb_num = 0;
static int unique_opcode_num = 0;

static int64 tb_executed = 0;
static int64 tb_executed_by_type[TB_TYPE_JUMPIN-TB_TYPE_NONE+1];
static int64 ir1_inst_executed = 0;
static int64 ir1_inst_executed_by_opcode[X86_INS_ENDING];
static int64 ir1_inst_executed_by_group[X86_GRP_ENDING] = {0};
static int64 ir2_inst_executed_zx_address = 0;
int64 indirect_block_cnt = 0;
int64 ibtc_hit_cnt = 0;
int64 context_switch_time = 0;
int64 fpu_call_helper_times = 0; /* aggregate */
int64 fpu_call_xxx_times[PROFILE_HELPER_NUM];
ETB *etb_array[ETB_ARRAY_SIZE];
int tb_num = 0;

static inline void profile_sum(void) {
    for (int i=0; i<tb_num; ++i) {
        ETB *etb = etb_array[i];

        tb_executed += etb->_execution_times;
        tb_executed_by_type[etb->_tb_type-TB_TYPE_NONE] += etb->_execution_times;
        ir1_inst_executed += etb->_ir1_num * etb->_execution_times;

        for (int j=0; j<etb->_ir1_num; ++j) {
            IR1_INST *pir1 = etb->_ir1_instructions + j;
            ir1_inst_executed_by_opcode[ir1_opcode(pir1)] += etb->_execution_times;


            if (pir1->info->detail->groups_count) {
                int j;
                for(j = 0; j < pir1->info->detail->groups_count; j++) {
                    ir1_inst_executed_by_group[pir1->info->detail->groups[j]] += etb->_execution_times;
                }
        	}

            /* profile fpu call helpers */
            switch (ir1_opcode(pir1)) {
            case X86_INS_FPREM:
                fpu_call_xxx_times[0] += etb->_execution_times;
                break;
            case X86_INS_FPREM1:
                fpu_call_xxx_times[1] += etb->_execution_times;
                break;
            case X86_INS_FRNDINT:
                fpu_call_xxx_times[2] += etb->_execution_times;
                break;
            case X86_INS_FSCALE:
                fpu_call_xxx_times[3] += etb->_execution_times;
                break;
            case X86_INS_FXAM:
                fpu_call_xxx_times[4] += etb->_execution_times;
                break;
            case X86_INS_F2XM1:
                fpu_call_xxx_times[5] += etb->_execution_times;
                break;
            case X86_INS_FXTRACT:
                fpu_call_xxx_times[6] += etb->_execution_times;
                break;
            default:
                break;
            }

            if (BITS_ARE_SET(pir1->flags, FI_ZX_ADDR_USED))
                ir2_inst_executed_zx_address += etb->_execution_times;
        }
    }

    indirect_block_cnt = tb_executed_by_type[TB_TYPE_CALLIN-TB_TYPE_NONE] +
                         tb_executed_by_type[TB_TYPE_JUMPIN-TB_TYPE_NONE];
    for (int i = 0; i < PROFILE_HELPER_NUM; i++) {
        fpu_call_helper_times += fpu_call_xxx_times[i];
    }
}


static inline int cmp_tb_exec_count(const void *etb1, const void *etb2) {
    int64 c1 = (*(const ETB **)etb1)->_execution_times;
    int64 c2 = (*(const ETB **)etb2)->_execution_times;
    if (c2 > c1) return 1;
    else if(c2 == c1) return 0;
    else return -1;
}

static inline int cmp_opcode_exec_count(const void *op1, const void *op2) {
    int64 c1 = ir1_inst_executed_by_opcode[*(const IR1_OPCODE*)op1];
    int64 c2 = ir1_inst_executed_by_opcode[*(const IR1_OPCODE*)op2];
    if (c2 > c1) return 1;
    else if(c2 == c1) return 0;
    else return -1;
}

static inline int cmp_tb_type(const void *op1, const void *op2) {
    int64 c1 = tb_executed_by_type[*(int*)op1];
    int64 c2 = tb_executed_by_type[*(int*)op2];
    if (c2 > c1) return 1;
    else if(c2 == c1) return 0;
    else return -1;
}

static inline int cmp_fpu_call_helper_count(const void *op1, const void *op2) {
    int64 c1 = fpu_call_xxx_times[*(int*)op1];
    int64 c2 = fpu_call_xxx_times[*(int*)op2];
    if (c2 > c1) return 1;
    else if (c2 == c1) return 0;
    else return -1;
}

static inline void profile_sort_tb(void) {
    if (unique_tb != NULL)
        mm_free(unique_tb);
    unique_tb = (ETB **)mm_calloc(tb_num, sizeof(ETB *));
    int n = 0;

    for (int i=0; i<tb_num; ++i) {
        ETB *etb = etb_array[i];
        if (etb->_execution_times > 10)
            unique_tb[n++] = etb;
    }

    unique_tb = (ETB **) mm_realloc(unique_tb, sizeof(ETB *)*n);
    unique_tb_num = n;

    qsort(unique_tb, n, sizeof(ETB*), cmp_tb_exec_count);
}


static inline void profile_sort_opcode(void) {
    if (unique_opcode != NULL)
        mm_free(unique_opcode);
    unique_opcode = (IR1_OPCODE *)mm_calloc(X86_INS_ENDING, sizeof(IR1_OPCODE));
    int n = 0;

    for (int i=0; i<X86_INS_ENDING; ++i) {
        if (ir1_inst_executed_by_opcode[i] > 10)
            unique_opcode[n++] = (IR1_OPCODE)i;
    }
    unique_opcode = (IR1_OPCODE*) mm_realloc(unique_opcode, sizeof(IR1_OPCODE)*n);
    unique_opcode_num = n;

    qsort(unique_opcode, n, sizeof(IR1_OPCODE), cmp_opcode_exec_count);
}

static inline void profile_dump_number(void) {
    fprintf(stderr, "*************************************\n");
    fprintf(stderr, "context_switch_times:%" PRId64 "\n", context_switch_time);
    fprintf(stderr, "indirect_block_cnt:%" PRId64 "\n", indirect_block_cnt);
    fprintf(stderr, "ibtc_hit_cnt:%" PRId64 "\n", ibtc_hit_cnt);
    fprintf(stderr, "ibtc_hit_rate:%5.2f%%\n", ibtc_hit_cnt*100.0/indirect_block_cnt);
    fprintf(stderr, "fpu_call_helper_times:%" PRId64 "\n", fpu_call_helper_times);
    fprintf(stderr, "*************************************\n");
}

static inline void profile_dump_tb_type(void) {
    static const char* tb_type_to_string[] = {
        "NONE", /* include syscall, split tb(tb is too long) */
        "BRANCH",
        "CALL",
        "RETURN",
        "JUMP",
        "CALLIN",
        "JUMPIN"
    };
    int num = TB_TYPE_JUMPIN - TB_TYPE_NONE + 1;
    int *rank = (int*) mm_malloc(sizeof(int)*num);
    for (int i=0; i<num; i++)
        rank[i] = i;
    qsort(rank, num, sizeof(int), cmp_tb_type);

    fprintf(stderr, " TB types rank are:\n");
    fprintf(stderr, " rank    tb_type  exec_count percentage \n");
    for (int i=0; i<num; i++) {
        int index = rank[i];
        fprintf(stderr, " [%2d] %10s %11" PRId64 "     %5.2f%% \n",
                i+1, tb_type_to_string[index], tb_executed_by_type[index],
                tb_executed_by_type[index]*100.0/tb_executed);
    }
}

static inline void profile_dump_fpu_call_helper(void) {
    static const char* helper_to_string[] = {
        "FPREM",
        "FPREM1",
        "FRNDINT",
        "FSCALE",
        "FXAM",
        "F2XM1",
        "FXTRACT"
    };

    int rank[PROFILE_HELPER_NUM] = {0, };
    for (int i = 0; i < PROFILE_HELPER_NUM; i++)
        rank[i] = i;
    qsort(rank, PROFILE_HELPER_NUM, sizeof(int), cmp_fpu_call_helper_count);

    fprintf(stderr, " Helpers calling rank are:\n");
    fprintf(stderr, " rank     helper  exec_count percentage \n");
    for (int i = 0; i < PROFILE_HELPER_NUM; i++) {
        int index = rank[i];
        fprintf(stderr, " [%2d] %10s %11" PRId64 "     %5.2f%% \n",
                i+1, helper_to_string[index], fpu_call_xxx_times[index],
                fpu_call_xxx_times[index]*100.0/fpu_call_helper_times);
    }
}


static inline void profile_dump_tb(int top_n) {
    if (top_n >unique_tb_num)
        top_n = unique_tb_num;
    fprintf(stderr, "%d TB execute %" PRId64 " times, and top %d are: \n", tb_num, tb_executed, top_n);

    fprintf(stderr, " rank ir1_num ir2_num  exec_count percentage x86_addr <function>\n");
    for (int i=0; i<top_n; ++i) {
        ETB *etb = unique_tb[i];
        fprintf(stderr, " [%2d] %7d %7ld %11" PRId64 "     %5.2f%% %8" PRIADDRX " %s\n",
            i+1, etb->_ir1_num, etb->tb->tc.size/4, etb->_execution_times,
            etb->_execution_times*100.0/tb_executed, (uint32_t)etb->pc, lookup_symbol(etb->tb->pc));
    }

    //for (int i=0; i<top_n; ++i) {
    //    ETB *ptb = unique_tb[i];
    //    ptb->dump();
    //}
}

static inline void profile_dump_opcode(int top_n) {
    if (top_n >unique_opcode_num)
        top_n = unique_opcode_num;
    fprintf(stderr, "%" PRId64 " x86 instructions are executed, and top %d are: \n", ir1_inst_executed, top_n);
    fprintf(stderr, " rank     opcode  exec_count percentage \n");
    for (int i=0; i<top_n; ++i) {
        int opc = unique_opcode[i];
        fprintf(stderr, " [%2d] %10s %11" PRId64 "     %5.2f%%\n",
            i+1, ir1_name(opc), ir1_inst_executed_by_opcode[opc],
            ir1_inst_executed_by_opcode[opc]*100.0/ir1_inst_executed);
    }
}

static inline void profile_dump_group(void)
{
    fprintf(stderr, "group profile(one ins multi group): \n");
    fprintf(stderr, "           group   exec_count \n");
    for (int i = X86_GRP_JUMP; i <= X86_GRP_BRANCH_RELATIVE; ++i) {
        fprintf(stderr, "%16s %11" PRId64 "\n", ir1_group_name(i),
                ir1_inst_executed_by_group[i]);
    }
    for (int i = X86_GRP_VM; i < X86_GRP_ENDING; ++i) {
        fprintf(stderr, "%16s %11" PRId64 "\n", ir1_group_name(i),
                ir1_inst_executed_by_group[i]);
    }
}

void profile_generate(void) {
    profile_sum();
    profile_sort_tb();
    profile_sort_opcode();
}

void profile_dump(int top_n) {
    profile_dump_number();
    profile_dump_opcode(top_n);
    profile_dump_group();
    profile_dump_tb_type();
    profile_dump_fpu_call_helper();
    profile_dump_tb(top_n);
}

void xtm_pf_inc_jc_clear(void *cpu) {}
#endif
