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

int findTableIndex(symbol *table, char *name, int lexlevel, int kind);

int findIndex(symbol *table, char *name, int lexlevel);

int findProc(symbol *table, char *procName, int lexlevel, int kind);

void codeBlock(instruction *code, symbol *table, int lexlevel);

void codeStatement(instruction *code, symbol *table, int lexlevel);

void codeExpression(instruction *code, symbol *table, int registerVal, int lexlevel);

void codeCondition(instruction *code, symbol *table, int lexlevel);

void codeTerm(instruction *code, symbol *table, int registerVal, int lexlevel);

void codeFactor(instruction *code, symbol *table, int registerVal, int lexlevel);

#endif /*_CODEGEN_H */
