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
    int readData1;
    int readData2;
    int writeData;
} RegisterFile;

typedef struct
{
    int branch;
    int memRead;
    int memWrite;
    int memToReg;
    int ALUsrc;
    int regWrite;
    int regDst;
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

// Global Initialization
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

int parseRegNum(char test[])
{
    char res[strlen(test) - 1];
    int intRes;
    for (int i = 1; i < strlen(test); i++)
    {
        res[i - 1] = test[i];
    }
    intRes = atol(res);
    return intRes;
}
int parseInt(char test[])
{
    int intRes;
    intRes = atol(test);
    return intRes;
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

void parse()
{
    // char fileName[] = "MIPS.txt";
    FILE *input;
    // Open a file in read mode
    char instruction[100];
    int instructionType;
    int registerSource;
    int registerDest;
    int registerTarget;
    int imm;
    int address;
    int shamt;
    char instType;
    
   
    input = fopen("filename.txt", "r");
    int c = 0;
    while (fgets(instruction, 100, input))
    {   
        char **instructionSplitted = mySplit(instruction);
        // what to do with first line
        if (strcmp(instructionSplitted[0], "ADD") == 0)
        {
            instructionType = 0b00000000000000000000000000000000;
            instType = 'R';
        }
        else if (strcmp(instructionSplitted[0], "SUB") == 0)
        {
            instructionType = 0b00010000000000000000000000000000;
            instType = 'R';
        }
        else if (strcmp(instructionSplitted[0], "MULI") == 0)
        {
            instructionType = 0b00100000000000000000000000000000;
            instType = 'I';
        }
        else if (strcmp(instructionSplitted[0], "ADDI") == 0)
        {
            instructionType = 0b00110000000000000000000000000000;
            instType = 'I';
        }
        else if (strcmp(instructionSplitted[0], "BNE") == 0)
        {
            instructionType = 0b01000000000000000000000000000000;
            instType = 'I';
        }
        else if (strcmp(instructionSplitted[0], "ANDI") == 0)
        {
            instructionType = 0b01010000000000000000000000000000;
            instType = 'I';
        }
        else if (strcmp(instructionSplitted[0], "ORI") == 0)
        {
            instructionType = 0b01100000000000000000000000000000;
            instType = 'I';
        }
        else if (strcmp(instructionSplitted[0], "J") == 0)
        {
            instructionType = 0b01110000000000000000000000000000;
            instType = 'J';
        }
        else if (strcmp(instructionSplitted[0], "SLL") == 0)
        {
            instructionType = 0b10000000000000000000000000000000;
            instType = 'R';
        }
        else if (strcmp(instructionSplitted[0], "SRL") == 0)
        {
            instructionType = 0b10010000000000000000000000000000;
            instType = 'R';
        }
        else if (strcmp(instructionSplitted[0], "LW") == 0)
        {
            instructionType = 0b10100000000000000000000000000000;
            instType = 'I';
        }
        else if (strcmp(instructionSplitted[0], "SW") == 0)
        {
            instructionType = 0b10110000000000000000000000000000;
            instType = 'I';
        }
        else
        {
            continue;
        }
        int R1;
        int R2;
        int R3 = 0;
        int shamt = 0;
        int imm;
        switch (instType)
        {
        case 'R':
            R1 = parseRegNum(instructionSplitted[1]);
            R2 = parseRegNum(instructionSplitted[2]);
            if (strcmp(instructionSplitted[0], "ADD") || strcmp(instructionSplitted[0], "SUB"))
                R3 = parseRegNum(instructionSplitted[3]);
            else
                shamt = parseRegNum(instructionSplitted[3]);
            instructionType = instructionType | R1 << 23 | R2 << 18 | R3 << 13 | shamt;
            break;
        case 'I':
            R1 = parseRegNum(instructionSplitted[1]);
            R2 = parseRegNum(instructionSplitted[2]);
            imm = parseInt(instructionSplitted[3]);
            instructionType = instructionType | R1 << 23 | R2 << 18 | imm;

            break;
        case 'J':
            address = parseInt(instructionSplitted[1]);
            instructionType = instructionType | address;
            break;
        }
        mainMemory.mainMemory[c++]=instructionType;
    }
    fclose(input);
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
    registerFile.regWrite = (instruction & 0b00001111100000000000000000000000) >> 23;
    registerFile.readReg1 = (instruction & 0b00000000011111000000000000000000) >> 18;
    registerFile.readReg2 = (instruction & 0b00000000000000111110000000000000) >> 13;
    decodedValues.shamt = (instruction & 0b00000000000000000001111111111111);
    decodedValues.imm = (instruction & 0b00000000000000111111111111111111);
    decodedValues.address = (instruction & 0b00001111111111111111111111111111);
}
void decode2(int instruction)
{
    registerFile.readData1=(registerFile.registerArray[registerFile.readReg1]);
    
}


void controlUnitSignals(int opcode)
{
    CU.memRead = opcode == 10;
    CU.memWrite = opcode == 5;
    CU.ALUsrc = opcode == 2 || opcode == 3 || opcode== 4|| opcode == 5 || opcode == 6 || opcode == 10 || opcode == 11;
    CU.regWrite = opcode == 0 || opcode == 1 || opcode == 2 || opcode == 3 || opcode == 5 || opcode == 6 || opcode == 8 || opcode == 9 || opcode == 10;
    CU.memToReg = opcode == 10;
    CU.branch= opcode==4;
    CU.regDst= opcode==0 || opcode==1 || opcode==8 || opcode==9;



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

int checkControlHazard(int instruction)
{
    int opCode = instruction >> 28;
    if (opCode == 0b0101 || opCode == 0b0111)
    {
        return 1;
    }
    return 0;
}

int main()
{

    registerFile.registerArray = registerInit(32);
    // printf("%d", registerFile.registerArray[32].regValue);
    // // fetch(registerFile);
    // printf("%d", registerFile.registerArray[32].regValue);
    parse();
    printf("%d\n", mainMemory.mainMemory[0]);
    printf("%d\n", mainMemory.mainMemory[1]);
    printf("%d\n", mainMemory.mainMemory[2]);

    return 0;
}
