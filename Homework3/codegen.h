#ifndef _CODEGEN_H
#define _CODEGEN_H

#include "lex.h"
#include "parser.h"

typedef struct
{
    int opcode;
    int r;
    int l;
    int m;
} instruction;

instruction *generate_code(symbol *table, lexeme *list);

void emit(instruction *code, int op, int r, int l, int m);

void codeBlock(instruction *code, symbol *table);

void codeStatement(instruction *code, symbol *table);

void codeExpression(instruction *code, symbol *table, int registerVal);

void codeCondition(instruction *code, symbol *table);

void codeTerm(instruction *code, symbol *table, int registerVal);

void codeFactor(instruction *code, symbol *table, int registerVal);

#endif /*_CODEGEN_H */
