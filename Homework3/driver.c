// Alex Santiago
// UCF ID: 3216767
// COP 3402 Fall 2020

// This is my final solution for programming assignment #3

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "lex.h"
#include "parser.h"
#include "codegen.h"
#include "vm.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Error: Please include file name\n");
        return 1;
    }

    FILE *filePointer = fopen(argv[1], "r");

    if (filePointer == NULL)
    {
        printf("Failed to open input file\n");
        return 1;
    }

    lexeme *list = lex_analyze(filePointer);

    // At this point, the lexeme list being 'NULL' would indicate that an error
    // occured while scanning
    if (list == NULL)
    {
        fclose(filePointer);
        return 1;
    }

    printf("\n");
    symbol *table = parse(list);

    // At this point, the symbol table being 'NULL' would indicate that an error
    // occured while parsing
    if (table == NULL)
    {
        // Free all of the memory associated with the linked list before terminating
        list = destroyLinkedList(list);
        // Close the input file
        fclose(filePointer);
        return 1;
    }

    printf("No errors, program is syntactically correct\n");

    printf("\n");
    instruction *code = generate_code(table, list);

    for (int i = 2; i < argc; i++)
    {
        if (!strcmp(argv[i], "-l"))
        {
            printf("Lexeme List:\n");
            printList(list);
        }

        if (!strcmp(argv[i], "-a"))
            virtual_machine(code, 1);

        if (!strcmp(argv[i], "-v"))
            virtual_machine(code, 0);
    }

    // Free all of the memory associated with the linked list used for
    // the lexeme list
    list = destroyLinkedList(list);
    // Free all of the memory associated with the symbol table
    free(table);
    // Free all of the memory associated with the code generated
    free(code);

    // Close the input file
    fclose(filePointer);

    return 0;
}
