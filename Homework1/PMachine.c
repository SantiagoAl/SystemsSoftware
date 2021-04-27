// Alex Santiago
// UCF ID: 3216767

// This is my final solution for programming assignment #1

#include "stdio.h"
#include "string.h"
#include "stdlib.h"

// This is what is used to emulate the 'text segment' as well as the
// 'Instruction Register' (IR)
typedef struct
{
    int OP;
    int R;
    int L;
    int M;
} Instructions;

const int MAX_STACK_HEIGHT = 1000;
const int MAX_CODE_LENGTH  = 500;
int arBIndex = 999;

// This function will help to find a variable in a different activation record
// some 'L' levels up
int base(int L, int base, int *stack)
{
    int b1; // find base L levels up
    b1 = base;

    while (L > 0)
    {
        b1 = stack[b1];
        L--;
    }

    return b1;
}

// This is a helper funciton to print out all of the values for the register
// file, pc, bp, sp, what instruciton we are currently on and the contents of
// the stack
void printValues(int *stack, int *rf, int pc, int bp, int sp, char *instr,
    Instructions IR, int prevPc, int *arBound, int flag)
{
    // This flag is used when printing out the initial values
    if (flag)
    {
        printf("Initial Values");
        printf("\t pc: %d  bp: %d  sp: %d\n", pc, bp, sp);
        printf("Registers: ");
        for (int i = 0; i < 8; i++)
            printf("%d", rf[i]);
        printf("\n");
        printf("Stack ");
        for (int i = 0; i < 40; i++)
            printf("%d ", stack[i]);
        printf("\n");
        return;
    }

    // Print to the screen the instruction, 'pc', 'bp', and 'cp'
    printf("%-4d", prevPc);
    printf("%s  %d  %d  %d", instr, IR.R, IR.L, IR.M);
    printf("\t pc: %d  bp: %d  sp: %d\n", pc, bp, sp);

    // Print out the values in the register to the screen
    printf("Registers:");
    for (int i = 0; i < 8; i++)
        printf("%d ", rf[i]);
    printf("\n");

    // Print out the values that are in the stack to the screen
    printf("Stack: ");
    int j = MAX_STACK_HEIGHT - 1;
    for (int i = MAX_STACK_HEIGHT - 1; i >= sp; i--)
    {
        // This is used to separate the activation records
        if (i == arBound[j])
        {
            j--;
            printf("| ");
        }

        printf("%d ", stack[i]);
    }
    printf("\n");
}

// The main function of the program
int main(int argc, char **argv)
{
    // Check that a file was passed in through the command line
    if (argc < 2)
    {
        printf("Please provide a file with test cases\n");
        return 1;
    }

    FILE *filePointer; // Declaration of the file Pointer

    // The stack that will be used, with all inidices initialized to zero
    int STACK[MAX_STACK_HEIGHT];

    // Initialize the stack indices to zero to avoid accessing garbage values
    for (int i = 0; i < MAX_STACK_HEIGHT; i++)
        STACK[i] = 0;

    // Declare and initialize the PM/0 registers
    int SP = MAX_STACK_HEIGHT;
    int BP = SP - 1;
    int PC = 0;
    Instructions IR;

    // Variable to break out of loop once the program 'Halts'
    int HALT = 1;

    // This is the register file with eight registers all intialized to zero
    int RF[8] = {0};

    // This is the 'text' segment that will hold all of the instructions
    // passed in through the input file
    Instructions IM[MAX_CODE_LENGTH];

    // This array will hold the indices for the AR bound
    int arBound[MAX_CODE_LENGTH];

    // Initialize all of the instructions to zero, to avoid any runtime
    // complications
    for (int i = 0; i < MAX_CODE_LENGTH; i++)
    {
        IM[i].OP = 0;
        IM[i].R = 0;
        IM[i].L = 0;
        IM[i].M = 0;
        arBound[i] = 0;
    }

    // An array of strings containing the instructions
    char instructions[22][4] =
    {
        "lit", "rtn", "lod", "sto", "cal", "inc", "jmp", "jpc", "sio", "neg",
        "add", "sub", "mul", "div", "odd", "mod", "eql", "neq", "lss", "leq",
        "gtr", "geq"
    };

    // Open the file passed in to the program
    filePointer = fopen(argv[1], "r");

    // If the file failed to open, return 1 and terminate the program
    if (filePointer == NULL)
    {
        printf("Failed to open input file\n");
        return 1;
    }

    int j = 0; // Variable for initializing the 'text' segment indices

    // Read instructions from the file while there are still instructions left
    // to read and store them in the instruction memory array
    while (!feof(filePointer))
    {
        fscanf(filePointer, "%d %d %d %d", &IM[j].OP, &IM[j].R, &IM[j].L,
                &IM[j].M);
        j++;
    }

    printf("Displaying input instructions: \n");

    printf("Line  OP   R   L   M\n");

    for (int i = 0; i < j; i++)
    {
        printf("%-5d", i);
        printf("%s", *(instructions + (IM[i].OP - 1)));
        printf("%4d", IM[i].R);
        printf("%4d", IM[i].L);
        printf("%4d\n", IM[i].M);
    }

    printf("\n");
    printValues(STACK, RF, PC, BP, SP, *(instructions + IM[0].OP - 1), IR, 0,
                arBound, 1);

    while (HALT == 1)
    {
        // Grab the next instruction from the 'text' segment
        IR = IM[PC];
        int prevPc = PC; // Used to print out line numbers
        // Move the 'PC' forward to get the next instruction
        PC = PC + 1;
        int input; // This variable is used for user input

        switch (IR.OP) {
            case 1: // LIT instruction
                // Store in 'RF' the value at 'M' at location 'RF[R]'
                RF[IR.R] = IR.M;
                break;
            case 2: // RTN instructions
                // Move 'SP' back
                SP = BP + 1;
                BP = STACK[SP - 2]; // 'BP' grabs the dynamic link
                PC = STACK[SP - 3]; // 'PC' gets the return address
                arBound[arBIndex] = 0;
                arBIndex++;
                break;
            case 3: // LOD instruction
                // Load into 'RF[R]' that is at 'L' activation records back,
                // with offset 'M'
                RF[IR.R] = STACK[base(IR.L, BP, STACK) - IR.M];
                break;
            case 4: // STO instruction
                // Store into the stack at 'L' activation records back, with
                // offset 'M', at 'RF[R]' (Register File)
                STACK[base(IR.L, BP, STACK) - IR.M] = RF[IR.R];
                break;
            case 5: // CAL instruction
                STACK[SP - 1] = base(IR.L, BP, STACK); // Static Link (SL)
                STACK[SP - 2] = BP; // Dynamic Link (DL)
                STACK[SP - 3] = PC; // Return address (RA)
                BP = SP - 1; // Move the 'BP' forward
                PC = IR.M; // 'PC' (Program Counter) gets value at 'M'
                arBound[arBIndex] = BP;
                arBIndex--;
                break;
            case 6: // INC instruction
                // Here we increment the 'SP' according the the value at 'M'
                SP = SP - IR.M;
                break;
            case 7: // JMP instruction
                // Here we move 'PC' to the value specified by 'M'
                PC = IR.M;
                break;
            case 8: // JPC instruction
                // This is the conditional jump. If the value at 'RF[R]' is
                // equal to zero, then we move the 'PC' to the value specified
                // at 'M'
                if (RF[IR.R] == 0)
                    PC = IR.M;
                break;
            case 9: // SYS instruction
                // If 'M' is 1, we print the value of 'RF[R]' to the screen
                if (IR.M == 1)
                    printf("\nThe value is: %d\n", RF[IR.R]);
                // If 'M' is 2, we take in input from the keyboard
                else if (IR.M == 2)
                {
                    printf("Enter an integer value: ");
                    scanf("%d", &input);
                    printf("\n");
                    RF[IR.R] = input;
                }
                // Else 'M' would be three and we want to HALT the program
                else
                    HALT = 0;
                break;
            case 10: // NEG instruction
                // Negate the value at 'RF[R]'
                RF[IR.R] = RF[IR.R] * -1;
                break;
            case 11: // ADD instruction
                // Add 'RF[L]' and 'RF[R]', then store at 'RF[R]'
                RF[IR.R] = RF[IR.L] + RF[IR.M];
                break;
            case 12: // SUB instruction
                // Substract 'RF[L]' and 'RF[R]', then store at 'RF[R]'
                RF[IR.R] = RF[IR.L] - RF[IR.M];
                break;
            case 13: // MUl instruction
                // Multiply 'RF[L]' and 'RF[R]', then store at 'RF[R]'
                RF[IR.R] = RF[IR.L] * RF[IR.M];
                break;
            case 14: // DIV instruction
                // Divide 'RF[L]' and 'RF[R]', then store at 'RF[R]'
                RF[IR.R] = RF[IR.L] / RF[IR.M];
                break;
            case 15: // ODD instruction
                // Determine if the value at 'RF[R]' is odd
                RF[IR.R] = RF[IR.R] % 2;
                break;
            case 16: // MOD instruction
                // // MOD 'RF[L]' and 'RF[R]', then store at 'RF[R]'
                RF[IR.R] = RF[IR.L] % RF[IR.M];
                break;
            case 17: // EQL instruction
                // Determine if the values at 'RF[L]' and 'RF[M]' are equal
                // then store that at location 'RF[R]'
                RF[IR.R] = (RF[IR.L] == RF[IR.M]);
                break;
            case 18: // NEQ instruction
                // Determine if the values at 'RF[L]' and 'RF[M]' are not equal
                // then store that at location 'RF[R]'
                RF[IR.R] = (RF[IR.L] != RF[IR.M]);
                break;
            case 19: // LSS instruction
                // Determine if the value at 'RF[L]' is less than 'RF[M]'
                // then store that at location 'RF[R]'
                RF[IR.R] = (RF[IR.L] < RF[IR.M]);
                break;
            case 20: // LEQ instruction
                // Determine if the value at 'RF[L]' is less than or equal to
                // 'RF[M]' then store that at location 'RF[R]'
                RF[IR.R] = (RF[IR.L] <= RF[IR.M]);
                break;
            case 21: // GTR instruction
                // Determine if the value at 'RF[L]' is greater than 'RF[M]'
                // then store that at location 'RF[R]'
                RF[IR.R] = (RF[IR.L] > RF[IR.M]);
                break;
            case 22: // GEQ instruction
                // Determine if the value at 'RF[L]' is greater than or equal to
                // 'RF[M]', then store at location 'RF[R]'
                RF[IR.R] = (RF[IR.L] >= RF[IR.M]);
                break;
            default:
                printf("No instruction\n");
        }

        // After each instruction has been executed, print the state of all the
        // segments in the VM
        printf("\n");
        printValues(STACK, RF, PC, BP, SP, *(instructions + IR.OP - 1), IR,
                    prevPc, arBound, 0);
    }

    // Make sure to close the file before terminating. Because you know...
    // Memory Leaks!
    fclose(filePointer);

    return 0;
}
