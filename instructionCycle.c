#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    int mainMemory[2048];
    int memRead;
    int memWrite;
} MainMemory;

typedef struct
{
    char regName[4];
    int regValue;
} Register;

typedef struct
{
    Register *registerArray;
    int regWrite;
    int readReg1;
    int readReg2;
} RegisterFile;

typedef struct
{
    int branch;
    int memRead;
    int memWrite;
    int memtoReg;
    int ALUSrc;
    int regWrite;
} ControlUnit;

typedef struct
{
    int opcode;
    int r1;
    int r2;
    int r3;
    int shamt;
    int imm;
    int address;
} DecodedValues;

// Global Initialisation
MainMemory mainMemory;
RegisterFile registerFile;
Register PC = {"PC", 0};
DecodedValues decodedValues;
ControlUnit CU;
int zeroFlag = 0;

Register *registerInit(int regCount)
{
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

// Binary int format in c is 0b00000000000000000000000000000000 (32 bits)

char **mySplit(char *str)
{

    char **storeSplit = (char **)malloc(sizeof(char *) * strlen(str));
    int counter = 0;

    // Initializes all the string[] values to "" so when the string
    // and char concatenates, 'null' doesn't appear.
    for (int i = 0; i < strlen(str); i++)
    {
        storeSplit[i] = (char *)malloc(sizeof(char) * 2);
        storeSplit[i][0] = '\0';
    }

    // Puts the str values into the split array and concatenates until
    // a delimiter is found, then it moves to the next array index.
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] != ' ')
        {
            char temp[2];
            temp[0] = str[i];
            temp[1] = '\0';
            strcat(storeSplit[counter], temp);
        }
        else
        {
            ++counter;
            // Move to the next index
            storeSplit[counter] = (char *)malloc(sizeof(char) * 2);
            storeSplit[counter][0] = '\0';
        }
    }

    return storeSplit;
}

void freeSplit(char **split, int numTokens)
{
    for (int i = 0; i < numTokens; i++)
    {
        free(split[i]);
    }
    free(split);
}

int parse()
{
    char fileName[] = "MIPS.txt";
    FILE *input;
    // Open a file in read mode
    input = fopen("filename.txt", "r");
    char instruction[100];
    int instructionType;
    while (fgets(instruction, 100, input))
    {
        char **split = mySplit(instruction);
        // what to do with first line
        for (int i = 0; i < strlen(instruction); i++)
        {

            switch (instruction[0])
            {

            case 'ADD':
                instructionType = 0b0000;
                break;
            case 'SUB':
                instructionType = 0b0001;
                break;
            case 'MULI':
                instructionType = 0b0010;
                break;
            case 'ADDI':
                instructionType = 0b0011;
                break;
            case 'BNE':
                instructionType = 0b0100;
                break;
            case 'ANDI':
                instructionType = 0b0101;
                break ;
            case 'ORI':
                instructionType = 0b0110;
                break;
            case 'J':
                instructionType = 0b0111;
                break;
            case 'SLL':
                instructionType = 0b1000;
                break;
            case 'SRL':
                instructionType = 0b1001;
                break;
            case 'LW':
                instructionType = 0b1010;
                break;
            case 'SW':
                instructionType = 0b1011;
                break;
            };
        }
    }

    char operation[5];
}

int fetch()
{
    int instruction = mainMemory.mainMemory[PC.regValue];
    PC.regValue++;
    return instruction;
}

void decode1(int instruction)
{

    decodedValues.opcode = (instruction & 0b11110000000000000000000000000000) >> 28;
    decodedValues.r1 = (instruction & 0b00001111100000000000000000000000) >> 23;
    decodedValues.r2 = (instruction & 0b00000000011111000000000000000000) >> 18;
    decodedValues.r3 = (instruction & 0b00000000000000111110000000000000) >> 13;
    decodedValues.shamt = (instruction & 0b00000000000000000001111111111111);
    decodedValues.imm = (instruction & 0b00000000000000111111111111111111);
    decodedValues.address = (instruction & 0b00001111111111111111111111111111);
}

void controlUnitSignals(int opcode)
{
    CU.memRead = opcode == 10;
    CU.memWrite = opcode == 5;
    CU.ALUSrc = opcode == 2 || opcode == 3 || opcode == 5 || opcode == 6 || opcode == 9 || opcode == 10;
    CU.regWrite = opcode == 0 || opcode == 1 || opcode == 2 || opcode == 3 || opcode == 5 || opcode == 6 || opcode == 8 || opcode == 9 || opcode == 10;
    CU.memtoReg = opcode == 11;
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
    registerFile.registerArray = registerInit(32);
    printf("%d", registerFile.registerArray[32].regValue);
    fetch(registerFile);
    printf("%d", registerFile.registerArray[32].regValue);
    return 0;
}
