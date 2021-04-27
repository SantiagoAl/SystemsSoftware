#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"
#include "lex.h"

// This is a function to create a node in our linked list
lexeme *createNode(int data, char *buffer, int value)
{
    // Allocate memory for the new node
    lexeme *node = malloc(sizeof(lexeme));

    // If there is no memory for the node terminate the program
    if (node == NULL)
    {
        fprintf(stderr, "Error: Out of memory in createNode()\n");
        exit(1);
    }

    // Initialize the node's variables
    node->typeToken = 0;
    node->numValue = 0;
    node->next = NULL;

    // Data being a 2 indicates an identifier. Store the token type and the
    // identifier name
    if (data == 2)
    {
        node->typeToken = data;
        strcpy(node->name, buffer);
    }
    // Data being 3 indicates and number. Store the token type and value
    else if (data == 3)
    {
        node->typeToken = data;
        node->numValue = value;
    }
    // Else it is a reserved/keyword, just store the token type
    else
    {
        node->typeToken = data;
    }

    // Return the newly allocate node
    return node;
}

// This is a function which takes the tail pointer and inserts a new node in
// O(1) time. Returns a pointer to the new tail of the list
lexeme *insertToken(lexeme *tail, int data, char *buffer, int value)
{
    // Return a new node if the 'tail' is NULL
    if (tail == NULL)
        return createNode(data, buffer, value);
    // Insert at the end of the linked list, and return that node
    tail->next = createNode(data, buffer, value);
    return tail->next;
}

// Iteratively free all memory associated with this linked list. Returns NULL
// so I can easily set the head pointer in 'driver.c' back to NULL by calling
// this function
lexeme *destroyLinkedList(lexeme *head)
{
    if (head == NULL)
        return NULL;

    lexeme *current = head;
    lexeme *temp;

    // Iterate until the end of the linked list, freeing the memory as we go
    // along
    while (current != NULL)
    {
        temp = current->next;
        free(current);
        current = temp;
    }

    // Return NULL which will assign the head to 'NULL' back in 'driver.c'
    return NULL;
}

// This is a helper function to print the Lexeme List
void printList(lexeme *head)
{
    if (head == NULL)
    return;

    lexeme *temp = head;

    while (temp != NULL)
    {
        if (temp->typeToken == 2)
            printf("%d %s ", temp->typeToken, temp->name);
        else if (temp->typeToken == 3)
            printf("%d %d ", temp->typeToken, temp->numValue);
        else
            printf("%d ", temp->typeToken);

        temp = temp->next;
    }

    printf("\n");
}

// This function will check for reserved words, and return the value associated
// with it, if in fact 'buffer' is a reserved word
int isReservedWord(char buffer[])
{
    char keywords[14][10] = {"const", "var", "procedure", "call", "begin", "end",
                            "if", "then", "else", "while", "do", "read", "write",
                            "odd"};

    int flag = 0;
    int returnToken = 0;

    // Iterate through the reserved words and check if 'buffer' is there
    for (int i = 1; i <= 14; i++)
    {
        if (!strcmp(buffer, keywords[i - 1]))
        {
            flag = i;
            break;
        }
    }

    // Switch statement to help return the value associated with the reserved
    // word
    switch (flag)
    {
        // Case for a const
        case 1:
            returnToken = constsym;
            break;
        // Case for a variable
        case 2:
            returnToken = varsym;
            break;
        // Case for a procedure
        case 3:
            returnToken = procsym;
            break;
        // Case for 'call'
        case 4:
            returnToken = callsym;
            break;
        case 5:
            returnToken = beginsym;
            break;
        // Case for 'end'
        case 6:
            returnToken = endsym;
            break;
        // Case for 'if' statement
        case 7:
            returnToken = ifsym;
            break;
        // Case for 'then'
        case 8:
            returnToken = thensym;
            break;
        // Case for 'else'
        case 9:
            returnToken = elsesym;
            break;
        // Case for 'while'
        case 10:
            returnToken = whilesym;
            break;
        // Case for 'do'
        case 11:
            returnToken = dosym;
            break;
        // Case for 'read'
        case 12:
            returnToken = readsym;
            break;
        // Case for 'write'
        case 13:
            returnToken = writesym;
            break;
        // Case for 'odd'
        case 14:
            returnToken = oddsym;
            break;
        // The default would indicate that it is an indentifier
        default:
            returnToken = identsym;
            break;
    }
    // Return the value associated with the reserved word
    return returnToken;
}

// This is a helper function to check for a special character. 'buffer' contains
// the special character, and 'ch' is the character in question
int isSpecialCharacter(char ch, char *buffer)
{
    int flag = 0;

    // Check if what 'ch' is indeed a special character
    for (int i = 1; i <= 13; i++)
    {
        if (ch == buffer[i - 1])
        {
            flag = i;
            break;
        }
    }
    // Return the index of the character in 'buffer'. Zero if it is no present
    return flag;
}

lexeme *lex_analyze(FILE *input)
{
    // 'Ch' holds the characters in the file as we read them
    char ch;
    // A character array containing the special characters
    char specialSymbols[] = "+-*/()=,.<>;:";
    int error = 0; // This will indicate if the scanner encountered an error
    // This will indicate if we encounter a comment
    int comment = 0;

    // This will be the head of our linked list. There is also a pointer to the
    // tail of our linked list so we can achieve O(1) insertion
    lexeme *head = NULL, *tail = NULL;

    // Get the first character in the input program to kick off the scanning
    ch = fgetc(input);

    // This is the main loop that will be sanning the source program
    while (ch != EOF)
    {
        // If the character is a space or some other control character, get the
        // next character and continue on the the next iteration
        if ((isspace(ch) != 0) || iscntrl(ch) != 0)
        {
            ch = fgetc(input);
            continue;
        }

        // If 'ch' is an alphanumeric
        if (isalpha(ch))
        {
            // 'variableString' will take in the string from the source program.
            // It is initialized with 12 indices to leave at least one spot for
            // the termination character '\0'
            char variableString[12];
            memset(variableString, '\0', 12);
            int index = 0;
            variableString[index++] = ch;

            // While the character is alphanumeric or a digit, continuer looping
            while (isalpha(ch = fgetc(input)) || isdigit(ch))
            {
                if (index > 10)
                {
                    // Set error to 1 to indicate that we encounter a string
                    // whose length is to long
                    error = 1;

                    // Loop through the rest of the variable so we can move on
                    while (isalpha(ch = fgetc(input)) || isdigit(ch))
                        ;

                    break;
                }

                // Insert the character into 'variableString'
                variableString[index++] = ch;
            }

            // If there was en error, then the string was to long. Set error
            // back to zero, print out the error in the Lexeme Table and move
            // on to the next iteration
            if (error)
            {
                error = 0;
                head = destroyLinkedList(head);
                tail = NULL;
                printf("%-13s", variableString);
                printf("Error: Identifier to long\n");
                return NULL;
            }

            // Check if the string is a reserved word or not
            int reservedWord = isReservedWord(variableString);

            // Insert the token into the list
            tail = insertToken(tail, reservedWord, variableString, 0);
        }
        // Else if the beginning of the string is a digit
        else if (isdigit(ch))
        {
            // 'Value' will contain the value of the variable
            int value = ch - '0';
            // 'nextDigit' holds the next digit, and plaveValue is to indicate
            // if the number has become to long
            int nextDigit = 0, placeValue = 1;

            while (isdigit(ch = fgetc(input)))
            {
                if (placeValue > 4)
                {
                    // Set error to 1 to indicate that the number is too long
                    error = 1;

                    // Loop through the rest of the number so we can move on
                    while (isdigit(ch = fgetc(input)))
                    {
                        nextDigit = ch - '0';
                        value = 10 * value + nextDigit;
                        placeValue++;
                    }

                    break;
                }

                // Compose the number
                nextDigit = ch - '0';
                value = 10 * value + nextDigit;
                placeValue++;
            }

            // Here getting a letter would cause an error, as identifiers cannot
            // begin with digits
            if (isalpha(ch))
            {
                // Iterate throught the rest of the string, this is alread an error
                while (isalpha(ch = fgetc(input)) || isdigit(ch))
                    ;

                // An error of two indicates an identifier beginning with a letter
                error = 2;
            }

            // If there was an error then we just want to continue to the next
            // iteration of the loop
            if (error == 1)
            {
                error = 0;
                head = destroyLinkedList(head);
                tail = NULL;
                printf("%-13d", value);
                printf("%s\n", "Error: Number to long");
                return NULL;
            }

            // If there was an error to, print out the error
            if (error == 2)
            {
                error = 0;
                head = destroyLinkedList(head);
                tail = NULL;
                printf("%20s\n", "Error: Identifier cannot start with a letter");
                return NULL;
            }

            // Here we insert the information into the Linked List
            tail = insertToken(tail, numbersym, NULL, value);
        }
        // Else the character must be another type character
        else
        {
            // Check if the characters is a special symbol
            int specialSymbol = isSpecialCharacter(ch, specialSymbols);
            // 'type' is to hold the token type, 'forwardlook' indicates if we
            // have to look at the next character for more info, and 'nextPrint'
            // helps to inidicate if we have to print two characters
            int type = 0, forwardLook = 0, nextPrint = 0;
            // This holds the previous char, when we have to print two chars
            // for example, '<='
            char prevCh;

            // This will check if the character is actually a special character
            // and will also take into account comments
            switch(specialSymbol)
            {
                // Case for plus sign
                case 1:
                    type = plussym;
                    break;
                // Case for minus sign
                case 2:
                    type = minussym;
                    break;
                // Case for multiplication sign
                case 3:
                    type = multsym;
                    break;
                // Case for comments
                case 4:
                    forwardLook = 1;
                    nextPrint = 1;
                    prevCh = ch;
                    // Get the next character in the file
                    ch = fgetc(input);
                    // The means we encountered a comment
                    if (ch == '*')
                    {
                        comment = 1;
                        forwardLook = 0;
                        ch = fgetc(input);

                        // Keep reading while there is still comments left
                        while (comment)
                        {
                            if (ch == '*')
                            {
                                ch = fgetc(input);
                                // If we encounter a final '*' and '/', the
                                // the comment is done
                                if (ch == '/')
                                    comment = 0;
                            }
                            else
                            {
                                ch = fgetc(input);
                            }
                        }
                        // Set comment back to one at the end, to indicate we
                        // just finished a comment
                        comment = 1;
                    }
                    // Else the character is just a '/'
                    else
                    {
                        type = slashsym;
                        nextPrint = 0;
                    }
                    break;
                case 5:
                    // Case for '('
                    type = lparentsym;
                    break;
                case 6:
                    // Case for ')'
                    type = rparentsym;
                    break;
                case 7:
                    // Case for '='
                    type = eqsym;
                    break;
                case 8:
                    // Case for ','
                    type = commasym;
                    break;
                case 9:
                    // Case for '.'
                    type = periodsym;
                    break;
                // Case for '<'
                case 10:
                    forwardLook = 1;
                    nextPrint = 1;
                    prevCh = ch;
                    // Get the next character in the file
                    ch = fgetc(input);
                    // This indicates '<>'
                    if (ch == '>')
                    {
                        type = neqsym;
                        forwardLook = 0;
                    }
                    // This indicates'<='
                    else if (ch == '=')
                    {
                        type = leqsym;
                        forwardLook = 0;
                    }
                    // Else if just '<'
                    else
                    {
                        type = lessym;
                        nextPrint = 0;
                    }
                    // This is for printing out two characters in a string
                    // For example '<='
                    if (nextPrint)
                    {
                    }

                    break;
                // Case for '>'
                case 11:
                    forwardLook = 1;
                    nextPrint = 1;
                    prevCh = ch;
                    // Get the next character in the file
                    ch = fgetc(input);
                    // this indicates '>='
                    if (ch == '=')
                    {
                        type = geqsym;
                        forwardLook = 0;
                    }
                    // This indicates '>'
                    else
                    {
                        type = gtrsym;
                        nextPrint = 0;
                    }
                    // This is for printing out two characters in a string
                    // For example '>='
                    if (nextPrint)
                    {
                    }

                    break;
                // Case for ';'
                case 12:
                    type = semicolonsym;
                    break;
                // Case for ':'
                case 13:
                    forwardLook = 1;
                    prevCh = ch;
                    // get the next character in the file
                    ch = fgetc(input);
                    // Indicates ':='
                    if (ch == '=')
                    {
                        type = becomesym;
                        forwardLook = 0;
                        nextPrint = 1;
                    }
                    // Else there is an error, print it out to the Lexeme Table
                    else
                    {
                        printf("%-13c", prevCh);
                        printf("%s\n", "Error: Invalid Symbol");
                        error = 1;
                    }
                    // This is for printing out two characters in a string
                    // For example ':='
                    if (nextPrint)
                    {
                    }

                    break;
                default:
                    printf("%-13c", ch);
                    printf("%s\n", "Error: Invalid Symbol");
                    ch = fgetc(input);
                    error = 1;
                    break;
            }

            // If there was an error, we will store the error and continue
            // on to the next loop
            if (error)
            {
                head = destroyLinkedList(head);
                tail = NULL;
                error = 0;
                return NULL;
            }

            // This happens when we needed to look at the next character, but we
            // are going to use that character in the next iteration of the loop
            if (!nextPrint && forwardLook)
            {
            }
            // Else we did not need to look forward, just print out the character
            else if (!nextPrint)
            {
            }

            // Here, if we were not looking foward in the source program to
            // verify a special symbol or comments, then we want to get the next
            // character in the source program so we can continue scanning
            if (!forwardLook)
                ch = fgetc(input);

            // Here, we will insert the information into the Linked List, only if
            // we are not in the middle of the comment
            if (!comment)
                tail = insertToken(tail, type, NULL, 0);
        }

        // This is used initially, when the linked list is first created
        if (head == NULL)
            head = tail;
    }

    return head;
}
