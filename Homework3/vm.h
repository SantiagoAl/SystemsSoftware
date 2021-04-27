#ifndef _VM_H
#define _VM_H

#include "codegen.h"

// This is what is used to emulate the 'text segment' as well as the
// 'Instruction Register' (IR)
typedef struct
{
    int OP;
    int R;
    int L;
    int M;
} Instructions;

void virtual_machine(instruction *code, int printFlag);

int base(int L, int base, int *stack);

#endif /* _VM_H */
