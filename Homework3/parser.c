#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "parser.h"
#include "lex.h"

int parserError = 0;
int tableIndex = 0;
lexeme *nodePointer;

void addSymbol(symbol *table, int kind, char *savedName, int value, int lexLevel,
                int address)
{
    table[tableIndex].kind = kind;
    strcpy(table[tableIndex].name, savedName);
    table[tableIndex].val = value;
    table[tableIndex].level = lexLevel;
    table[tableIndex++].addr = address;
}

int findSymbol(symbol *table, char *identName)
{
    for (int i = 0; i < tableIndex; i++)
    {
        if (!strcmp(table[i].name, identName))
            return i;
    }

    return -1;
}

void factor(symbol *table)
{
    if (nodePointer->typeToken == identsym)
    {
        char identName[12];
        strcpy(identName, nodePointer->name);

        int foundVal = findSymbol(table, identName);

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
        expression(table);

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

void term(symbol *table)
{
    factor(table);

    if (parserError)
        return;

    while (nodePointer->typeToken == multsym || nodePointer->typeToken == slashsym)
    {
        nodePointer = nodePointer->next;
        factor(table);

        if (parserError)
            return;
    }
}

void expression(symbol *table)
{
    if (nodePointer->typeToken == plussym || nodePointer->typeToken == minussym)
        nodePointer = nodePointer->next;

    term(table);

    if (parserError)
        return;

    while (nodePointer->typeToken == plussym || nodePointer->typeToken == minussym)
    {
        nodePointer = nodePointer->next;
        term(table);

        if (parserError)
            return;
    }

    return;
}

void condition(symbol *table)
{
    if (nodePointer->typeToken == oddsym)
    {
        // Get next Token
        nodePointer = nodePointer->next;
        expression(table);

        if (parserError)
            return;
    }
    else
    {
        expression(table);

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
        expression(table);

        if (parserError)
            return;
    }
}

void statement(symbol *table)
{
    if (nodePointer->typeToken == identsym)
    {
        char identName[12];
        strcpy(identName, nodePointer->name);

        int foundVal = findSymbol(table, identName);

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
        expression(table);

        if (parserError)
            return;

        return;
    }

    if (nodePointer->typeToken == beginsym)
    {
        // Get next Token
        nodePointer = nodePointer->next;
        statement(table);

        if (parserError)
            return;

        while (nodePointer->typeToken == semicolonsym)
        {
            nodePointer = nodePointer->next;
            statement(table);

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
        condition(table);

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
        statement(table);

        return;
    }

    if (nodePointer->typeToken == whilesym)
    {
        // Get next Token
        nodePointer = nodePointer->next;
        condition(table);

        if (parserError)
            return;

        if (nodePointer->typeToken != dosym)
        {
            parserError = 1;
            printf("Error: 'do' expected\n");
            return;
        }
        nodePointer = nodePointer->next;
        statement(table);
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
        int foundIt = findSymbol(table, symbolName);

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

        if (nodePointer->typeToken != identsym)
        {
            parserError = 1;
            printf("Error: expected identifier\n");
            return;
        }

        char symbolName[12];
        strcpy(symbolName, nodePointer->name);
        int foundIt = findSymbol(table, symbolName);

        if (foundIt == -1)
        {
            parserError = 1;
            printf("Error: use of undeclared identifier '%s'\n", symbolName);
            return;
        }

        nodePointer = nodePointer->next;
        return;
    }

    return;
}

void varDeclaration(symbol *table)
{
    if (nodePointer->typeToken == varsym)
    {
        int numVars = 0;

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
                return;
            }

            // Save ident name
            char identName[12];
            strcpy(identName, nodePointer->name);

            int foundVal = findSymbol(table, identName);
            // Check if this identifier is already in the symbol table
            if (foundVal != -1)
            {
                parserError = 1;
                printf("Error: Duplicate Identifier\n");
                return;
            }

            addSymbol(table, 2, identName, 0, 0, numVars + 2);
            // Get next Token
            nodePointer = nodePointer->next;
        } while(nodePointer->typeToken == commasym);

        // Check for grammatical errors
        if (nodePointer->typeToken != semicolonsym)
        {
            parserError = 1;
            printf("Error: 'semicolon' or 'comma' missing\n");
            return;
        }

        // Get next Token
        nodePointer = nodePointer->next;
    }
}

void constDeclaration(symbol *table)
{
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
                return;
            }

            // Save identName
            char identName[12];
            strcpy(identName, nodePointer->name);

            int foundVal = findSymbol(table, identName);
            // Check if this identifier is already in the symbol table
            if (foundVal != -1)
            {
                parserError = 1;
                printf("Error: Duplicate Identifier\n");
                return;
            }

            // Get next Token
            nodePointer = nodePointer->next;

            // Check for grammatical errors
            if (nodePointer->typeToken != eqsym)
            {
                parserError = 1;
                printf("Error: Identifier must be followed by '=' for const\n");
                return;
            }

            // Get next Token
            nodePointer = nodePointer->next;
            // Check for grammatical errors
            if (nodePointer->typeToken != numbersym)
            {
                parserError = 1;
                printf("'=' must be followed by a number\n");
                return;
            }

            addSymbol(table, 1, identName, nodePointer->numValue, 0, 0);
            // Get next Token
            nodePointer = nodePointer->next;

        } while(nodePointer->typeToken == commasym);

        // Check for grammatical errors
        if (nodePointer->typeToken != semicolonsym)
        {
            parserError = 1;
            printf("Error: 'semicolon' or 'comma' missing\n");
            return;
        }

        // Get next Token
        nodePointer = nodePointer->next;
    }
}

void block(symbol *table)
{
    // Check grammar for const-declaration
    constDeclaration(table);
    // If we encountered an error return to 'parse'
    if (parserError)
        return;
    // Check grammar for var-declaration
    varDeclaration(table);
    // If we encountered an error return to 'parse'
    if (parserError)
        return;
    statement(table);

    return;
}

symbol *parse(lexeme *list)
{
    symbol *table = malloc(500 * sizeof(symbol));

    // Check to make sure that dynamic memory allocation worked
    if (table == NULL)
    {
        printf("Dynamic memory allocation failed in 'parse'\n");
        return NULL;
    }

    nodePointer = list;

    block(table);

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
