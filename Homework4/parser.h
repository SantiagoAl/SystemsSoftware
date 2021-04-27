#ifndef _PARSER_H
#define _PARSER_H

#include "lex.h"

typedef struct
{
    int kind; // const = 1, var = 2, proc = 3
    char name[12]; // name up to 11 chars
    int val; // number (ASCII value)
    int level; // L level
    int addr; // M address
    int marked;
}symbol;

symbol *parse(lexeme *list);

void block(symbol *table, int lexlevel);

int procedureDeclaration(symbol *table, int lexlevel);

int constDeclaration(symbol *table, int lexlevel);

int varDeclaration(symbol *table, int lexlevel);

void statement(symbol *table, int lexlevel);

void condition(symbol *table, int lexlevel);

void expression(symbol *table, int lexlevel);

void term(symbol *table, int lexlevel);

void factor(symbol *table, int lexlevel);

int findSymbol(symbol *table, char *name, int lexlevel);

int reverseFind(symbol *table, char *name);

int findProcedure(symbol *table, char *name);

int factorFind(symbol *table, char *name);

void addSymbol(symbol *table, int kind, char *savedName, int value, int lexLevel,
                int address, int marked);

#endif /* _PARSER_H */
