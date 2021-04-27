#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "lex.h"
#include "parser.h"
#include "codegen.h"

const int CODE_SIZE = 500;
int cx = 0;
int tIndex = 1;

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

int findTableIndex(symbol *table, char *name, int lexlevel, int kind)
{
    int index = 1;

    for (int i = 1; i < CODE_SIZE; i++)
    {
        if (table[i].kind == -1)
            break;

        if ((table[i].level <= lexlevel) && !table[i].marked
            && (table[i].kind == kind))
            {
                index = i;
            }
    }

    return index;
}

int findIndex(symbol *table, char *name, int lexlevel)
{
    int index = 1;

    for (int i = 1; i < CODE_SIZE; i++)
    {
        if (table[i].kind == -1 || table[i].level > lexlevel)
            break;

        if (!strcmp(table[i].name, name) && table[i].level <= lexlevel &&
            !table[i].marked && (table[i].kind == 1 || table[i].kind == 2))
            index = i;
    }

    return index;
}

int findProc(symbol *table, char *procName, int lexlevel, int kind)
{
    int index = 0;

    for (int i = tIndex - 1; i >= 0; i--)
    {
        if (!strcmp(table[i].name, procName) && table[i].level >= lexlevel
            && table[i].kind == kind && !table[i].marked)
            index = i;
    }

    return index;
}

void codeFactor(instruction *code, symbol *table, int registerVal, int lexlevel)
{
    if (nodePointer->typeToken == identsym)
    {
        char nameIdent[12];
        strcpy(nameIdent, nodePointer->name);

        int i = findIndex(table, nameIdent, lexlevel);

        if (table[i].kind == 1)
        {
            emit(code, 1, registerVal, 0, table[i].val);
        }

        if (table[i].kind == 2)
        {
            // emit(code, 3, registerVal, (table[i].level - lexlevel), table[i].addr);
            emit(code, 3, registerVal, (lexlevel - table[i].level), table[i].addr - 1);
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
        codeExpression(code, table, registerVal, lexlevel);
        nodePointer = nodePointer->next;
    }

    return;
}

void codeTerm(instruction *code, symbol *table, int registerVal, int lexlevel)
{
    codeFactor(code, table, registerVal, lexlevel);
    while (nodePointer->typeToken == multsym || nodePointer->typeToken == slashsym)
    {
        if (nodePointer->typeToken == multsym)
        {
            nodePointer = nodePointer->next;
            codeFactor(code, table, registerVal + 1, lexlevel);
            emit(code, 13, registerVal, registerVal, registerVal + 1);
        }

        if (nodePointer->typeToken == slashsym)
        {
            nodePointer = nodePointer->next;
            codeFactor(code, table, registerVal + 1, lexlevel);
            emit(code, 14, registerVal, registerVal, registerVal + 1);
        }
    }

    return;
}

void codeExpression(instruction *code, symbol *table, int registerVal, int lexlevel)
{
    if (nodePointer->typeToken == plussym)
        nodePointer = nodePointer->next;

    if (nodePointer->typeToken == minussym)
    {
        nodePointer = nodePointer->next;
        codeTerm(code, table, registerVal, lexlevel);
        emit(code, 10, registerVal, 0, 0);

        while (nodePointer->typeToken == plussym || nodePointer->typeToken == minussym)
        {
            if (nodePointer->typeToken == plussym)
            {
                nodePointer = nodePointer->next;
                codeTerm(code, table, registerVal + 1, lexlevel);
                emit(code, 11, registerVal, registerVal, registerVal + 1);
            }

            if (nodePointer->typeToken == minussym)
            {
                nodePointer = nodePointer->next;
                codeTerm(code, table, registerVal + 1, lexlevel);
                emit(code, 12, registerVal, registerVal, registerVal + 1);
            }

            return;
        }
    }

    codeTerm(code, table, registerVal, lexlevel);

    while (nodePointer->typeToken == plussym || nodePointer->typeToken == minussym)
    {
        if (nodePointer->typeToken == plussym)
        {
            nodePointer = nodePointer->next;
            codeTerm(code, table, registerVal + 1, lexlevel);
            emit(code, 11, registerVal, registerVal, registerVal + 1);
        }

        if (nodePointer->typeToken == minussym)
        {
            nodePointer = nodePointer->next;
            codeTerm(code, table, registerVal + 1, lexlevel);
            emit(code, 12, registerVal, registerVal, registerVal + 1);
        }
    }

    return;
}

void codeCondition(instruction *code, symbol *table, int lexlevel)
{
    if (nodePointer->typeToken == oddsym)
    {
        nodePointer = nodePointer->next;
        codeExpression(code, table, 0, lexlevel);
        emit(code, 15, 0, 0, 0);
    }
    else
    {
        codeExpression(code, table, 0, lexlevel);
        if (nodePointer->typeToken == eqsym)
        {
            nodePointer = nodePointer->next;
            codeExpression(code, table, 1, lexlevel);
            emit(code, 17, 0, 0, 1);
        }

        if (nodePointer->typeToken == neqsym)
        {
            nodePointer = nodePointer->next;
            codeExpression(code, table, 1, lexlevel);
            emit(code, 18, 0, 0, 1);
        }

        if (nodePointer->typeToken == lessym)
        {
            nodePointer = nodePointer->next;
            codeExpression(code, table, 1, lexlevel);
            emit(code, 19, 0, 0, 1);
        }

        if (nodePointer->typeToken == leqsym)
        {
            nodePointer = nodePointer->next;
            codeExpression(code, table, 1, lexlevel);
            emit(code, 20, 0, 0, 1);
        }

        if (nodePointer->typeToken == gtrsym)
        {
            nodePointer = nodePointer->next;
            codeExpression(code, table, 1, lexlevel);
            emit(code, 21, 0, 0, 1);
        }

        if (nodePointer->typeToken == geqsym)
        {
            nodePointer = nodePointer->next;
            codeExpression(code, table, 1, lexlevel);
            emit(code, 22, 0, 0, 1);
        }
    }

    return;
}

void codeStatement(instruction *code, symbol *table, int lexlevel)
{
    if (nodePointer->typeToken == identsym)
    {
        char nameIdent[12];
        strcpy(nameIdent, nodePointer->name);

        int index = findTableIndex(table, nameIdent, lexlevel, 2);
        nodePointer = nodePointer->next->next;

        codeExpression(code, table, 0, lexlevel);
        emit(code, 4, 0, table[index].level, table[index].addr);
    }

    if (nodePointer->typeToken == beginsym)
    {
        nodePointer = nodePointer->next;
        codeStatement(code, table, lexlevel);

        while (nodePointer->typeToken == semicolonsym)
        {
            nodePointer = nodePointer->next;
            codeStatement(code, table, lexlevel);
        }

        nodePointer = nodePointer->next;
    }

    if (nodePointer->typeToken == callsym)
    {
        nodePointer = nodePointer->next;

        char nameIdent[12];
        strcpy(nameIdent, nodePointer->name);

        int index = findProc(table, nameIdent, lexlevel, 3);
        // int index = findTableIndex(table, nameIdent, lexlevel, 3);

        emit(code, 5, 0, (lexlevel - table[index].level), table[index].val);

        nodePointer = nodePointer->next;
    }

    if (nodePointer->typeToken == ifsym)
    {
        nodePointer = nodePointer->next;
        codeCondition(code, table, lexlevel);

        int savedIndex = cx;
        emit(code, 8, 0, 0, 0);
        nodePointer = nodePointer->next;
        codeStatement(code, table, lexlevel);

        if (nodePointer->typeToken == elsesym)
        {
            nodePointer = nodePointer->next;
            int nextSavedIndex = cx;
            emit(code, 7, 0, 0, 0);
            code[savedIndex].m = cx;
            // code[savedIndex].m = code[cx].m;

            codeStatement(code, table, lexlevel);

            code[nextSavedIndex].m = cx;
            // code[nextSavedIndex].m = code[cx].m;
        }
        else
        {
            code[savedIndex].m = cx;;
            // code[savedIndex].m = code[cx].m;
        }
    }

    if (nodePointer->typeToken == whilesym)
    {
        nodePointer = nodePointer->next;
        int conditionIndex = cx;
        codeCondition(code, table, lexlevel);

        nodePointer = nodePointer->next;
        int jumpIndex = cx;

        emit(code, 8, 0, 0, 0);
        codeStatement(code, table, lexlevel);

        emit(code, 7, 0, 0, code[conditionIndex].m);

        code[jumpIndex].m = cx;
        // code[jumpIndex].m = code[cx].m;
    }

    if (nodePointer->typeToken == readsym)
    {
        nodePointer = nodePointer->next;

        char nameIdent[12];
        strcpy(nameIdent, nodePointer->name);

        int i = findTableIndex(table, nameIdent, lexlevel, 2);
        nodePointer = nodePointer->next;

        emit(code, 9, 0, 0, 2);
        emit(code, 4, 0, (lexlevel - table[i].level), table[i].addr);
    }

    if (nodePointer->typeToken == writesym)
    {
        nodePointer = nodePointer->next;

        codeExpression(code, table, 0, lexlevel);
        emit(code, 9, 0, 0, 1);

        // nodePointer = nodePointer->next;
    }

    return;
}

void codeBlock(instruction *code, symbol *table, int lexlevel)
{
    int numSymbols = 0;
    int numVars = 0;
    int procIndex = -1;

    if (nodePointer->typeToken == constsym)
    {
        do
        {
            nodePointer = nodePointer->next;
            numSymbols++;
            table[tIndex++].marked = 0;
            nodePointer = nodePointer->next->next->next;

        } while(nodePointer->typeToken == commasym);

        nodePointer = nodePointer->next;
    }

    if (nodePointer->typeToken == varsym)
    {
        do
        {
            nodePointer = nodePointer->next;
            numVars++;
            numSymbols++;
            table[tIndex++].marked = 0;
            nodePointer = nodePointer->next;
        } while(nodePointer->typeToken == commasym);

        nodePointer = nodePointer->next;
    }

    if (nodePointer->typeToken == procsym)
    {
        do
        {
            nodePointer = nodePointer->next;
            numSymbols++;
            procIndex = tIndex;
            table[tIndex++].marked = 0;
            nodePointer = nodePointer->next->next;
            codeBlock(code, table, lexlevel + 1);
            emit(code, 2, 0, 0, 0);
            // table[procIndex].addr = cx;
            // printf("%d and %d\n", procIndex, cx);
            nodePointer = nodePointer->next;
        } while(nodePointer->typeToken == procsym);
    }

    if (procIndex != -1)
        table[procIndex].addr = cx;

    emit(code, 6, 0, 0, 3 + numVars);
    codeStatement(code, table, lexlevel);

    int i = tIndex - 1;

    while (numSymbols > 0)
    {
        if (!table[i].marked)
        {
            table[i--].marked = 1;
            numSymbols--;
        }
        else
        {
            i--;
        }
    }

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

    int i = 1;
    for (int j = 1; j < CODE_SIZE; j++)
    {
        if (table[j].kind == -1)
            break;

        if (table[j].kind == 3)
        {
            table[j].val = i++;
            emit(code, 7, 0, 0, 0);
        }
    }

    codeBlock(code, table, 0);

    i = 1;
    for (int j = 0; j < cx; j++)
    {
        if (code[j].opcode == 7)
        {
            for (int x = 0; x < CODE_SIZE; x++)
            {
                if (table[x].kind == -1)
                    break;

                if (table[x].kind == 3 && table[x].val == i)
                {
                    code[j].m = table[x].addr;
                    i++;
                    break;
                }
            }
        }
    }

    // for (int j = 0; j < cx; j++)
    // {
    //     if (code[j].opcode == 5)
    //     {
    //         for (int x = 1; x < CODE_SIZE; x++)
    //         {
    //             if (table[x].kind == -1)
    //                 break;
    //
    //             if (table[x].kind == 3 && table[x].val == code[j].m)
    //             {
    //                 code[j].m = table[x].addr;
    //                 break;
    //             }
    //         }
    //     }
    // }

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
