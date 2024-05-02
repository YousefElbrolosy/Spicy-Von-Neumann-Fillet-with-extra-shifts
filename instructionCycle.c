#include <stdio.h>
#include <stdlib.h>
// int NumberofInstructions = sizeof(instructionMemory) / sizeof(int);

typedef struct
{
    char regName[4];
    int regValue;
} Register;

typedef struct
{
    int opcode;
    int r1;
    int r2;
    int r3;
    int shamt;
    int imm;
    int address;
} DecodedValues{
    /* data */
};

Register *registerInit(int regCount)
{
    // Register zeroReg;
    // zeroReg.regName = "zReg";
    // zeroReg.regValue = 0;

    // /* using memcpy to copy string: */
    // char zeroRegName = "zReg";
    // memcpy(zeroReg.regName, zeroRegName, strlen(zeroRegName) + 1);
    // zeroReg.regValue = 0;

    // /* using memcpy to copy structure: */
    // memcpy(&person_copy, &person, sizeof(person));

    Register *registers = malloc(regCount * sizeof(Register));
    const Register ZERO_REGISTER = {"R0", 0};
    registers[0] = ZERO_REGISTER;
    for (int i = 1; i < regCount; i++)
    {
        Register reg;
        sprintf(reg.regName, "R%d", i);
        reg.regValue = 0;
        registers[i] = reg;
    }

    return registers;
}

// Global Initialisation
int mainMemory[2048];
int zeroFlag = 0;
Register PC = {"PC", 0};
Register *registerFile;
DecodedValues decodedValues;

// Binary int format in c is 0b00000000000000000000000000000000 (32 bits)
int fetch()
{
    int instruction = mainMemory[registerFile[32].regValue];
    registerFile[32].regValue++;
    return instruction;
}

void decode(int instruction)
{

    decodedValues.opcode = (instruction & 0b11110000000000000000000000000000) >> 28;
    decodedValues.r1 = (instruction & 0b00001111100000000000000000000000) >> 23;
    decodedValues.r2 = (instruction & 0b00000000011111000000000000000000) >> 18;
    decodedValues.r3 = (instruction & 0b00000000000000111110000000000000) >> 13;
    decodedValues.shamt = (instruction & 0b00000000000000000001111111111111);
    decodedValues.imm = (instruction & 0b00000000000000111111111111111111);
    decodedValues.address = (instruction & 0b00001111111111111111111111111111);
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
    registerFile = registerInit(32);
    printf("%d", registerFile[32].regValue);
    fetch(registerFile);
    printf("%d", registerFile[32].regValue);
    return 0;
}
