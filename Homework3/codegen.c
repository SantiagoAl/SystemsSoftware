#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "lex.h"
#include "parser.h"
#include "codegen.h"

const int CODE_SIZE = 500;
int cx = 0;

lexeme *nodePointer;

void emit(instruction *code, int op, int r, int l, int m)
{
    if (cx > CODE_SIZE)
    {
        printf("Error: To much code\n");
        return;
    }
    else
    {
        code[cx].opcode = op;
        code[cx].r = r;
        code[cx].l = l;
        code[cx++].m = m;
    }
}

int findTableIndex(symbol *table, char *nameIdent)
{
    for (int i = 0; i < CODE_SIZE; i++)
    {
        if (table[i].kind == -1)
            break;

        if (!strcmp(table[i].name, nameIdent))
            return i;
    }

    return -1;
}

void codeFactor(instruction *code, symbol *table, int registerVal)
{
    if (nodePointer->typeToken == identsym)
    {
        char nameIdent[12];
        strcpy(nameIdent, nodePointer->name);

        int i = findTableIndex(table, nameIdent);

        if (nodePointer->typeToken == constsym)
        {
            emit(code, 1, registerVal, 0, table[i].val);
        }

        if (nodePointer->typeToken == varsym)
        {
            emit(code, 3, registerVal, 0, table[i].addr);
        }

        nodePointer = nodePointer->next;
    }
    else if (nodePointer->typeToken == numbersym)
    {
        emit(code, 1, registerVal, 0, nodePointer->numValue);
        nodePointer = nodePointer->next;
    }
    else
    {
        nodePointer = nodePointer->next;
        codeExpression(code, table, registerVal);
        nodePointer = nodePointer->next;
    }

    return;
}

void codeTerm(instruction *code, symbol *table, int registerVal)
{
    codeFactor(code, table, registerVal);
    while (nodePointer->typeToken == multsym || nodePointer->typeToken == slashsym)
    {
        if (nodePointer->typeToken == multsym)
        {
            nodePointer = nodePointer->next;
            codeFactor(code, table, registerVal + 1);
            emit(code, 13, registerVal, registerVal, registerVal + 1);
        }

        if (nodePointer->typeToken == slashsym)
        {
            nodePointer = nodePointer->next;
            codeFactor(code, table, registerVal + 1);
            emit(code, 14, registerVal, registerVal, registerVal + 1);
        }
    }

    return;
}

void codeExpression(instruction *code, symbol *table, int registerVal)
{
    if (nodePointer->typeToken == plussym)
        nodePointer = nodePointer->next;

    if (nodePointer->typeToken == minussym)
    {
        nodePointer = nodePointer->next;
        codeTerm(code, table, registerVal);
        emit(code, 10, registerVal + 1, 0, 0);

        while (nodePointer->typeToken == plussym || nodePointer->typeToken == minussym)
        {
            if (nodePointer->typeToken == plussym)
            {
                nodePointer = nodePointer->next;
                codeTerm(code, table, registerVal + 1);
                emit(code, 11, registerVal, registerVal, registerVal + 1);
            }

            if (nodePointer->typeToken == minussym)
            {
                nodePointer = nodePointer->next;
                codeTerm(code, table, registerVal + 1);
                emit(code, 12, registerVal, registerVal, registerVal + 1);
            }

            return;
        }
    }

    codeTerm(code, table, registerVal);

    while (nodePointer->typeToken == plussym || nodePointer->typeToken == minussym)
    {
        if (nodePointer->typeToken == plussym)
        {
            nodePointer = nodePointer->next;
            codeTerm(code, table, registerVal + 1);
            emit(code, 11, registerVal, registerVal, registerVal + 1);
        }

        if (nodePointer->typeToken == minussym)
        {
            nodePointer = nodePointer->next;
            codeTerm(code, table, registerVal + 1);
            emit(code, 12, registerVal, registerVal, registerVal + 1);
        }
    }

    return;
}

void codeCondition(instruction *code, symbol *table)
{
    if (nodePointer->typeToken == oddsym)
    {
        nodePointer = nodePointer->next;
        codeExpression(code, table, 0);
        emit(code, 15, 0, 0, 0);
    }
    else
    {
        codeExpression(code, table, 0);
        if (nodePointer->typeToken == eqsym)
        {
            nodePointer = nodePointer->next;
            codeExpression(code, table, 1);
            emit(code, 17, 0, 0, 1);
        }

        if (nodePointer->typeToken == neqsym)
        {
            nodePointer = nodePointer->next;
            codeExpression(code, table, 1);
            emit(code, 18, 0, 0, 1);
        }

        if (nodePointer->typeToken == lessym)
        {
            nodePointer = nodePointer->next;
            codeExpression(code, table, 1);
            emit(code, 19, 0, 0, 1);
        }

        if (nodePointer->typeToken == leqsym)
        {
            nodePointer = nodePointer->next;
            codeExpression(code, table, 1);
            emit(code, 20, 0, 0, 1);
        }

        if (nodePointer->typeToken == gtrsym)
        {
            nodePointer = nodePointer->next;
            codeExpression(code, table, 1);
            emit(code, 21, 0, 0, 1);
        }

        if (nodePointer->typeToken == geqsym)
        {
            nodePointer = nodePointer->next;
            codeExpression(code, table, 1);
            emit(code, 22, 0, 0, 1);
        }
    }

    return;
}

void codeStatement(instruction *code, symbol *table)
{
    if (nodePointer->typeToken == identsym)
    {
        char nameIdent[12];
        strcpy(nameIdent, nodePointer->name);

        int i = findTableIndex(table, nameIdent);
        nodePointer = nodePointer->next->next;

        codeExpression(code, table, 0);
        emit(code, 4, 0, 0, table[i].addr);
    }

    if (nodePointer->typeToken == beginsym)
    {
        nodePointer = nodePointer->next;
        codeStatement(code, table);

        while (nodePointer->typeToken == semicolonsym)
        {
            nodePointer = nodePointer->next;
            codeStatement(code, table);
        }

        nodePointer = nodePointer->next;
    }

    if (nodePointer->typeToken == ifsym)
    {
        nodePointer = nodePointer->next;
        codeCondition(code, table);

        int savedIndex = cx;
        emit(code, 8, 0, 0, 0);
        nodePointer = nodePointer->next;
        codeStatement(code, table);
        code[savedIndex].m = code[cx].m;
    }

    if (nodePointer->typeToken == whilesym)
    {
        nodePointer = nodePointer->next;
        int conditionIndex = cx;
        codeCondition(code, table);

        nodePointer = nodePointer->next;
        int jumpIndex = cx;

        emit(code, 8, 0, 0, 0);
        codeStatement(code, table);

        emit(code, 7, 0, 0, code[conditionIndex].m);

        code[jumpIndex].m = code[cx].m;
    }

    if (nodePointer->typeToken == readsym)
    {
        nodePointer = nodePointer->next;

        char nameIdent[12];
        strcpy(nameIdent, nodePointer->name);

        int i = findTableIndex(table, nameIdent);
        nodePointer = nodePointer->next;

        emit(code, 9, 0, 0, 2);
        emit(code, 4, 0, 0, table[i].addr);
    }

    if (nodePointer->typeToken == writesym)
    {
        nodePointer = nodePointer->next;

        char nameIdent[12];
        strcpy(nameIdent, nodePointer->name);

        int i = findTableIndex(table, nameIdent);

        if (table[i].kind == 2)
        {
            emit(code, 3, 0, 0, table[i].addr);
            emit(code, 9, 0, 0, 1);
        }

        if (table[i].kind == 1)
        {
            emit(code, 1, 0, 0, table[i].val);
            emit(code, 9, 0, 0, 1);
        }

        nodePointer = nodePointer->next;
    }

    return;
}

void codeBlock(instruction *code, symbol *table)
{
    int numVars = 0;

    if (nodePointer->typeToken == constsym)
    {
        do
        {
            nodePointer = nodePointer->next->next->next->next;

        } while(nodePointer->typeToken == commasym);

        nodePointer = nodePointer->next;
    }

    if (nodePointer->typeToken == varsym)
    {
        do
        {
            numVars++;
            nodePointer = nodePointer->next->next;
        } while(nodePointer->typeToken == commasym);

        nodePointer = nodePointer->next;
    }

    emit(code, 6, 0, 0, 3 + numVars);
    codeStatement(code, table);

    return;
}

instruction *generate_code(symbol *table, lexeme *list)
{
    instruction *code = malloc(500 * sizeof(instruction));

    if (code == NULL)
    {
        printf("Dynamic memory allocation failed in 'generate_code'\n");
        return NULL;
    }

    nodePointer = list;

    emit(code, 7, 0, 0, 1);

    codeBlock(code, table);

    emit(code, 9, 0, 0, 3);

    if (cx < 499)
    {
        code[cx].opcode = -1;
        code[cx].r = 0;
        code[cx].l = 0;
        code[cx++].m = 0;
    }

    return code;
}
