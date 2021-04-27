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
}symbol;

symbol *parse(lexeme *list);

void block(symbol *table);

void constDeclaration(symbol *table);

void varDeclaration(symbol *table);

void statement(symbol *table);

void condition(symbol *table);

void expression(symbol *table);

void term(symbol *table);

void factor(symbol *table);

int findSymbol(symbol *table, char *name);

void addSymbol(symbol *table, int kind, char *savedName, int value, int lexLevel,
                int address);

#endif /* _PARSER_H */
