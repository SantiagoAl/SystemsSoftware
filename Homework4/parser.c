#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "parser.h"
#include "lex.h"

int parserError = 0;
int tableIndex = 0;
lexeme *nodePointer;

void addSymbol(symbol *table, int kind, char *savedName, int value, int lexlevel,
                int address, int marked)
{
    table[tableIndex].kind = kind;
    strcpy(table[tableIndex].name, savedName);
    table[tableIndex].val = value;
    table[tableIndex].level = lexlevel;
    table[tableIndex].addr = address;
    table[tableIndex++].marked = marked;
}

int findSymbol(symbol *table, char *identName, int lexlevel)
{
    for (int i = 0; i < tableIndex; i++)
    {
        if (!strcmp(table[i].name, identName) && !table[i].marked
            && table[i].level == lexlevel)
            return i;
    }

    return -1;
}

int reverseFind(symbol *table, char *name)
{
    for (int i = tableIndex - 1; i >= 0; i--)
    {
        if (!strcmp(table[i].name, name) && !table[i].marked)
            return i;
    }

    return -1;
}

int findProcedure(symbol *table, char *name)
{
    for (int i = tableIndex - 1; i >= 0; i--)
    {
        if (table[i].kind == 3 && !strcmp(table[i].name, name) && !table[i].marked)
            return i;
    }

    return -1;
}

int factorFind(symbol *table, char *name)
{
    for (int i = tableIndex - 1; i >= 0; i--)
    {
        if (table[i].kind == 2 && !strcmp(table[i].name, name) && !table[i].marked)
            return i;

        if (table[i].kind == 1 && !strcmp(table[i].name, name) && !table[i].marked)
            return i;
    }

    return -1;
}

void factor(symbol *table, int lexlevel)
{
    if (nodePointer->typeToken == identsym)
    {
        char identName[12];
        strcpy(identName, nodePointer->name);

        int foundVal = factorFind(table, identName);

        if (foundVal == -1)
        {
            parserError = 1;
            printf("Error: use of undeclared identifier '%s'\n", identName);
            return;
        }

        nodePointer = nodePointer->next;
    }
    else if (nodePointer->typeToken == numbersym)
    {
        nodePointer = nodePointer->next;
    }
    else if (nodePointer->typeToken == lparentsym)
    {
        nodePointer = nodePointer->next;
        expression(table, lexlevel);

        if (parserError)
            return;

        if (nodePointer->typeToken != rparentsym)
        {
            parserError = 1;
            printf("Error: Right parenthesis is missing\n");
            return;
        }

        nodePointer = nodePointer->next;
    }
    else
    {
        parserError = 1;
        printf("Error: missing expression\n");
        return;
    }

    return;
}

void term(symbol *table, int lexlevel)
{
    factor(table, lexlevel);

    if (parserError)
        return;

    while (nodePointer->typeToken == multsym || nodePointer->typeToken == slashsym)
    {
        nodePointer = nodePointer->next;
        factor(table, lexlevel);

        if (parserError)
            return;
    }
    return;
}

void expression(symbol *table, int lexlevel)
{
    if (nodePointer->typeToken == plussym || nodePointer->typeToken == minussym)
        nodePointer = nodePointer->next;

    term(table, lexlevel);

    if (parserError)
        return;

    while (nodePointer->typeToken == plussym || nodePointer->typeToken == minussym)
    {
        nodePointer = nodePointer->next;
        term(table, lexlevel);

        if (parserError)
            return;
    }

    return;
}

void condition(symbol *table, int lexlevel)
{
    if (nodePointer->typeToken == oddsym)
    {
        // Get next Token
        nodePointer = nodePointer->next;
        expression(table, lexlevel);

        if (parserError)
            return;
    }
    else
    {
        expression(table, lexlevel);

        if (parserError)
            return;

        int type = nodePointer->typeToken;

        if (type == eqsym)
        {
        }
        else if (type == neqsym)
        {
        }
        else if (type == lessym)
        {
        }
        else if (type == leqsym)
        {
        }
        else if (type == gtrsym)
        {
        }
        else if (type == geqsym)
        {
        }
        else
        {
            parserError = 1;
            printf("Error: expected relational operator\n");
            return;
        }

        nodePointer = nodePointer->next;
        expression(table, lexlevel);

        if (parserError)
            return;
    }
    return;
}

void statement(symbol *table, int lexlevel)
{
    if (nodePointer->typeToken == identsym)
    {
        char identName[12];
        strcpy(identName, nodePointer->name);
        int foundVal = reverseFind(table, identName);

        if (foundVal == -1)
        {
            parserError = 1;
            printf("Error: use of undeclared identifier '%s'\n", identName);
            return;
        }

        if (table[foundVal].kind != 2)
        {
            parserError = 1;
            printf("Error: cannot assign to variable '%s' with const-qualified type\n", identName);
            return;
        }

        // Get next Token
        nodePointer = nodePointer->next;

        if (nodePointer->typeToken != becomesym)
        {
            parserError = 1;
            printf("Error: Assignment operator expected\n");
            return;
        }

        // Get next Token
        nodePointer = nodePointer->next;
        expression(table, lexlevel);

        if (parserError)
            return;

        return;
    }

    if (nodePointer->typeToken == callsym)
    {
        // Get next Token
        nodePointer = nodePointer->next;

        char procName[12];
        strcpy(procName, nodePointer->name);

        if (findProcedure(table, procName) == -1)
        {
            parserError = 1;
            printf("Error: Could not identify procedure '%s'\n", procName);
            return;
        }

        // Get next Token
        nodePointer = nodePointer->next;

        return;
    }

    if (nodePointer->typeToken == beginsym)
    {
        // Get next Token
        nodePointer = nodePointer->next;
        statement(table, lexlevel);

        if (parserError)
            return;

        while (nodePointer->typeToken == semicolonsym)
        {
            nodePointer = nodePointer->next;
            statement(table, lexlevel);

            if (parserError)
                return;
        }

        if (nodePointer->typeToken != endsym)
        {
            parserError = 1;
            printf("Error: expected 'end'\n");
            return;
        }

        // Get next Token
        nodePointer = nodePointer->next;
        return;
    }

    if (nodePointer->typeToken == ifsym)
    {
        // Get next Token
        nodePointer = nodePointer->next;
        condition(table, lexlevel);

        if (parserError)
            return;

        if (nodePointer->typeToken != thensym)
        {
            parserError = 1;
            printf("Error: 'then' expected\n");
            return;
        }

        // Get next Token
        nodePointer = nodePointer->next;
        statement(table, lexlevel);

        if (parserError)
            return;

        if (nodePointer->typeToken == elsesym)
        {
            // Get next Token
            nodePointer = nodePointer->next;
            statement(table, lexlevel);

            if (parserError)
                return;
        }

        return;
    }

    if (nodePointer->typeToken == whilesym)
    {
        // Get next Token
        nodePointer = nodePointer->next;
        condition(table, lexlevel);

        if (parserError)
            return;

        if (nodePointer->typeToken != dosym)
        {
            parserError = 1;
            printf("Error: 'do' expected\n");
            return;
        }
        nodePointer = nodePointer->next;
        statement(table, lexlevel);

        if (parserError)
            return;

        return;
    }

    if (nodePointer->typeToken == readsym)
    {
        nodePointer = nodePointer->next;

        if (nodePointer->typeToken != identsym)
        {
            parserError = 1;
            printf("Error: expected identifier\n");
            return;
        }

        char symbolName[12];
        strcpy(symbolName, nodePointer->name);
        int foundIt = reverseFind(table, symbolName);

        if (foundIt == -1)
        {
            parserError = 1;
            printf("Error: use of undeclared identifier '%s'\n", symbolName);
            return;
        }

        if (table[foundIt].kind != 2)
        {
            parserError = 1;
            printf("Error: 'const', 'var', 'procedure' must by followed by"
                    " an identifer\n");
            return;
        }

        nodePointer = nodePointer->next;
        return;
    }

    if (nodePointer->typeToken == writesym)
    {
        // Get next Token
        nodePointer = nodePointer->next;

        expression(table, lexlevel);

        if (parserError)
            return;

        return;
    }

    return;
}

int procedureDeclaration(symbol *table, int lexlevel)
{
    int numProc = 0;

    if (nodePointer->typeToken == procsym)
    {
        do
        {
            // Increment the number of procedures
            numProc++;
            // Get next Token
            nodePointer = nodePointer->next;

            if (nodePointer->typeToken != identsym)
            {
                parserError = 1;
                printf("Error: 'const', 'var', 'procedure' must be followed"
                        " by an identifier\n");
                return 0;
            }

            // Save ident name
            char identName[12];
            strcpy(identName, nodePointer->name);

            int foundVal = findSymbol(table, identName, lexlevel);
            // Check if the identifier is already in the symbol table
            if (foundVal != -1)
            {
                parserError = 1;
                printf("Error: Duplicate Identifier\n");
                return 0;
            }

            // Add procedure to the symbol table
            addSymbol(table, 3, identName, 0, lexlevel, 0, 0);

            // Get next Token
            nodePointer = nodePointer->next;

            if (nodePointer->typeToken != semicolonsym)
            {
                parserError = 1;
                printf("Error: Expected Semicolon\n");
                return 0;
            }

            // Get next Token
            nodePointer = nodePointer->next;

            block(table, lexlevel);

            // If for some reason there is an error when coming back from block
            // then just keep returning
            if (parserError) { return 0; }

            if (nodePointer->typeToken != semicolonsym)
            {
                parserError = 1;
                printf("Error: Expected Semicolon\n");
                return 0;
            }

            // Get next Token
            nodePointer = nodePointer->next;

        } while(nodePointer->typeToken == procsym);
    }

    // Return the number of procedures
    return numProc;
}

int varDeclaration(symbol *table, int lexlevel)
{
    int numVars = 0;

    if (nodePointer->typeToken == varsym)
    {
        do
        {
            // Increment the number of vars
            numVars++;
            // Get next Token
            nodePointer = nodePointer->next;
            // Check for grammatical errors
            if (nodePointer->typeToken != identsym)
            {
                parserError = 1;
                printf("Error: 'const', 'var', 'procedure' must be followed"
                        " by an identifier\n");
                return 0;
            }

            // Save ident name
            char identName[12];
            strcpy(identName, nodePointer->name);

            int foundVal = findSymbol(table, identName, lexlevel);
            // Check if this identifier is already in the symbol table
            if (foundVal != -1)
            {
                parserError = 1;
                printf("Error: Duplicate Identifier\n");
                return 0;
            }

            addSymbol(table, 2, identName, 0, lexlevel, numVars + 2, 0);
            // Get next Token
            nodePointer = nodePointer->next;
        } while(nodePointer->typeToken == commasym);

        // Check for grammatical errors
        if (nodePointer->typeToken != semicolonsym)
        {
            parserError = 1;
            printf("Error: 'semicolon' or 'comma' missing\n");
            return 0;
        }

        // Get next Token
        nodePointer = nodePointer->next;
    }

    return numVars;
}

int constDeclaration(symbol *table, int lexlevel)
{
    int numConst = 0;

    if (nodePointer->typeToken == constsym)
    {
        do
        {
            // Get next Token
            nodePointer = nodePointer->next;

            // Check if there has been en error
            if (nodePointer->typeToken != identsym)
            {
                parserError = 1;
                printf("Error: 'const' must be followed by an identifier\n");
                return 0;
            }

            // Save identName
            char identName[12];
            strcpy(identName, nodePointer->name);

            int foundVal = findSymbol(table, identName, lexlevel);
            // Check if this identifier is already in the symbol table
            if (foundVal != -1)
            {
                parserError = 1;
                printf("Error: Duplicate Identifier\n");
                return 0;
            }

            // Get next Token
            nodePointer = nodePointer->next;

            // Check for grammatical errors
            if (nodePointer->typeToken != eqsym)
            {
                parserError = 1;
                printf("Error: Identifier must be followed by '=' for const\n");
                return 0;
            }

            // Get next Token
            nodePointer = nodePointer->next;
            // Check for grammatical errors
            if (nodePointer->typeToken != numbersym)
            {
                parserError = 1;
                printf("'=' must be followed by a number\n");
                return 0;
            }

            addSymbol(table, 1, identName, nodePointer->numValue, lexlevel, 0, 0);

            // Increment the number of consts in the symbol Table at this line
            numConst++;

            // Get next Token
            nodePointer = nodePointer->next;

        } while(nodePointer->typeToken == commasym);

        // Check for grammatical errors
        if (nodePointer->typeToken != semicolonsym)
        {
            parserError = 1;
            printf("Error: 'semicolon' or 'comma' missing\n");
            return 0;
        }

        // Get next Token
        nodePointer = nodePointer->next;
    }

    // Return the number of consts
    return numConst;
}

void block(symbol *table, int lexlevel)
{
    int numSymbols = 0;

    // Check grammar for const-declaration
    numSymbols += constDeclaration(table, lexlevel);
    // If we encountered an error return to 'parse'
    if (parserError)
        return;
    // Check grammar for var-declaration
    numSymbols += varDeclaration(table, lexlevel);
    // If we encountered an error return to 'parse'
    if (parserError)
        return;
    // Check grammar for procedure declaration
    numSymbols += procedureDeclaration(table, lexlevel + 1);
    // If we encounter an error, return to 'parse'
    if (parserError)
        return;

    statement(table, lexlevel);

    int i = tableIndex - 1;

    while (numSymbols > 0 && i >= 0)
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

symbol *parse(lexeme *list)
{
    symbol *table = malloc(500 * sizeof(symbol));

    // Check to make sure that dynamic memory allocation worked
    if (table == NULL)
    {
        printf("Dynamic memory allocation failed in 'parser'\n");
        return NULL;
    }

    char identName[12];
    strcpy(identName, "main");

    addSymbol(table, 3, identName, 0, 0, 0, 0);

    nodePointer = list;

    block(table, 0);

    if (parserError)
    {
        free(table);
        return NULL;
    }

    if (nodePointer == NULL || nodePointer->typeToken != periodsym)
    {
        free(table);
        printf("Error: Period Expected\n");
        return NULL;
    }

    if (tableIndex < 499)
    {
        table[tableIndex].kind = -1;
        strcpy(table[tableIndex].name, "end");
        table[tableIndex].val = 0;
        table[tableIndex].level = 0;
        table[tableIndex].addr = 0;
    }

    return table;
}
