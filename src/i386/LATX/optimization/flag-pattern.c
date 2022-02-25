#include "lsenv.h"
#include "etb.h"
#include "reg-alloc.h"
#include "translate.h"
#include "flag-reduction.h"
#include "latx-options.h"
#include <string.h>
#include "flag-pattern.h"

#ifdef CONFIG_LATX_FLAG_PATTERN
typedef enum {
    CMP_JO = (X86_INS_CMP << 16) + X86_INS_JO,
    CMP_JNO = (X86_INS_CMP << 16) + X86_INS_JNO,
    CMP_JB = (X86_INS_CMP << 16) + X86_INS_JB,
    CMP_JAE = (X86_INS_CMP << 16) + X86_INS_JAE,
    CMP_JZ = (X86_INS_CMP << 16) + X86_INS_JE,
    CMP_JNZ = (X86_INS_CMP << 16) + X86_INS_JNE,
    CMP_JBE = (X86_INS_CMP << 16) + X86_INS_JBE,
    CMP_JA = (X86_INS_CMP << 16) + X86_INS_JA,
    CMP_JS = (X86_INS_CMP << 16) + X86_INS_JS,
    CMP_JNS = (X86_INS_CMP << 16) + X86_INS_JNS,
    CMP_JP = (X86_INS_CMP << 16) + X86_INS_JP,
    CMP_JNP = (X86_INS_CMP << 16) + X86_INS_JNP,
    CMP_JL = (X86_INS_CMP << 16) + X86_INS_JL,
    CMP_JGE = (X86_INS_CMP << 16) + X86_INS_JGE,
    CMP_JLE = (X86_INS_CMP << 16) + X86_INS_JLE,
    CMP_JG = (X86_INS_CMP << 16) + X86_INS_JG,
    CMP_CMOVO = (X86_INS_CMP << 16) + X86_INS_CMOVO,
    CMP_CMOVNO = (X86_INS_CMP << 16) + X86_INS_CMOVNO,
    CMP_CMOVB = (X86_INS_CMP << 16) + X86_INS_CMOVB,
    CMP_CMOVAE = (X86_INS_CMP << 16) + X86_INS_CMOVAE,
    CMP_CMOVZ = (X86_INS_CMP << 16) + X86_INS_CMOVE,
    CMP_CMOVNZ = (X86_INS_CMP << 16) + X86_INS_CMOVNE,
    CMP_CMOVBE = (X86_INS_CMP << 16) + X86_INS_CMOVBE,
    CMP_CMOVA = (X86_INS_CMP << 16) + X86_INS_CMOVA,
    CMP_CMOVS = (X86_INS_CMP << 16) + X86_INS_CMOVS,
    CMP_CMOVNS = (X86_INS_CMP << 16) + X86_INS_CMOVNS,
    CMP_CMOVP = (X86_INS_CMP << 16) + X86_INS_CMOVP,
    CMP_CMOVNP = (X86_INS_CMP << 16) + X86_INS_CMOVNP,
    CMP_CMOVL = (X86_INS_CMP << 16) + X86_INS_CMOVL,
    CMP_CMOVGE = (X86_INS_CMP << 16) + X86_INS_CMOVGE,
    CMP_CMOVLE = (X86_INS_CMP << 16) + X86_INS_CMOVLE,
    CMP_CMOVG = (X86_INS_CMP << 16) + X86_INS_CMOVG,
    CMP_SETO = (X86_INS_CMP << 16) + X86_INS_SETO,
    CMP_SETNO = (X86_INS_CMP << 16) + X86_INS_SETNO,
    CMP_SETB = (X86_INS_CMP << 16) + X86_INS_SETB,
    CMP_SETAE = (X86_INS_CMP << 16) + X86_INS_SETAE,
    CMP_SETZ = (X86_INS_CMP << 16) + X86_INS_SETE,
    CMP_SETNZ = (X86_INS_CMP << 16) + X86_INS_SETNE,
    CMP_SETBE = (X86_INS_CMP << 16) + X86_INS_SETBE,
    CMP_SETA = (X86_INS_CMP << 16) + X86_INS_SETA,
    CMP_SETS = (X86_INS_CMP << 16) + X86_INS_SETS,
    CMP_SETNS = (X86_INS_CMP << 16) + X86_INS_SETNS,
    CMP_SETP = (X86_INS_CMP << 16) + X86_INS_SETP,
    CMP_SETNP = (X86_INS_CMP << 16) + X86_INS_SETNP,
    CMP_SETL = (X86_INS_CMP << 16) + X86_INS_SETL,
    CMP_SETGE = (X86_INS_CMP << 16) + X86_INS_SETGE,
    CMP_SETLE = (X86_INS_CMP << 16) + X86_INS_SETLE,
    CMP_SETG = (X86_INS_CMP << 16) + X86_INS_SETG,
    TEST_JO = (X86_INS_TEST << 16) + X86_INS_JO,
    TEST_JNO = (X86_INS_TEST << 16) + X86_INS_JNO,
    TEST_JB = (X86_INS_TEST << 16) + X86_INS_JB,
    TEST_JAE = (X86_INS_TEST << 16) + X86_INS_JAE,
    TEST_JZ = (X86_INS_TEST << 16) + X86_INS_JE,
    TEST_JNZ = (X86_INS_TEST << 16) + X86_INS_JNE,
    TEST_JBE = (X86_INS_TEST << 16) + X86_INS_JBE,
    TEST_JA = (X86_INS_TEST << 16) + X86_INS_JA,
    TEST_JS = (X86_INS_TEST << 16) + X86_INS_JS,
    TEST_JNS = (X86_INS_TEST << 16) + X86_INS_JNS,
    TEST_JP = (X86_INS_TEST << 16) + X86_INS_JP,
    TEST_JNP = (X86_INS_TEST << 16) + X86_INS_JNP,
    TEST_JL = (X86_INS_TEST << 16) + X86_INS_JL,
    TEST_JGE = (X86_INS_TEST << 16) + X86_INS_JGE,
    TEST_JLE = (X86_INS_TEST << 16) + X86_INS_JLE,
    TEST_JG = (X86_INS_TEST << 16) + X86_INS_JG,
    TEST_CMOVO = (X86_INS_TEST << 16) + X86_INS_CMOVO,
    TEST_CMOVNO = (X86_INS_TEST << 16) + X86_INS_CMOVNO,
    TEST_CMOVB = (X86_INS_TEST << 16) + X86_INS_CMOVB,
    TEST_CMOVAE = (X86_INS_TEST << 16) + X86_INS_CMOVAE,
    TEST_CMOVZ = (X86_INS_TEST << 16) + X86_INS_CMOVE,
    TEST_CMOVNZ = (X86_INS_TEST << 16) + X86_INS_CMOVNE,
    TEST_CMOVBE = (X86_INS_TEST << 16) + X86_INS_CMOVBE,
    TEST_CMOVA = (X86_INS_TEST << 16) + X86_INS_CMOVA,
    TEST_CMOVS = (X86_INS_TEST << 16) + X86_INS_CMOVS,
    TEST_CMOVNS = (X86_INS_TEST << 16) + X86_INS_CMOVNS,
    TEST_CMOVP = (X86_INS_TEST << 16) + X86_INS_CMOVP,
    TEST_CMOVNP = (X86_INS_TEST << 16) + X86_INS_CMOVNP,
    TEST_CMOVL = (X86_INS_TEST << 16) + X86_INS_CMOVL,
    TEST_CMOVGE = (X86_INS_TEST << 16) + X86_INS_CMOVGE,
    TEST_CMOVLE = (X86_INS_TEST << 16) + X86_INS_CMOVLE,
    TEST_CMOVG = (X86_INS_TEST << 16) + X86_INS_CMOVG,
    TEST_SETO = (X86_INS_TEST << 16) + X86_INS_SETO,
    TEST_SETNO = (X86_INS_TEST << 16) + X86_INS_SETNO,
    TEST_SETB = (X86_INS_TEST << 16) + X86_INS_SETB,
    TEST_SETAE = (X86_INS_TEST << 16) + X86_INS_SETAE,
    TEST_SETZ = (X86_INS_TEST << 16) + X86_INS_SETE,
    TEST_SETNZ = (X86_INS_TEST << 16) + X86_INS_SETNE,
    TEST_SETBE = (X86_INS_TEST << 16) + X86_INS_SETBE,
    TEST_SETA = (X86_INS_TEST << 16) + X86_INS_SETA,
    TEST_SETS = (X86_INS_TEST << 16) + X86_INS_SETS,
    TEST_SETNS = (X86_INS_TEST << 16) + X86_INS_SETNS,
    TEST_SETP = (X86_INS_TEST << 16) + X86_INS_SETP,
    TEST_SETNP = (X86_INS_TEST << 16) + X86_INS_SETNP,
    TEST_SETL = (X86_INS_TEST << 16) + X86_INS_SETL,
    TEST_SETGE = (X86_INS_TEST << 16) + X86_INS_SETGE,
    TEST_SETLE = (X86_INS_TEST << 16) + X86_INS_SETLE,
    TEST_SETG = (X86_INS_TEST << 16) + X86_INS_SETG,
    OR_JO = (X86_INS_OR << 16) + X86_INS_JO,
    OR_JNO = (X86_INS_OR << 16) + X86_INS_JNO,
    OR_JB = (X86_INS_OR << 16) + X86_INS_JB,
    OR_JAE = (X86_INS_OR << 16) + X86_INS_JAE,
    OR_JZ = (X86_INS_OR << 16) + X86_INS_JE,
    OR_JNZ = (X86_INS_OR << 16) + X86_INS_JNE,
    OR_JBE = (X86_INS_OR << 16) + X86_INS_JBE,
    OR_JA = (X86_INS_OR << 16) + X86_INS_JA,
    OR_JS = (X86_INS_OR << 16) + X86_INS_JS,
    OR_JNS = (X86_INS_OR << 16) + X86_INS_JNS,
    OR_JP = (X86_INS_OR << 16) + X86_INS_JP,
    OR_JNP = (X86_INS_OR << 16) + X86_INS_JNP,
    OR_JL = (X86_INS_OR << 16) + X86_INS_JL,
    OR_JGE = (X86_INS_OR << 16) + X86_INS_JGE,
    OR_JLE = (X86_INS_OR << 16) + X86_INS_JLE,
    OR_JG = (X86_INS_OR << 16) + X86_INS_JG,
    OR_CMOVO = (X86_INS_OR << 16) + X86_INS_CMOVO,
    OR_CMOVNO = (X86_INS_OR << 16) + X86_INS_CMOVNO,
    OR_CMOVB = (X86_INS_OR << 16) + X86_INS_CMOVB,
    OR_CMOVAE = (X86_INS_OR << 16) + X86_INS_CMOVAE,
    OR_CMOVZ = (X86_INS_OR << 16) + X86_INS_CMOVE,
    OR_CMOVNZ = (X86_INS_OR << 16) + X86_INS_CMOVNE,
    OR_CMOVBE = (X86_INS_OR << 16) + X86_INS_CMOVBE,
    OR_CMOVA = (X86_INS_OR << 16) + X86_INS_CMOVA,
    OR_CMOVS = (X86_INS_OR << 16) + X86_INS_CMOVS,
    OR_CMOVNS = (X86_INS_OR << 16) + X86_INS_CMOVNS,
    OR_CMOVP = (X86_INS_OR << 16) + X86_INS_CMOVP,
    OR_CMOVNP = (X86_INS_OR << 16) + X86_INS_CMOVNP,
    OR_CMOVL = (X86_INS_OR << 16) + X86_INS_CMOVL,
    OR_CMOVGE = (X86_INS_OR << 16) + X86_INS_CMOVGE,
    OR_CMOVLE = (X86_INS_OR << 16) + X86_INS_CMOVLE,
    OR_CMOVG = (X86_INS_OR << 16) + X86_INS_CMOVG,
    OR_SETO = (X86_INS_OR << 16) + X86_INS_SETO,
    OR_SETNO = (X86_INS_OR << 16) + X86_INS_SETNO,
    OR_SETB = (X86_INS_OR << 16) + X86_INS_SETB,
    OR_SETAE = (X86_INS_OR << 16) + X86_INS_SETAE,
    OR_SETZ = (X86_INS_OR << 16) + X86_INS_SETE,
    OR_SETNZ = (X86_INS_OR << 16) + X86_INS_SETNE,
    OR_SETBE = (X86_INS_OR << 16) + X86_INS_SETBE,
    OR_SETA = (X86_INS_OR << 16) + X86_INS_SETA,
    OR_SETS = (X86_INS_OR << 16) + X86_INS_SETS,
    OR_SETNS = (X86_INS_OR << 16) + X86_INS_SETNS,
    OR_SETP = (X86_INS_OR << 16) + X86_INS_SETP,
    OR_SETNP = (X86_INS_OR << 16) + X86_INS_SETNP,
    OR_SETL = (X86_INS_OR << 16) + X86_INS_SETL,
    OR_SETGE = (X86_INS_OR << 16) + X86_INS_SETGE,
    OR_SETLE = (X86_INS_OR << 16) + X86_INS_SETLE,
    OR_SETG = (X86_INS_OR << 16) + X86_INS_SETG,
    SUB_JO = (X86_INS_SUB << 16) + X86_INS_JO,
    SUB_JNO = (X86_INS_SUB << 16) + X86_INS_JNO,
    SUB_JB = (X86_INS_SUB << 16) + X86_INS_JB,
    SUB_JAE = (X86_INS_SUB << 16) + X86_INS_JAE,
    SUB_JZ = (X86_INS_SUB << 16) + X86_INS_JE,
    SUB_JNZ = (X86_INS_SUB << 16) + X86_INS_JNE,
    SUB_JBE = (X86_INS_SUB << 16) + X86_INS_JBE,
    SUB_JA = (X86_INS_SUB << 16) + X86_INS_JA,
    SUB_JS = (X86_INS_SUB << 16) + X86_INS_JS,
    SUB_JNS = (X86_INS_SUB << 16) + X86_INS_JNS,
    SUB_JP = (X86_INS_SUB << 16) + X86_INS_JP,
    SUB_JNP = (X86_INS_SUB << 16) + X86_INS_JNP,
    SUB_JL = (X86_INS_SUB << 16) + X86_INS_JL,
    SUB_JGE = (X86_INS_SUB << 16) + X86_INS_JGE,
    SUB_JLE = (X86_INS_SUB << 16) + X86_INS_JLE,
    SUB_JG = (X86_INS_SUB << 16) + X86_INS_JG,
    SUB_CMOVO = (X86_INS_SUB << 16) + X86_INS_CMOVO,
    SUB_CMOVNO = (X86_INS_SUB << 16) + X86_INS_CMOVNO,
    SUB_CMOVB = (X86_INS_SUB << 16) + X86_INS_CMOVB,
    SUB_CMOVAE = (X86_INS_SUB << 16) + X86_INS_CMOVAE,
    SUB_CMOVZ = (X86_INS_SUB << 16) + X86_INS_CMOVE,
    SUB_CMOVNZ = (X86_INS_SUB << 16) + X86_INS_CMOVNE,
    SUB_CMOVBE = (X86_INS_SUB << 16) + X86_INS_CMOVBE,
    SUB_CMOVA = (X86_INS_SUB << 16) + X86_INS_CMOVA,
    SUB_CMOVS = (X86_INS_SUB << 16) + X86_INS_CMOVS,
    SUB_CMOVNS = (X86_INS_SUB << 16) + X86_INS_CMOVNS,
    SUB_CMOVP = (X86_INS_SUB << 16) + X86_INS_CMOVP,
    SUB_CMOVNP = (X86_INS_SUB << 16) + X86_INS_CMOVNP,
    SUB_CMOVL = (X86_INS_SUB << 16) + X86_INS_CMOVL,
    SUB_CMOVGE = (X86_INS_SUB << 16) + X86_INS_CMOVGE,
    SUB_CMOVLE = (X86_INS_SUB << 16) + X86_INS_CMOVLE,
    SUB_CMOVG = (X86_INS_SUB << 16) + X86_INS_CMOVG,
    SUB_SETO = (X86_INS_SUB << 16) + X86_INS_SETO,
    SUB_SETNO = (X86_INS_SUB << 16) + X86_INS_SETNO,
    SUB_SETB = (X86_INS_SUB << 16) + X86_INS_SETB,
    SUB_SETAE = (X86_INS_SUB << 16) + X86_INS_SETAE,
    SUB_SETZ = (X86_INS_SUB << 16) + X86_INS_SETE,
    SUB_SETNZ = (X86_INS_SUB << 16) + X86_INS_SETNE,
    SUB_SETBE = (X86_INS_SUB << 16) + X86_INS_SETBE,
    SUB_SETA = (X86_INS_SUB << 16) + X86_INS_SETA,
    SUB_SETS = (X86_INS_SUB << 16) + X86_INS_SETS,
    SUB_SETNS = (X86_INS_SUB << 16) + X86_INS_SETNS,
    SUB_SETP = (X86_INS_SUB << 16) + X86_INS_SETP,
    SUB_SETNP = (X86_INS_SUB << 16) + X86_INS_SETNP,
    SUB_SETL = (X86_INS_SUB << 16) + X86_INS_SETL,
    SUB_SETGE = (X86_INS_SUB << 16) + X86_INS_SETGE,
    SUB_SETLE = (X86_INS_SUB << 16) + X86_INS_SETLE,
    SUB_SETG = (X86_INS_SUB << 16) + X86_INS_SETG,
    AND_JO = (X86_INS_AND << 16) + X86_INS_JO,
    AND_JNO = (X86_INS_AND << 16) + X86_INS_JNO,
    AND_JB = (X86_INS_AND << 16) + X86_INS_JB,
    AND_JAE = (X86_INS_AND << 16) + X86_INS_JAE,
    AND_JZ = (X86_INS_AND << 16) + X86_INS_JE,
    AND_JNZ = (X86_INS_AND << 16) + X86_INS_JNE,
    AND_JBE = (X86_INS_AND << 16) + X86_INS_JBE,
    AND_JA = (X86_INS_AND << 16) + X86_INS_JA,
    AND_JS = (X86_INS_AND << 16) + X86_INS_JS,
    AND_JNS = (X86_INS_AND << 16) + X86_INS_JNS,
    AND_JP = (X86_INS_AND << 16) + X86_INS_JP,
    AND_JNP = (X86_INS_AND << 16) + X86_INS_JNP,
    AND_JL = (X86_INS_AND << 16) + X86_INS_JL,
    AND_JGE = (X86_INS_AND << 16) + X86_INS_JGE,
    AND_JLE = (X86_INS_AND << 16) + X86_INS_JLE,
    AND_JG = (X86_INS_AND << 16) + X86_INS_JG,
    AND_CMOVO = (X86_INS_AND << 16) + X86_INS_CMOVO,
    AND_CMOVNO = (X86_INS_AND << 16) + X86_INS_CMOVNO,
    AND_CMOVB = (X86_INS_AND << 16) + X86_INS_CMOVB,
    AND_CMOVAE = (X86_INS_AND << 16) + X86_INS_CMOVAE,
    AND_CMOVZ = (X86_INS_AND << 16) + X86_INS_CMOVE,
    AND_CMOVNZ = (X86_INS_AND << 16) + X86_INS_CMOVNE,
    AND_CMOVBE = (X86_INS_AND << 16) + X86_INS_CMOVBE,
    AND_CMOVA = (X86_INS_AND << 16) + X86_INS_CMOVA,
    AND_CMOVS = (X86_INS_AND << 16) + X86_INS_CMOVS,
    AND_CMOVNS = (X86_INS_AND << 16) + X86_INS_CMOVNS,
    AND_CMOVP = (X86_INS_AND << 16) + X86_INS_CMOVP,
    AND_CMOVNP = (X86_INS_AND << 16) + X86_INS_CMOVNP,
    AND_CMOVL = (X86_INS_AND << 16) + X86_INS_CMOVL,
    AND_CMOVGE = (X86_INS_AND << 16) + X86_INS_CMOVGE,
    AND_CMOVLE = (X86_INS_AND << 16) + X86_INS_CMOVLE,
    AND_CMOVG = (X86_INS_AND << 16) + X86_INS_CMOVG,
    AND_SETO = (X86_INS_AND << 16) + X86_INS_SETO,
    AND_SETNO = (X86_INS_AND << 16) + X86_INS_SETNO,
    AND_SETB = (X86_INS_AND << 16) + X86_INS_SETB,
    AND_SETAE = (X86_INS_AND << 16) + X86_INS_SETAE,
    AND_SETZ = (X86_INS_AND << 16) + X86_INS_SETE,
    AND_SETNZ = (X86_INS_AND << 16) + X86_INS_SETNE,
    AND_SETBE = (X86_INS_AND << 16) + X86_INS_SETBE,
    AND_SETA = (X86_INS_AND << 16) + X86_INS_SETA,
    AND_SETS = (X86_INS_AND << 16) + X86_INS_SETS,
    AND_SETNS = (X86_INS_AND << 16) + X86_INS_SETNS,
    AND_SETP = (X86_INS_AND << 16) + X86_INS_SETP,
    AND_SETNP = (X86_INS_AND << 16) + X86_INS_SETNP,
    AND_SETL = (X86_INS_AND << 16) + X86_INS_SETL,
    AND_SETGE = (X86_INS_AND << 16) + X86_INS_SETGE,
    AND_SETLE = (X86_INS_AND << 16) + X86_INS_SETLE,
    AND_SETG = (X86_INS_AND << 16) + X86_INS_SETG,
    INC_JO = (X86_INS_INC << 16) + X86_INS_JO,
    INC_JNO = (X86_INS_INC << 16) + X86_INS_JNO,
    INC_JB = (X86_INS_INC << 16) + X86_INS_JB,
    INC_JAE = (X86_INS_INC << 16) + X86_INS_JAE,
    INC_JZ = (X86_INS_INC << 16) + X86_INS_JE,
    INC_JNZ = (X86_INS_INC << 16) + X86_INS_JNE,
    INC_JBE = (X86_INS_INC << 16) + X86_INS_JBE,
    INC_JA = (X86_INS_INC << 16) + X86_INS_JA,
    INC_JS = (X86_INS_INC << 16) + X86_INS_JS,
    INC_JNS = (X86_INS_INC << 16) + X86_INS_JNS,
    INC_JP = (X86_INS_INC << 16) + X86_INS_JP,
    INC_JNP = (X86_INS_INC << 16) + X86_INS_JNP,
    INC_JL = (X86_INS_INC << 16) + X86_INS_JL,
    INC_JGE = (X86_INS_INC << 16) + X86_INS_JGE,
    INC_JLE = (X86_INS_INC << 16) + X86_INS_JLE,
    INC_JG = (X86_INS_INC << 16) + X86_INS_JG,
    INC_CMOVO = (X86_INS_INC << 16) + X86_INS_CMOVO,
    INC_CMOVNO = (X86_INS_INC << 16) + X86_INS_CMOVNO,
    INC_CMOVB = (X86_INS_INC << 16) + X86_INS_CMOVB,
    INC_CMOVAE = (X86_INS_INC << 16) + X86_INS_CMOVAE,
    INC_CMOVZ = (X86_INS_INC << 16) + X86_INS_CMOVE,
    INC_CMOVNZ = (X86_INS_INC << 16) + X86_INS_CMOVNE,
    INC_CMOVBE = (X86_INS_INC << 16) + X86_INS_CMOVBE,
    INC_CMOVA = (X86_INS_INC << 16) + X86_INS_CMOVA,
    INC_CMOVS = (X86_INS_INC << 16) + X86_INS_CMOVS,
    INC_CMOVNS = (X86_INS_INC << 16) + X86_INS_CMOVNS,
    INC_CMOVP = (X86_INS_INC << 16) + X86_INS_CMOVP,
    INC_CMOVNP = (X86_INS_INC << 16) + X86_INS_CMOVNP,
    INC_CMOVL = (X86_INS_INC << 16) + X86_INS_CMOVL,
    INC_CMOVGE = (X86_INS_INC << 16) + X86_INS_CMOVGE,
    INC_CMOVLE = (X86_INS_INC << 16) + X86_INS_CMOVLE,
    INC_CMOVG = (X86_INS_INC << 16) + X86_INS_CMOVG,
    INC_SETO = (X86_INS_INC << 16) + X86_INS_SETO,
    INC_SETNO = (X86_INS_INC << 16) + X86_INS_SETNO,
    INC_SETB = (X86_INS_INC << 16) + X86_INS_SETB,
    INC_SETAE = (X86_INS_INC << 16) + X86_INS_SETAE,
    INC_SETZ = (X86_INS_INC << 16) + X86_INS_SETE,
    INC_SETNZ = (X86_INS_INC << 16) + X86_INS_SETNE,
    INC_SETBE = (X86_INS_INC << 16) + X86_INS_SETBE,
    INC_SETA = (X86_INS_INC << 16) + X86_INS_SETA,
    INC_SETS = (X86_INS_INC << 16) + X86_INS_SETS,
    INC_SETNS = (X86_INS_INC << 16) + X86_INS_SETNS,
    INC_SETP = (X86_INS_INC << 16) + X86_INS_SETP,
    INC_SETNP = (X86_INS_INC << 16) + X86_INS_SETNP,
    INC_SETL = (X86_INS_INC << 16) + X86_INS_SETL,
    INC_SETGE = (X86_INS_INC << 16) + X86_INS_SETGE,
    INC_SETLE = (X86_INS_INC << 16) + X86_INS_SETLE,
    INC_SETG = (X86_INS_INC << 16) + X86_INS_SETG,
    DEC_JO = (X86_INS_DEC << 16) + X86_INS_JO,
    DEC_JNO = (X86_INS_DEC << 16) + X86_INS_JNO,
    DEC_JB = (X86_INS_DEC << 16) + X86_INS_JB,
    DEC_JAE = (X86_INS_DEC << 16) + X86_INS_JAE,
    DEC_JZ = (X86_INS_DEC << 16) + X86_INS_JE,
    DEC_JNZ = (X86_INS_DEC << 16) + X86_INS_JNE,
    DEC_JBE = (X86_INS_DEC << 16) + X86_INS_JBE,
    DEC_JA = (X86_INS_DEC << 16) + X86_INS_JA,
    DEC_JS = (X86_INS_DEC << 16) + X86_INS_JS,
    DEC_JNS = (X86_INS_DEC << 16) + X86_INS_JNS,
    DEC_JP = (X86_INS_DEC << 16) + X86_INS_JP,
    DEC_JNP = (X86_INS_DEC << 16) + X86_INS_JNP,
    DEC_JL = (X86_INS_DEC << 16) + X86_INS_JL,
    DEC_JGE = (X86_INS_DEC << 16) + X86_INS_JGE,
    DEC_JLE = (X86_INS_DEC << 16) + X86_INS_JLE,
    DEC_JG = (X86_INS_DEC << 16) + X86_INS_JG,
    DEC_CMOVO = (X86_INS_DEC << 16) + X86_INS_CMOVO,
    DEC_CMOVNO = (X86_INS_DEC << 16) + X86_INS_CMOVNO,
    DEC_CMOVB = (X86_INS_DEC << 16) + X86_INS_CMOVB,
    DEC_CMOVAE = (X86_INS_DEC << 16) + X86_INS_CMOVAE,
    DEC_CMOVZ = (X86_INS_DEC << 16) + X86_INS_CMOVE,
    DEC_CMOVNZ = (X86_INS_DEC << 16) + X86_INS_CMOVNE,
    DEC_CMOVBE = (X86_INS_DEC << 16) + X86_INS_CMOVBE,
    DEC_CMOVA = (X86_INS_DEC << 16) + X86_INS_CMOVA,
    DEC_CMOVS = (X86_INS_DEC << 16) + X86_INS_CMOVS,
    DEC_CMOVNS = (X86_INS_DEC << 16) + X86_INS_CMOVNS,
    DEC_CMOVP = (X86_INS_DEC << 16) + X86_INS_CMOVP,
    DEC_CMOVNP = (X86_INS_DEC << 16) + X86_INS_CMOVNP,
    DEC_CMOVL = (X86_INS_DEC << 16) + X86_INS_CMOVL,
    DEC_CMOVGE = (X86_INS_DEC << 16) + X86_INS_CMOVGE,
    DEC_CMOVLE = (X86_INS_DEC << 16) + X86_INS_CMOVLE,
    DEC_CMOVG = (X86_INS_DEC << 16) + X86_INS_CMOVG,
    DEC_SETO = (X86_INS_DEC << 16) + X86_INS_SETO,
    DEC_SETNO = (X86_INS_DEC << 16) + X86_INS_SETNO,
    DEC_SETB = (X86_INS_DEC << 16) + X86_INS_SETB,
    DEC_SETAE = (X86_INS_DEC << 16) + X86_INS_SETAE,
    DEC_SETZ = (X86_INS_DEC << 16) + X86_INS_SETE,
    DEC_SETNZ = (X86_INS_DEC << 16) + X86_INS_SETNE,
    DEC_SETBE = (X86_INS_DEC << 16) + X86_INS_SETBE,
    DEC_SETA = (X86_INS_DEC << 16) + X86_INS_SETA,
    DEC_SETS = (X86_INS_DEC << 16) + X86_INS_SETS,
    DEC_SETNS = (X86_INS_DEC << 16) + X86_INS_SETNS,
    DEC_SETP = (X86_INS_DEC << 16) + X86_INS_SETP,
    DEC_SETNP = (X86_INS_DEC << 16) + X86_INS_SETNP,
    DEC_SETL = (X86_INS_DEC << 16) + X86_INS_SETL,
    DEC_SETGE = (X86_INS_DEC << 16) + X86_INS_SETGE,
    DEC_SETLE = (X86_INS_DEC << 16) + X86_INS_SETLE,
    DEC_SETG = (X86_INS_DEC << 16) + X86_INS_SETG,
    ADD_JO = (X86_INS_ADD << 16) + X86_INS_JO,
    ADD_JNO = (X86_INS_ADD << 16) + X86_INS_JNO,
    ADD_JB = (X86_INS_ADD << 16) + X86_INS_JB,
    ADD_JAE = (X86_INS_ADD << 16) + X86_INS_JAE,
    ADD_JZ = (X86_INS_ADD << 16) + X86_INS_JE,
    ADD_JNZ = (X86_INS_ADD << 16) + X86_INS_JNE,
    ADD_JBE = (X86_INS_ADD << 16) + X86_INS_JBE,
    ADD_JA = (X86_INS_ADD << 16) + X86_INS_JA,
    ADD_JS = (X86_INS_ADD << 16) + X86_INS_JS,
    ADD_JNS = (X86_INS_ADD << 16) + X86_INS_JNS,
    ADD_JP = (X86_INS_ADD << 16) + X86_INS_JP,
    ADD_JNP = (X86_INS_ADD << 16) + X86_INS_JNP,
    ADD_JL = (X86_INS_ADD << 16) + X86_INS_JL,
    ADD_JGE = (X86_INS_ADD << 16) + X86_INS_JGE,
    ADD_JLE = (X86_INS_ADD << 16) + X86_INS_JLE,
    ADD_JG = (X86_INS_ADD << 16) + X86_INS_JG,
    ADD_CMOVO = (X86_INS_ADD << 16) + X86_INS_CMOVO,
    ADD_CMOVNO = (X86_INS_ADD << 16) + X86_INS_CMOVNO,
    ADD_CMOVB = (X86_INS_ADD << 16) + X86_INS_CMOVB,
    ADD_CMOVAE = (X86_INS_ADD << 16) + X86_INS_CMOVAE,
    ADD_CMOVZ = (X86_INS_ADD << 16) + X86_INS_CMOVE,
    ADD_CMOVNZ = (X86_INS_ADD << 16) + X86_INS_CMOVNE,
    ADD_CMOVBE = (X86_INS_ADD << 16) + X86_INS_CMOVBE,
    ADD_CMOVA = (X86_INS_ADD << 16) + X86_INS_CMOVA,
    ADD_CMOVS = (X86_INS_ADD << 16) + X86_INS_CMOVS,
    ADD_CMOVNS = (X86_INS_ADD << 16) + X86_INS_CMOVNS,
    ADD_CMOVP = (X86_INS_ADD << 16) + X86_INS_CMOVP,
    ADD_CMOVNP = (X86_INS_ADD << 16) + X86_INS_CMOVNP,
    ADD_CMOVL = (X86_INS_ADD << 16) + X86_INS_CMOVL,
    ADD_CMOVGE = (X86_INS_ADD << 16) + X86_INS_CMOVGE,
    ADD_CMOVLE = (X86_INS_ADD << 16) + X86_INS_CMOVLE,
    ADD_CMOVG = (X86_INS_ADD << 16) + X86_INS_CMOVG,
    ADD_SETO = (X86_INS_ADD << 16) + X86_INS_SETO,
    ADD_SETNO = (X86_INS_ADD << 16) + X86_INS_SETNO,
    ADD_SETB = (X86_INS_ADD << 16) + X86_INS_SETB,
    ADD_SETAE = (X86_INS_ADD << 16) + X86_INS_SETAE,
    ADD_SETZ = (X86_INS_ADD << 16) + X86_INS_SETE,
    ADD_SETNZ = (X86_INS_ADD << 16) + X86_INS_SETNE,
    ADD_SETBE = (X86_INS_ADD << 16) + X86_INS_SETBE,
    ADD_SETA = (X86_INS_ADD << 16) + X86_INS_SETA,
    ADD_SETS = (X86_INS_ADD << 16) + X86_INS_SETS,
    ADD_SETNS = (X86_INS_ADD << 16) + X86_INS_SETNS,
    ADD_SETP = (X86_INS_ADD << 16) + X86_INS_SETP,
    ADD_SETNP = (X86_INS_ADD << 16) + X86_INS_SETNP,
    ADD_SETL = (X86_INS_ADD << 16) + X86_INS_SETL,
    ADD_SETGE = (X86_INS_ADD << 16) + X86_INS_SETGE,
    ADD_SETLE = (X86_INS_ADD << 16) + X86_INS_SETLE,
    ADD_SETG = (X86_INS_ADD << 16) + X86_INS_SETG,
} FP_MODE;

void fp_init(void)
{
    memset(lsenv->fp_data->is_head_or_tail, 0,
           sizeof(int8) * MAX_IR1_NUM_PER_TB);
    memset(lsenv->fp_data->pattern_items, 0,
           sizeof(FLAG_PATTERN_ITEM) * MAX_PATTERN_HEAD_NUM_PER_TB);
    lsenv->fp_data->pattern_items_num = 1;
}
static bool fpi_is_head_op(IR1_OPCODE op)
{
    if (op == X86_INS_CMP || op == X86_INS_SUB || op == X86_INS_TEST || op == X86_INS_OR ||
        op == X86_INS_AND || op == X86_INS_DEC || op == X86_INS_ADD || op == X86_INS_INC)
        return true;
    else
        return false;
}

static bool fpi_is_tail_op(IR1_OPCODE op)
{
    //(op >= x86_jo && op <= x86_jg) || (op >= x86_cmovo && op <= x86_cmovg) || (op >= x86_seto && op <= x86_setg))
    switch (op)
    {
    case X86_INS_JO:
    case X86_INS_JNO:
    case X86_INS_JB:
    case X86_INS_JAE:
    case X86_INS_JE:
    case X86_INS_JNE:
    case X86_INS_JBE:
    case X86_INS_JA:
    case X86_INS_JS:
    case X86_INS_JNS:
    case X86_INS_JP:
    case X86_INS_JNP:
    case X86_INS_JL:
    case X86_INS_JGE:
    case X86_INS_JLE:
    case X86_INS_JG:
    case X86_INS_CMOVO:
    case X86_INS_CMOVNO:
    case X86_INS_CMOVB:
    case X86_INS_CMOVAE:
    case X86_INS_CMOVE:
    case X86_INS_CMOVNE:
    case X86_INS_CMOVBE:
    case X86_INS_CMOVA:
    case X86_INS_CMOVS:
    case X86_INS_CMOVNS:
    case X86_INS_CMOVP:
    case X86_INS_CMOVNP:
    case X86_INS_CMOVL:
    case X86_INS_CMOVGE:
    case X86_INS_CMOVLE:
    case X86_INS_CMOVG:
    case X86_INS_SETO:
    case X86_INS_SETNO:
    case X86_INS_SETB:
    case X86_INS_SETAE:
    case X86_INS_SETE:
    case X86_INS_SETNE:
    case X86_INS_SETBE:
    case X86_INS_SETA:
    case X86_INS_SETS:
    case X86_INS_SETNS:
    case X86_INS_SETP:
    case X86_INS_SETNP:
    case X86_INS_SETL:
    case X86_INS_SETGE:
    case X86_INS_SETLE:
    case X86_INS_SETG:
        return true;
    default:
        return false;
    }
}

static bool fpi_is_helper_op(IR1_OPCODE op)
{
    switch(op)
    {
        case X86_INS_FSIN:
        case X86_INS_FCOS:
        case X86_INS_FPATAN:
        case X86_INS_FXTRACT:
        case X86_INS_FYL2X:
        case X86_INS_FYL2XP1:
        case X86_INS_FSINCOS:

        case X86_INS_INT:
        case X86_INS_CPUID:

        case X86_INS_FPREM:
        case X86_INS_FPREM1:
        case X86_INS_FRNDINT:
        case X86_INS_FSCALE:
        case X86_INS_FXAM:
        case X86_INS_F2XM1:
        case X86_INS_FPTAN:
        case X86_INS_FLDCW:
        case X86_INS_LDMXCSR:
        case X86_INS_FNINIT:
        case X86_INS_FLDENV:
          return true;
        default:
          return false;
    }
}

static int fpi_get_pattern_item_index(IR1_INST *pir1)
{
    int pir1_index =
        pir1 - tb_ir1_inst_first(lsenv->tr_data->curr_tb);

    int pattern_item_index = lsenv->fp_data->is_head_or_tail[pir1_index];
    return pattern_item_index;
}

static IR1_INST *fpi_find_pattern_head_from_tail(IR1_INST *tail,
                                                 IR1_INST *pir1_first)
{
    /* 1. if not a tail opcode, return */
    if (!fpi_is_tail_op(ir1_opcode(tail)))
        return NULL;

    /* 2. scan the previous instructions for the head */
    const uint8 tail_use = ir1_get_eflag_use(tail);
    for (IR1_INST *head = tail - 1; head >= pir1_first; head--) {
        if (fpi_is_helper_op(ir1_opcode(head)))
            return NULL;

        uint8 head_def = ir1_get_eflag_def(head);
        if (head_def == 0)
            continue; /* head does not define any eflag */

        uint8 intersection_usedef = tail_use & head_def;
        if (intersection_usedef == 0)
            continue; /* head defines some eflag, but those eflags do not */
                      /* affect tail */
        if (intersection_usedef != tail_use)
            return NULL; /* head defines some eflag that tail uses, but not all
                          */

        if (!fpi_is_head_op(ir1_opcode(head)))
            return NULL;
        return head;
    }

    return NULL;
}

static bool fpi_record_pattern_item(IR1_INST *head, IR1_INST *tail)
{
    FLAG_PATTERN_DATA *fp_data = lsenv->fp_data;
    int pattern_item_index = fpi_get_pattern_item_index(head);
    FLAG_PATTERN_ITEM *pattern_item =
        fp_data->pattern_items + pattern_item_index;

    struct TranslationBlock *tb = lsenv->tr_data->curr_tb;
    lsassert(tb != NULL);

    /* 1. if head info does not exist, build a new one */
    if (pattern_item_index == 0) {
        pattern_item_index = fp_data->pattern_items_num++;
        lsassert(pattern_item_index < MAX_PATTERN_HEAD_NUM_PER_TB);
        pattern_item = fp_data->pattern_items + pattern_item_index;

        /* record head info */
        pattern_item->head = head;
        int head_index = head - tb_ir1_inst_first(tb);
        lsassert(head_index >= 0 && head_index < etb_ir1_num(tb));
        fp_data->is_head_or_tail[head_index] = pattern_item_index;
    }

    /* 2. record tail in pattern_item */
    bool record_finish = false;
    for (int i = 0; i < MAX_PATTERN_TAIL_NUM_PER_HEAD; ++i) {
        if (pattern_item->tails[i] == NULL) {
            /* record tail info */
            pattern_item->tails[i] = tail;
            int tail_index = tail - tb_ir1_inst_first(tb);
            lsassert(tail_index >= 0 && tail_index < etb_ir1_num(tb));
            fp_data->is_head_or_tail[tail_index] = pattern_item_index;

            record_finish = true;
            break;
        }
    }

    return record_finish;
}

static void fpi_adjust_pattern_head_skipped_eflags(void *tb,
                                                   const int pattern_item_index)
{
    FLAG_PATTERN_ITEM *item =
        lsenv->fp_data->pattern_items + pattern_item_index;

    /* 1. in most cases, the eflags that the tails use can be skipped */
    uint8 eflags_used_by_tails = 0;
    for (int i = 0; i < MAX_PATTERN_TAIL_NUM_PER_HEAD; ++i) {
        if (item->tails[i] == NULL)
            break;
        eflags_used_by_tails |= ir1_get_eflag_use(item->tails[i]);
    }

    bool is_save_src = true;
    IR1_OPCODE ir1_op = ir1_opcode(item->head);

    /* 2. determine save dest or source */
    if ((ir1_op == X86_INS_TEST) || (ir1_op == X86_INS_AND) ||
        (ir1_op == X86_INS_OR)) /* test,and,or should save dest opnd */
        is_save_src = false;
    else if (BITS_ARE_SET_ANY(
                 eflags_used_by_tails,
                 CF_USEDEF_BIT | OF_USEDEF_BIT)) /* head is not test/and/or,
                                                    tail use CF or */
        /* OF, save the source operands */
        is_save_src = true;
    else /* head is not test/and/or, tail does not use CF or OF, save dest opnd
          */
        is_save_src = false;

    if (is_save_src) {
        item->src0_reg_num = -1;
        item->src1_reg_num = -1;
    } else {
        item->dest_reg_num = -1;
    }

    /* 3. flag reduction again, but ignore the pattern tails */
    uint8 pending_use = pending_use_of_succ(tb, MAX_DEPTH);
    for (IR1_INST *pir1 = tb_ir1_inst_last(tb); pir1 != item->head; --pir1) {
        /* 3.1 pending use may be satisfied by this instruction */
        pending_use &= (~(ir1_get_eflag_def(pir1)));
        if (ir1_prefix(pir1) != 0 &&
            (ir1_opcode(pir1) == X86_INS_CMPSB ||
             ir1_opcode(pir1) == X86_INS_CMPSW ||
             ir1_opcode(pir1) == X86_INS_CMPSD ||
             ir1_opcode(pir1) == X86_INS_SCASB ||
             ir1_opcode(pir1) == X86_INS_SCASW ||
             ir1_opcode(pir1) == X86_INS_SCASD ||
             ir1_opcode(pir1) == X86_INS_SCASQ)
           )
            pending_use |=
                ir1_get_eflag_def(pir1); /* "rep cmps" may not define eflags */

        /* 3.2 this instruction may generate new pending use. (note that pattern
         */
        /* tails are ignored) */
        if (fpi_get_pattern_item_index(pir1) != pattern_item_index)
            pending_use |= ir1_get_eflag_use(pir1);
    }

    /* 5. set the value into pattern_item */
    item->skipped_flags = ir1_get_eflag_def(item->head) & (~pending_use);
}

static bool fpi_mode_is_implemented(FP_MODE mode)
{
    switch (mode) {
    case CMP_JA:
    case SUB_JA:
    case CMP_JBE:
    case SUB_JBE:
    case CMP_JB:
    case SUB_JB:
    case CMP_JAE:
    case SUB_JAE:

    case CMP_JG:
    case SUB_JG:
    case CMP_JLE:
    case SUB_JLE:
    case CMP_JL:
    case SUB_JL:
    case CMP_JGE:
    case SUB_JGE:

    case CMP_JZ:
    case SUB_JZ:
    case CMP_JNZ:
    case SUB_JNZ:

    case CMP_JS:
    case SUB_JS:
    case CMP_JNS:
    case SUB_JNS:

    case TEST_JZ:
    case OR_JZ:
    case AND_JZ:
    case DEC_JZ:
    case ADD_JZ:
    case INC_JZ:

    case TEST_JNZ:
    case OR_JNZ:
    case AND_JNZ:
    case DEC_JNZ:
    case ADD_JNZ:
    case INC_JNZ:

    case TEST_JS:
    case OR_JS:
    case AND_JS:

    case TEST_JNS:
    case OR_JNS:
    case AND_JNS:
    case ADD_JNS:

    case TEST_JG:
    case OR_JG:
    case AND_JG:
    case TEST_JLE:
    case OR_JLE:
    case AND_JLE:
    case TEST_JL:
    case OR_JL:
    case AND_JL:
    case TEST_JGE:
    case OR_JGE:
    case AND_JGE:

    case ADD_JAE:
    case ADD_JB:

    case CMP_CMOVA:
    case SUB_CMOVA:
    case CMP_CMOVBE:
    case SUB_CMOVBE:
    case CMP_CMOVB:
    case SUB_CMOVB:
    case CMP_CMOVAE:
    case SUB_CMOVAE:

    case CMP_CMOVG:
    case SUB_CMOVG:
    case CMP_CMOVLE:
    case SUB_CMOVLE:
    case CMP_CMOVL:
    case SUB_CMOVL:
    case CMP_CMOVGE:
    case SUB_CMOVGE:

    case CMP_CMOVZ:
    case SUB_CMOVZ:
    case CMP_CMOVNZ:
    case SUB_CMOVNZ:

    case CMP_CMOVS:
    case SUB_CMOVS:
    case CMP_CMOVNS:
    case SUB_CMOVNS:

    case TEST_CMOVZ:
    case OR_CMOVZ:
    case AND_CMOVZ:
    case TEST_CMOVNZ:
    case OR_CMOVNZ:
    case AND_CMOVNZ:

    case TEST_CMOVS:
    case OR_CMOVS:
    case AND_CMOVS:
    case TEST_CMOVNS:
    case OR_CMOVNS:
    case AND_CMOVNS:

    case CMP_SETA:
    case SUB_SETA:
    case CMP_SETBE:
    case SUB_SETBE:
    case CMP_SETB:
    case SUB_SETB:
    case CMP_SETAE:
    case SUB_SETAE:

    case CMP_SETG:
    case SUB_SETG:
    case CMP_SETLE:
    case SUB_SETLE:
    case CMP_SETL:
    case SUB_SETL:
    case CMP_SETGE:
    case SUB_SETGE:

    case CMP_SETZ:
    case SUB_SETZ:
    case CMP_SETNZ:
    case SUB_SETNZ:

    case TEST_SETZ:
    case OR_SETZ:
    case AND_SETZ:
    case TEST_SETNZ:
    case OR_SETNZ:
    case AND_SETNZ:

        return true;
    default:
        /* env->tr_data->curr_tb->dump(); */
        /* fprintf(stderr, "flag pattern %s_%s not implemented\n\n", */
        /* ir1_name(mode>>16), ir1_name(mode&0xffff)); */
        return false;
    }
}

void tb_find_flag_pattern(void *tb)
{
    if (!option_flag_pattern)
        return;

    IR1_INST *pir1_first = tb_ir1_inst_first(tb);

    IR1_INST *prehead = tb_ir1_inst_last(tb) + 1;
    for (IR1_INST *tail = tb_ir1_inst_last(tb); tail != pir1_first; --tail) {
        if (ir1_get_eflag_use(tail) == 0)
            continue;

        IR1_INST *head = fpi_find_pattern_head_from_tail(tail, pir1_first);
        if (head == NULL ||
            !(prehead == head || (prehead > head && prehead > tail)))
            continue;

        FP_MODE mode = (FP_MODE)((ir1_opcode(head) << 16) + ir1_opcode(tail));
        if (fpi_mode_is_implemented(mode)) {
            fpi_record_pattern_item(head, tail);
            prehead = head;
        }
    }

    for (int i = 1; i < lsenv->fp_data->pattern_items_num; ++i) {
        fpi_adjust_pattern_head_skipped_eflags(tb, i);
    }
    /* fprintf(stderr, "0x%x fp: fp_skipped_flags=0x%x\n",this->addr(), */
    /* this->fp_skipped_flags() ); */
}

void fp_save_src_opnd(IR1_INST *pir1, IR2_OPND src0, IR2_OPND src1)
{
    int pattern_item_index = fpi_get_pattern_item_index(pir1);
    if (pattern_item_index == 0)
        return;
    FLAG_PATTERN_ITEM *item =
        lsenv->fp_data->pattern_items + pattern_item_index;
    if (item->head != pir1)
        return;

    if (item->src0_reg_num == -1) {
        /* lsassert(item->src1_reg_num == -1 && item->dest_reg_num==0); */
        int sll_value = 32 - ir1_opnd_size(ir1_get_opnd(pir1, 0));

        IR2_OPND save_src0 = ra_alloc_flag_pattern_saved_opnd0();
        la_append_ir2_opnd2i_em(LISA_SLLI_W, save_src0, src0, sll_value);
        item->src0_reg_num = ir2_opnd_base_reg_num(&save_src0);

        IR2_OPND save_src1 = ra_alloc_flag_pattern_saved_opnd1();
        if (ir2_opnd_is_ireg(&src1))
            la_append_ir2_opnd2i_em(LISA_SLLI_W, save_src1, src1, sll_value);
        else
            load_ireg_from_imm32(save_src1, ir2_opnd_imm(&src1) << sll_value,
                                 SIGN_EXTENSION);
        item->src1_reg_num = ir2_opnd_base_reg_num(&save_src1);
    } else {
        /* lsassert(item->src0_reg_num == 0 && item->src1_reg_num == 0 && */
        /* item->dest_reg_num==-1); */
    }
}

bool fp_is_save_dest_opnd(IR1_INST *pir1, IR2_OPND dest)
{
    int pattern_item_index = fpi_get_pattern_item_index(pir1);
    if (pattern_item_index == 0)
        return false;
    FLAG_PATTERN_ITEM *item =
        lsenv->fp_data->pattern_items + pattern_item_index;
    if (item->head != pir1)
        return false;

    if (item->dest_reg_num == -1) {
        return true;
    } else {
        /* lsassert(item->src0_reg_num>0 && item->src1_reg_num>0 && */
        /* item->dest_reg_num==0); */
        return false;
    }
}

void fp_save_dest_opnd(IR1_INST *pir1, IR2_OPND dest)
{
    int pattern_item_index = fpi_get_pattern_item_index(pir1);
    if (pattern_item_index == 0)
        return;
    FLAG_PATTERN_ITEM *item =
        lsenv->fp_data->pattern_items + pattern_item_index;
    if (item->head != pir1)
        return;

    if (item->dest_reg_num == -1) {
        /* lsassert(item->src0_reg_num == 0 && item->src1_reg_num == 0); */
        int sll_value = 32 - ir1_opnd_size(ir1_get_opnd(pir1, 0));

        if ((ir1_opcode(pir1) == X86_INS_CMP || ir1_opcode(pir1) == X86_INS_TEST) &&
            sll_value == 0) {
            item->dest_reg_num = ir2_opnd_base_reg_num(&dest);
        } else {
            IR2_OPND save_dest = ra_alloc_flag_pattern_saved_opnd0();
            la_append_ir2_opnd2i_em(LISA_SLLI_W, save_dest, dest, sll_value);
            item->dest_reg_num = ir2_opnd_base_reg_num(&save_dest);
        }
    } else {
        /* lsassert(item->src0_reg_num>0 && item->src1_reg_num>0 && */
        /* item->dest_reg_num==0); */
    }
}

static void fpi_translate_pattern_tail_unsigned(FLAG_PATTERN_ITEM *item,
                                                FP_MODE mode,
                                                IR2_OPND provided_opnd2)
{
    IR2_OPND src0 = ir2_opnd_new(IR2_OPND_IREG, item->src0_reg_num);
    IR2_OPND src1 = ir2_opnd_new(IR2_OPND_IREG, item->src1_reg_num);
    IR2_OPND condition = ra_alloc_itemp(); /* for jcc */

    /* generate ir2 for pattern tail */
    switch (mode) {
    case CMP_JA:
    case SUB_JA:
        la_append_ir2_opnd3_em(LISA_SLTU, condition, src1, src0);
        la_append_ir2_opnd3(LISA_BNE, condition, zero_ir2_opnd, provided_opnd2);
        break;
    case CMP_JBE:
    case SUB_JBE:
        la_append_ir2_opnd3_em(LISA_SLTU, condition, src1, src0);
        la_append_ir2_opnd3(LISA_BEQ, condition, zero_ir2_opnd, provided_opnd2);
        break;
    case CMP_JB:
    case SUB_JB:
        la_append_ir2_opnd3_em(LISA_SLTU, condition, src0, src1);
        la_append_ir2_opnd3(LISA_BNE, condition, zero_ir2_opnd, provided_opnd2);
        break;
    case CMP_JAE:
    case SUB_JAE:
        la_append_ir2_opnd3_em(LISA_SLTU, condition, src0, src1);
        la_append_ir2_opnd3(LISA_BEQ, condition, zero_ir2_opnd, provided_opnd2);
        break;

    case ADD_JAE:
        la_append_ir2_opnd3_em(LISA_ADD_W, src0, src0, src1);
        la_append_ir2_opnd3_em(LISA_SLTU, condition, src0, src1);
        la_append_ir2_opnd3(LISA_BEQ, condition, zero_ir2_opnd, provided_opnd2);
        break;
    case ADD_JB:
        la_append_ir2_opnd3_em(LISA_ADD_W, src0, src0, src1);
        la_append_ir2_opnd3_em(LISA_SLTU, condition, src0, src1);
        la_append_ir2_opnd3(LISA_BNE, condition, zero_ir2_opnd, provided_opnd2);
        break;

    case CMP_CMOVA:
    case SUB_CMOVA:
        la_append_ir2_opnd3_em(LISA_SLTU, provided_opnd2, src1, src0);
        la_append_ir2_opnd2i_em(LISA_XORI, provided_opnd2, provided_opnd2, 1);
        break;
    case CMP_CMOVBE:
    case SUB_CMOVBE:
        la_append_ir2_opnd3_em(LISA_SLTU, provided_opnd2, src1, src0);
        la_append_ir2_opnd2i_em(LISA_XORI, provided_opnd2, provided_opnd2, 1);
        break;
    case CMP_CMOVB:
    case SUB_CMOVB:
        la_append_ir2_opnd3_em(LISA_SLTU, provided_opnd2, src0, src1);
        break;
    case CMP_CMOVAE:
    case SUB_CMOVAE:
        la_append_ir2_opnd3_em(LISA_SLTU, provided_opnd2, src0, src1);
        break;

    case CMP_SETA:
    case SUB_SETA:
        la_append_ir2_opnd3_em(LISA_SLTU, provided_opnd2, src1, src0);
        break;
    case CMP_SETBE:
    case SUB_SETBE:
        la_append_ir2_opnd3_em(LISA_SLTU, provided_opnd2, src1, src0);
        la_append_ir2_opnd2i_em(LISA_XORI, provided_opnd2, provided_opnd2, 1);
        break;
    case CMP_SETB:
    case SUB_SETB:
        la_append_ir2_opnd3_em(LISA_SLTU, provided_opnd2, src0, src1);
        break;
    case CMP_SETAE:
    case SUB_SETAE:
        la_append_ir2_opnd3_em(LISA_SLTU, provided_opnd2, src0, src1);
        la_append_ir2_opnd2i_em(LISA_XORI, provided_opnd2, provided_opnd2, 1);
        break;

    default:
        /* env->tr_data->curr_tb->dump(); */
        /* lsassertm(0, "flag pattern %s_%s not implemented\n", */
        /* ir1_name(mode>>16), ir1_name(mode&0xffff)); */
        break;
    }

    ra_free_temp(condition);
}

static void fpi_translate_pattern_tail_signed(FLAG_PATTERN_ITEM *item,
                                              FP_MODE mode,
                                              IR2_OPND provided_opnd2)
{
    IR2_OPND src0 = ir2_opnd_new(IR2_OPND_IREG, item->src0_reg_num);
    IR2_OPND src1 = ir2_opnd_new(IR2_OPND_IREG, item->src1_reg_num);
    IR2_OPND dest = ir2_opnd_new(IR2_OPND_IREG, item->dest_reg_num);
    IR2_OPND condition = ra_alloc_itemp(); /* for jcc */

    /* generate ir2 for pattern tail */
    switch (mode) {
    case CMP_JG:
    case SUB_JG:
        la_append_ir2_opnd3(LISA_SLT, condition, src1, src0);
        la_append_ir2_opnd3(LISA_BNE, condition, zero_ir2_opnd, provided_opnd2);
        break;
    case CMP_JLE:
    case SUB_JLE:
        la_append_ir2_opnd3(LISA_SLT, condition, src1, src0);
        la_append_ir2_opnd3(LISA_BEQ, condition, zero_ir2_opnd, provided_opnd2);
        break;
    case CMP_JL:
    case SUB_JL:
        la_append_ir2_opnd3(LISA_SLT, condition, src0, src1);
        la_append_ir2_opnd3(LISA_BNE, condition, zero_ir2_opnd, provided_opnd2);
        break;
    case CMP_JGE:
    case SUB_JGE:
        la_append_ir2_opnd3(LISA_SLT, condition, src0, src1);
        la_append_ir2_opnd3(LISA_BEQ, condition, zero_ir2_opnd, provided_opnd2);
        break;

    case CMP_CMOVG:
    case SUB_CMOVG:
        la_append_ir2_opnd3(LISA_SLT, provided_opnd2, src1, src0);
        la_append_ir2_opnd2i_em(LISA_XORI, provided_opnd2, provided_opnd2, 1);
        break;
    case CMP_CMOVLE:
    case SUB_CMOVLE:
        la_append_ir2_opnd3(LISA_SLT, provided_opnd2, src1, src0);
        la_append_ir2_opnd2i_em(LISA_XORI, provided_opnd2, provided_opnd2, 1);
        break;
    case CMP_CMOVL:
    case SUB_CMOVL:
        la_append_ir2_opnd3(LISA_SLT, provided_opnd2, src0, src1);
        break;
    case CMP_CMOVGE:
    case SUB_CMOVGE:
        la_append_ir2_opnd3(LISA_SLT, provided_opnd2, src0, src1);
        break;

    case CMP_SETG:
    case SUB_SETG:
        la_append_ir2_opnd3(LISA_SLT, provided_opnd2, src1, src0);
        break;
    case CMP_SETLE:
    case SUB_SETLE:
        la_append_ir2_opnd3(LISA_SLT, provided_opnd2, src1, src0);
        la_append_ir2_opnd2i_em(LISA_XORI, provided_opnd2, provided_opnd2, 1);
        break;
    case CMP_SETL:
    case SUB_SETL:
        la_append_ir2_opnd3(LISA_SLT, provided_opnd2, src0, src1);
        break;
    case CMP_SETGE:
    case SUB_SETGE:
        la_append_ir2_opnd3(LISA_SLT, provided_opnd2, src0, src1);
        la_append_ir2_opnd2i_em(LISA_XORI, provided_opnd2, provided_opnd2, 1);
        break;

    case TEST_JG:
    case OR_JG:
    case AND_JG:
        la_append_ir2_opnd3(LISA_BLT, zero_ir2_opnd, dest, provided_opnd2);
        break;
    case TEST_JLE:
    case OR_JLE:
    case AND_JLE:
        la_append_ir2_opnd3(LISA_BGE, zero_ir2_opnd, dest, provided_opnd2);
        break;
    case TEST_JL:
    case OR_JL:
    case AND_JL:
        la_append_ir2_opnd3(LISA_BLT, dest, zero_ir2_opnd, provided_opnd2);
        break;
    case TEST_JGE:
    case OR_JGE:
    case AND_JGE:
        la_append_ir2_opnd3(LISA_BGE, dest, zero_ir2_opnd, provided_opnd2);
        break;

    default:
        /* env->tr_data->curr_tb->dump(); */
        /* lsassertm(0, "flag pattern %s_%s not implemented\n", */
        /* ir1_name(mode>>16), ir1_name(mode&0xffff)); */
        break;
    }

    ra_free_temp(condition);
}

static void fpi_translate_pattern_tail_szap(FLAG_PATTERN_ITEM *item,
                                            FP_MODE mode,
                                            IR2_OPND provided_opnd2)
{
    IR2_OPND dest = ir2_opnd_new(IR2_OPND_IREG, item->dest_reg_num);

    /* if the dest was not saved, calculate it */
    if (item->dest_reg_num == 0) {
        dest = ra_alloc_itemp();
        item->dest_reg_num = ir2_opnd_base_reg_num(&dest);

        IR2_OPND src0 = ir2_opnd_new(IR2_OPND_IREG, item->src0_reg_num);
        IR2_OPND src1 = ir2_opnd_new(IR2_OPND_IREG, item->src1_reg_num);

        switch (mode >> 16) {
        case X86_INS_CMP:
        case X86_INS_SUB:
            la_append_ir2_opnd3_em(LISA_SUB_W, dest, src0, src1);
            break;
        case X86_INS_ADD:
        case X86_INS_INC:
            la_append_ir2_opnd3_em(LISA_ADD_W, dest, src0, src1);
            break;
        default:
            /* lsassertm(0, "need calculate dest for %s\n", */
            /* ir1_name(mode>>16)); */
            break;
        }
    }

    /* generate ir2 for pattern tail */
    switch (mode) {
    case CMP_JZ:
    case SUB_JZ:
        la_append_ir2_opnd3(LISA_BEQ, dest, zero_ir2_opnd, provided_opnd2);
        break;
    case CMP_JNZ:
    case SUB_JNZ:
        la_append_ir2_opnd3(LISA_BNE, dest, zero_ir2_opnd, provided_opnd2);
        break;

    case CMP_JS:
    case SUB_JS:
        la_append_ir2_opnd2i_em(LISA_SRLI_W, dest, dest, 31);
        la_append_ir2_opnd3(LISA_BNE, dest, zero_ir2_opnd, provided_opnd2);
        break;
    case CMP_JNS:
    case SUB_JNS:
        la_append_ir2_opnd2i_em(LISA_SRLI_W, dest, dest, 31);
        la_append_ir2_opnd3(LISA_BEQ, dest, zero_ir2_opnd, provided_opnd2);
        break;

    case TEST_JZ:
    case OR_JZ:
    case AND_JZ:
    case DEC_JZ:
    case ADD_JZ:
    case INC_JZ:
        la_append_ir2_opnd3(LISA_BEQ, dest, zero_ir2_opnd, provided_opnd2);
        break;

    case TEST_JNZ:
    case OR_JNZ:
    case AND_JNZ:
    case DEC_JNZ:
    case ADD_JNZ:
    case INC_JNZ:
        la_append_ir2_opnd3(LISA_BNE, dest, zero_ir2_opnd, provided_opnd2);
        break;

    case TEST_JS:
    case OR_JS:
    case AND_JS:
        la_append_ir2_opnd2i_em(LISA_SRLI_W, dest, dest, 31);
        la_append_ir2_opnd3(LISA_BNE, dest, zero_ir2_opnd, provided_opnd2);
        break;
    case TEST_JNS:
    case OR_JNS:
    case AND_JNS:
    case ADD_JNS:
        la_append_ir2_opnd2i_em(LISA_SRLI_W, dest, dest, 31);
        la_append_ir2_opnd3(LISA_BEQ, dest, zero_ir2_opnd, provided_opnd2);
        break;

    case CMP_CMOVZ:
    case SUB_CMOVZ:
        la_append_ir2_opnd2i_em(LISA_SLTUI, provided_opnd2, dest, 1);
        break;
    case CMP_CMOVNZ:
    case SUB_CMOVNZ:
        la_append_ir2_opnd2i_em(LISA_SLTUI, provided_opnd2, dest, 1);
        break;

    case CMP_CMOVS:
    case SUB_CMOVS:
        la_append_ir2_opnd2i_em(LISA_SRLI_W, provided_opnd2, dest, 31);
        break;
    case CMP_CMOVNS:
    case SUB_CMOVNS:
        la_append_ir2_opnd2i_em(LISA_SRLI_W, provided_opnd2, dest, 31);
        break;

    case TEST_CMOVZ:
    case OR_CMOVZ:
    case AND_CMOVZ:
        la_append_ir2_opnd2i_em(LISA_SLTUI, provided_opnd2, dest, 1);
        break;
    case TEST_CMOVNZ:
    case OR_CMOVNZ:
    case AND_CMOVNZ:
        la_append_ir2_opnd2i_em(LISA_SLTUI, provided_opnd2, dest, 1);
        break;

    case TEST_CMOVS:
    case OR_CMOVS:
    case AND_CMOVS:
        la_append_ir2_opnd2i_em(LISA_SRLI_W, provided_opnd2, dest, 31);
        break;
    case TEST_CMOVNS:
    case OR_CMOVNS:
    case AND_CMOVNS:
        la_append_ir2_opnd2i_em(LISA_SRLI_W, provided_opnd2, dest, 31);
        break;

    case CMP_SETZ:
    case SUB_SETZ:
        la_append_ir2_opnd2i_em(LISA_SLTUI, provided_opnd2, dest, 1);
        break;
    case CMP_SETNZ:
    case SUB_SETNZ:
        la_append_ir2_opnd3_em(LISA_SLTU, provided_opnd2, zero_ir2_opnd, dest);
        break;

    case TEST_SETZ:
    case OR_SETZ:
    case AND_SETZ:
        la_append_ir2_opnd2i_em(LISA_SLTUI, provided_opnd2, dest, 1);
        break;
    case TEST_SETNZ:
    case OR_SETNZ:
    case AND_SETNZ:
        la_append_ir2_opnd3_em(LISA_SLTU, provided_opnd2, zero_ir2_opnd, dest);
        break;

    default:
        /* env->tr_data->curr_tb->dump(); */
        /* lsassertm(0, "flag pattern %s_%s not implemented\n", */
        /* ir1_name(mode>>16), ir1_name(mode&0xffff)); */
        break;
    }
}

bool fp_translate_pattern_tail(IR1_INST *pir1, IR2_OPND provided_opnd2)
{
    int pattern_item_index = fpi_get_pattern_item_index(pir1);
    if (pattern_item_index == 0)
        return false;

    /* determine the flag pattern mode and item */
    FLAG_PATTERN_ITEM *item =
        lsenv->fp_data->pattern_items + pattern_item_index;
    FP_MODE mode = (FP_MODE)((ir1_opcode(item->head) << 16) + ir1_opcode(pir1));

    /* call sub functions */
    uint8 eflags_used_by_tail = ir1_get_eflag_use(pir1);
    if (BITS_ARE_SET(eflags_used_by_tail, CF_USEDEF_BIT))
        fpi_translate_pattern_tail_unsigned(item, mode, provided_opnd2);
    else if (BITS_ARE_SET(eflags_used_by_tail, OF_USEDEF_BIT))
        fpi_translate_pattern_tail_signed(item, mode, provided_opnd2);
    else
        fpi_translate_pattern_tail_szap(item, mode, provided_opnd2);

    /* release external temp registers */
    if (pir1 == item->tails[0]) {
        if (item->dest_reg_num > 0)
            ra_free_itemp(item->dest_reg_num);
        if (item->src0_reg_num > 0)
            ra_free_itemp(item->src0_reg_num);
        if (item->src1_reg_num > 0)
            ra_free_itemp(item->src1_reg_num);
    }
    return true;
}

void fp_init_skipped_flags(IR1_INST *pir1)
{
    lsenv->tr_data->curr_ir1_skipped_eflags = 0;

    if (!fpi_is_head_op(ir1_opcode(pir1)))
        return;
    int pattern_item_index = fpi_get_pattern_item_index(pir1);
    if (pattern_item_index == 0)
        return;
    FLAG_PATTERN_ITEM *item =
        lsenv->fp_data->pattern_items + pattern_item_index;
    if (item->head != pir1)
        return;

    lsenv->tr_data->curr_ir1_skipped_eflags = item->skipped_flags;
}
#endif
