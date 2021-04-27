#ifndef _LEX_H
#define _LEX_H

// This struct will help to represent the input file as a linked list
typedef struct lexeme
{
    int typeToken;
    int numValue;
    char name[12];
    struct lexeme *next;
}lexeme;

// This enum will give a numerical value for each token
typedef enum
{
    nulsym = 1, identsym, numbersym, plussym, minussym, multsym, slashsym,
    oddsym, eqsym, neqsym, lessym, leqsym, gtrsym, geqsym, lparentsym, rparentsym,
    commasym, semicolonsym, periodsym, becomesym, beginsym, endsym, ifsym,
    thensym, whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
    readsym, elsesym
} tokenType;

// This is a function to create a node in our linked list
lexeme *createNode(int data, char *buffer, int value);

lexeme *insertToken(lexeme *tail, int data, char *buffer, int value);

lexeme *destroyLinkedList(lexeme *head);

void printList(lexeme *head);

int isReservedWord(char buffer[]);

int isSpecialCharacter(char ch, char *buffer);

lexeme *lex_analyze(FILE *input);

#endif /* _LEX_H */
