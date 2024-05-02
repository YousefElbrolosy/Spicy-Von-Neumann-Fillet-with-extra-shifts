#include <stdio.h>

// int NumberofInstructions = sizeof(instructionMemory) / sizeof(int);
int mainMemory[2048];
Register registerFile[];
int zeroFlag = 0;

typedef struct
{
    char regName[4];
    int regValue;
} Register;

Register *registerInit(int regCount)
{
    Register *registers = malloc(regCount * sizeof(Register));
    for (int i = 0; i < regCount; ++i)
    {
        Register reg;
        sprintf(reg.regName, "R%d", i);
        reg.regValue = 0;
        registers[i] = reg;
    }
    sprintf(registers[regCount - 1].regName, "PC");
    registers[regCount - 1].regValue = 0;
    return registers;
}



// Binary int format in c is 0b00000000000000000000000000000000 (32 bits)

void decode(int instruction)
{
}

void fetch()
{
}

int ALU(int operandA, int operandB, int operation)
{
    int output = 0;
    zeroFlag = 0;

    // Complete the ALU body here...
    switch (operation)
    {
    case 0:
        output = operandA >> operandB;
        break;
    case 1:
        output = operandA << operandB;
        break;

    case 2:
        output = operandA - operandB;
        break;
    case 3:
        output = operandA * operandB;
        break;
    case 4:
        output = operandA / operandB;
        break;
    case 5:
        if (operandA >= operandB)
        {
            output = 1;
        }
        break;
    case 6:
        output = ~(operandA & operandB);
        break;
    case 7:
        output = operandA + operandB;
        break;
    case 8:
        output = operandA ^ operandB;
        break;
    }

    if (output == 0)
    {
        zeroFlag = 1;
    }
    printf("Operation = %d\n", operation);
    printf("First Operand = %d\n", operandA);
    printf("Second Operand = %d\n", operandB);
    printf("Result = %d\n", output);
    printf("Zero Flag = %d\n", zeroFlag);

    return output;
}

int main()
{
    // initializing register file
    Register registerFile[] = registerInit(33);
    return 0;
}
