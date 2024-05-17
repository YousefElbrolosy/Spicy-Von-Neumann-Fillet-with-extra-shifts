#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"
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
} RegisterFile;

typedef struct
{
    int result;
    int zeroflag;
} ALUOutput;

typedef struct
{
    int branch;
    int memRead;
    int memWrite;
    int memtoReg;
    int ALUSrc;
    int regWrite;
    int jump;
} ControlUnit;

typedef struct
{
    int PC;
    int IR;
    bool active;
    int instNum; //to be able to know which instruction I'm in for printing purposes

} IF_ID;

typedef struct
{
    int reg1;    // number of reg 1 that will be the destination
    int reg2Addr;
    int reg2;    // value of reg 2
    int reg3Addr;
    int reg3;    // value of reg3
    int opcode;  // needed for execution
    int address; // needed for J-format
    int imm;     // needed for I-format
    int shamt;   // shift amount in R-format
    int PC;      // save PC value in this reg file since it'll be overwritten in the next cycle while pipelining

    int ALUSrc; // control signals (needed in execution phase)

    int branch;   // control signals (needed for memory access phase)
    int jump;     // control signals (needed for memory access phase)
    int memRead;  // control signals (needed for memory access phase)
    int memWrite; // control signals (needed for memory access phase)

    int memtoReg; // control signals (needed for write back phase)
    int regWrite; // control signals (needed for write back phase)


    bool active;
    int instNum; //to be able to know which instruction I'm in for printing purposes
} ID_EX;

typedef struct
{
    int ALUoutput;
    int prevALUOutput;  //needed for controlling a hazard

    // int zeroFlag;
    int reg1; // number of reg 1 that will be the destination
    int reg3; // value of reg3
    // int branchAddition;

    // int branch;  // control signals (needed for memory access phase)
    // int jump;    // control signals (needed for memory access phase)
    int memRead;  // control signals (needed for memory access phase)
    int memWrite; // control signals (needed for memory access phase)

    int memtoReg; // control signals (needed for write back phase)
    int regWrite; // control signals (needed for write back phase)

    bool active;
    int instNum; //to be able to know which instruction I'm in for printing purposes

} EX_MEM;

typedef struct
{
    int ALUoutput; // this or the below will be written in the regFile
    int readData;  // this or the above will be written in the regFile
    int reg1;      // number of reg 1 that will be the destination

    int memtoReg; // control signals (needed for write back phase)
    int regWrite; // control signals (needed for write back phase)

    bool active;
    int instNum; //to be able to know which instruction I'm in for printing purposes

} MEM_WB;

// Global Initialisation
MainMemory mainMemory;
RegisterFile registerFile;
Register PC = {"PC", 0};
ControlUnit CU;

IF_ID IF_ID_regFile;
ID_EX ID_EX_regFile;
EX_MEM EX_MEM_regFile;
MEM_WB MEM_WB_regFile;
int fetch_active = 1;
bool stallingNeeded = 0;


Register *registerInit(int regCount)
{
    Register *registers = malloc(regCount * sizeof(Register));
    Register ZERO_REGISTER = {"R0", 0};
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
    int address;
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
        int R3 = 0;//incase it is a shift
        int shamt = 0;
        int imm;
        switch (instType)
        {
            case 'R':
                R1 = parseRegNum(instructionSplitted[1]);
                R2 = parseRegNum(instructionSplitted[2]);
                if (strcmp(instructionSplitted[0], "ADD") == 0 || strcmp(instructionSplitted[0], "SUB") == 0)
                    R3 = parseRegNum(instructionSplitted[3]);
                else
                    shamt = parseInt(instructionSplitted[3]);
                instructionType = instructionType | R1 << 23 | R2 << 18 | R3 << 13 | shamt;
                break;
            case 'I':
                R1 = parseRegNum(instructionSplitted[1]);
                R2 = parseRegNum(instructionSplitted[2]);
                imm = parseInt(instructionSplitted[3]);
                int immNegative = 0b00000000000000111111111111111111 & imm;
                instructionType = instructionType | R1 << 23 | R2 << 18 | immNegative;
                break;
            case 'J':
                address = parseInt(instructionSplitted[1]);
                instructionType = instructionType | address;
                break;
        }
        mainMemory.mainMemory[c++] = instructionType;
    }
    fclose(input);
}

void fetch()
{
    if (fetch_active)
    {
        int instruction = mainMemory.mainMemory[PC.regValue];
        if (instruction == 0)
        {
            fetch_active = 0;
            return;
        }
        printf(BLU"Fetch Stage \n"RESET);
        printf( BLU"    Instruction %d is being fetched %b\n"RESET ,PC.regValue, instruction);
        PC.regValue++;
        if (stallingNeeded){
            PC.regValue--;
            stallingNeeded=false;
        }
        else {
            IF_ID_regFile.active = true;
            IF_ID_regFile.IR = instruction;
            IF_ID_regFile.PC = PC.regValue;
            IF_ID_regFile.instNum=PC.regValue-1;
        }

    }
}

void controlUnitSignals(int opcode)
{
    CU.memRead = opcode == 10;
    CU.memWrite = opcode == 11;
    CU.ALUSrc = opcode == 2 || opcode == 3 || opcode == 5 || opcode == 6 || opcode == 9 || opcode == 10;
    CU.regWrite = opcode == 0 || opcode == 1 || opcode == 2 || opcode == 3 || opcode == 5 || opcode == 6 || opcode == 8 || opcode == 9 || opcode == 10;
    CU.memtoReg = opcode == 10;
    CU.branch = opcode == 4;
    CU.jump = opcode == 7;
}

void decode()
{
    if (IF_ID_regFile.active)
    {
        printf(GRN "Decode Stage \n");
        IF_ID_regFile.active = 0;
        int instruction = IF_ID_regFile.IR;
        int reg2Addr = (instruction & 0b00000000011111000000000000000000) >> 18;
        int reg3Addr = (instruction & 0b00000000000000111110000000000000) >> 13;
        ID_EX_regFile.opcode = (instruction & 0b11110000000000000000000000000000) >> 28;

        // following is the hazard detection unit
        if (ID_EX_regFile.memRead == 1 && (ID_EX_regFile.reg1 == reg2Addr || ID_EX_regFile.reg1 == reg3Addr))
        {
            stallingNeeded=true;
            IF_ID_regFile.active = 1;
            ID_EX_regFile.ALUSrc = 0;
            ID_EX_regFile.branch = 0;
            ID_EX_regFile.jump =0;
            ID_EX_regFile.memRead = 0;
            ID_EX_regFile.memWrite = 0;
            ID_EX_regFile.memtoReg = 0;
            ID_EX_regFile.regWrite = 0;

        }
        else
        {
            controlUnitSignals(ID_EX_regFile.opcode);
            ID_EX_regFile.ALUSrc = CU.ALUSrc;
            ID_EX_regFile.branch = CU.branch;
            ID_EX_regFile.jump = CU.jump;
            ID_EX_regFile.memRead = CU.memRead;
            ID_EX_regFile.memWrite = CU.memWrite;
            ID_EX_regFile.memtoReg = CU.memtoReg;
            ID_EX_regFile.regWrite = CU.regWrite;
            ID_EX_regFile.active = true;
        }

        int opcode = ID_EX_regFile.opcode;
        ID_EX_regFile.reg1 = (instruction & 0b00001111100000000000000000000000) >> 23;
        ID_EX_regFile.reg2Addr = (instruction & 0b00000000011111000000000000000000) >> 18;
        ID_EX_regFile.reg3Addr = (instruction & 0b00000000000000111110000000000000) >> 13;
        ID_EX_regFile.shamt = (instruction & 0b00000000000000000001111111111111);
        int signBit = instruction & 0b00000000000000100000000000000000;
        ID_EX_regFile.imm = instruction & 0b00000000000000111111111111111111;
        if (signBit == 0b100000000000000000)
            ID_EX_regFile.imm = 0b11111111111111000000000000000000 | ID_EX_regFile.imm;
        ID_EX_regFile.address = (instruction & 0b00001111111111111111111111111111);
        ID_EX_regFile.reg2 = registerFile.registerArray[ID_EX_regFile.reg2Addr].regValue;                                                                                           // reads any operands required from the register file
        ID_EX_regFile.reg3 = (opcode == 4 || opcode == 11) ? registerFile.registerArray[ID_EX_regFile.reg1].regValue : registerFile.registerArray[ID_EX_regFile.reg3Addr].regValue; // reads any operands required from the register file
        ID_EX_regFile.PC = IF_ID_regFile.PC;
        ID_EX_regFile.instNum= IF_ID_regFile.instNum;// For pipelining; in case we needed PC value in BNE instruction in the execution phase

        printf("    Instruction %d is being decoded\n", IF_ID_regFile.instNum);

        switch(opcode){
            case 0: case 1:
                printf("    Type R\n");
                printf("    Opcode: %d\n", opcode);
                printf("    R1: %d\n", ID_EX_regFile.reg1);
                printf("    R2: %d\n", ID_EX_regFile.reg2Addr);
                printf("    R3: %d\n" RESET, ID_EX_regFile.reg3Addr);
                break;
            case 8: case 9:
                printf("    Type R\n");
                printf("    Opcode: %d\n", opcode);
                printf("    R1: %d\n", ID_EX_regFile.reg1);
                printf("    R2: %d\n", ID_EX_regFile.reg2Addr);
                printf("    SHAMT: %d\n" RESET, ID_EX_regFile.shamt);
                break;
            case 2: case 3: case 4: case 5: case 6: case 10: case 11:
                printf("    Type I\n");
                printf("    Opcode: %d\n", opcode);
                printf("    R1: %d\n", ID_EX_regFile.reg1);
                printf("    R2: %d\n", ID_EX_regFile.reg2Addr);
                printf("    IMM: %d\n" RESET, ID_EX_regFile.imm);
                break;
            case 7:
                printf("    Type J\n");
                printf("    Opcode: %d\n", opcode);
                printf("    R1: %d\n", ID_EX_regFile.reg1);
                printf("    Address: %d\n" RESET,ID_EX_regFile.address);
                break;
        }
        if (stallingNeeded) {
        printf("    Instruction %d discovered that STALLING IS NEEDED\n", IF_ID_regFile.instNum);
        }
    }
}

ALUOutput ALU(int operandA, int operandB, int operation)
{
    int output = 0;
    switch (operation)
    {
        case 0: // ADD
            output = operandA + operandB;
            break;
        case 1: // SUB
            output = operandA - operandB;
            break;
        case 2: // MUL
            output = operandA * operandB;
            break;
        case 3: // And
            output = operandA & operandB;
            break;
        case 4: // Or
            output = operandA | operandB;
            break;
        case 5: // SLL
            output = operandA << operandB;
            break;
        case 6: // SRL
            output = (unsigned int) operandA >> operandB;
            break;
    }

    ALUOutput outputStruct;
    outputStruct.result = output;
    outputStruct.zeroflag = (output == 0);
    return outputStruct;
}

void memAccess()
{
    if (EX_MEM_regFile.active)
    {
        printf(RED);
        EX_MEM_regFile.active = 0;
        if (EX_MEM_regFile.memRead == 1){
            printf("Memory Access Stage (MA)\n");
            MEM_WB_regFile.readData = mainMemory.mainMemory[EX_MEM_regFile.ALUoutput];
            printf("    Address read from is: %d \n",EX_MEM_regFile.ALUoutput);
            printf("    Data Read from memory: %d \n",MEM_WB_regFile.readData);
        }
        else if (EX_MEM_regFile.memWrite == 1) {
            bool printALU=false;
            if (MEM_WB_regFile.reg1==EX_MEM_regFile.reg1){
                mainMemory.mainMemory[EX_MEM_regFile.ALUoutput] = EX_MEM_regFile.prevALUOutput; // here I used the third register but this is because I assumed a fixed architecture where I saved the value of R1 in R3 in the reg file
                printALU=true;
            }
            else
                mainMemory.mainMemory[EX_MEM_regFile.ALUoutput] = EX_MEM_regFile.reg3; // here I used the third register but this is because I assumed a fixed architecture where I saved the value of R1 in R3 in the reg file
            printf("Memory Access Stage (MA) for instruction %d\n",EX_MEM_regFile.instNum);
            printf("    Address written into: %d \n",EX_MEM_regFile.ALUoutput);
            if (printALU)
                printf("    Data written into memory: %d \n",EX_MEM_regFile.prevALUOutput);
            else
                printf("    Data written into memory: %d \n",EX_MEM_regFile.reg3);
        }
        //    else if (EX_MEM_regFile.jump==1|| (EX_MEM_regFile.branch==1 && EX_MEM_regFile.zeroFlag!=1))
        //        PC.regValue= EX_MEM_regFile.branchAddition;

        MEM_WB_regFile.ALUoutput = EX_MEM_regFile.ALUoutput;
        MEM_WB_regFile.memtoReg = EX_MEM_regFile.memtoReg;
        MEM_WB_regFile.reg1 = EX_MEM_regFile.reg1;
        MEM_WB_regFile.regWrite = EX_MEM_regFile.regWrite;
        MEM_WB_regFile.active = 1;
        MEM_WB_regFile.instNum = EX_MEM_regFile.instNum;
        EX_MEM_regFile.reg1 = 0;
        printf(RESET);
    }
}

void writeBack()
{
    if (MEM_WB_regFile.active)
    {

        MEM_WB_regFile.active = 0;

        if (MEM_WB_regFile.regWrite == 1)
        {
            if(MEM_WB_regFile.reg1 != 0) {
                printf(CYN"Write Back Stage (WB) for instruction %d\n",MEM_WB_regFile.instNum);
                if (MEM_WB_regFile.memtoReg == 0)
                    registerFile.registerArray[MEM_WB_regFile.reg1].regValue = MEM_WB_regFile.ALUoutput;
                else
                    registerFile.registerArray[MEM_WB_regFile.reg1].regValue = MEM_WB_regFile.readData;
            }
            printf("    Register %s has value of %d \n"RESET,registerFile.registerArray[MEM_WB_regFile.reg1].regName,registerFile.registerArray[MEM_WB_regFile.reg1].regValue);
        }

    }

}

void flushPipeline()
{
    memset(&IF_ID_regFile, 0, sizeof(IF_ID_regFile));
    // memset betakhod ptr (el block el ha fill),value (el ha fill beeh el block),
    // number of bytes to be set to the value
    memset(&ID_EX_regFile, 0, sizeof(ID_EX_regFile));
    printf("    Pipeline Flushed.\n");
}
void exec()
{
    if (ID_EX_regFile.active)
    {
        int forwardA = 0;
        int forwardB = 0;

        // handling control signals of forwarding
        if (MEM_WB_regFile.regWrite == 1 && MEM_WB_regFile.reg1 != 0 &&
            !(EX_MEM_regFile.regWrite == 1 && EX_MEM_regFile.reg1 != 0 && EX_MEM_regFile.reg1 != ID_EX_regFile.reg2Addr) && (MEM_WB_regFile.reg1 == ID_EX_regFile.reg2Addr))
            forwardA = 1;

        if (MEM_WB_regFile.regWrite == 1 && MEM_WB_regFile.reg1 != 0 &&
            !(EX_MEM_regFile.regWrite == 1 && EX_MEM_regFile.reg1 != 0 && EX_MEM_regFile.reg1 != ID_EX_regFile.reg3Addr) && (MEM_WB_regFile.reg1 == ID_EX_regFile.reg3Addr))
            forwardB = 1;

        if (EX_MEM_regFile.regWrite == 1 && EX_MEM_regFile.reg1 != 0 && EX_MEM_regFile.reg1 == ID_EX_regFile.reg2Addr)
            forwardA = 2;

        if (EX_MEM_regFile.regWrite == 1 && EX_MEM_regFile.reg1 != 0 && EX_MEM_regFile.reg1 == ID_EX_regFile.reg3Addr)
            forwardB = 2;

        ID_EX_regFile.active = 0;
        int opCode = ID_EX_regFile.opcode;
        int operandA;
        if (forwardA == 0)
            operandA = ID_EX_regFile.reg2;
        else if (forwardA == 1)
        {
            if (MEM_WB_regFile.memtoReg == 0)
                operandA = MEM_WB_regFile.ALUoutput;
            else
                operandA = MEM_WB_regFile.readData;
        }
        else if (forwardA == 2)
            operandA = EX_MEM_regFile.ALUoutput;
        int operandB;
        int zeroFlag;
        printf(MAG"Execute Stage \n");
        printf("    Instruction %d is being executed\n", ID_EX_regFile.instNum);
        // when checking with the group, the rhs should come from ID_EX_regFile and the lhs from EX_MEM_regFile
        EX_MEM_regFile.prevALUOutput= EX_MEM_regFile.ALUoutput;
        switch (opCode)
        {
            case 0: // ADD
                printf("    Addition is being executed\n");
                if (forwardB == 0)
                    operandB = ID_EX_regFile.reg3;
                else if (forwardB == 1)
                {
                    if (MEM_WB_regFile.memtoReg == 0)
                        operandB = MEM_WB_regFile.ALUoutput;
                    else
                        operandB = MEM_WB_regFile.readData;
                }
                else if (forwardB == 2)
                    operandB = EX_MEM_regFile.ALUoutput;
                EX_MEM_regFile.ALUoutput = ALU(operandA, operandB, 0).result;
                printf("        Operand 1:%d\n", operandA);
                printf("        Operand 2:%d\n", operandB);
                printf("        ALU Result:%d\n",EX_MEM_regFile.ALUoutput);
                break;
            case 1: // SUB
            printf("    Subtraction is being executed\n");
                if (forwardB == 0)
                    operandB = ID_EX_regFile.reg3;
                else if (forwardB == 1)
                {
                    if (MEM_WB_regFile.memtoReg == 0)
                        operandB = MEM_WB_regFile.ALUoutput;
                    else
                        operandB = MEM_WB_regFile.readData;
                }
                else if (forwardB == 2)
                    operandB = EX_MEM_regFile.ALUoutput;
                EX_MEM_regFile.ALUoutput = ALU(operandA, operandB, 1).result;

                printf("        Operand 1: %d\n", operandA);
                printf("        Operand 2: %d\n", operandB);
                printf("        ALU Result: %d\n",EX_MEM_regFile.ALUoutput);

                break;
            case 2: // MULI
                printf("    Multiply Immediate is being executed\n");
                operandB = ID_EX_regFile.imm;
                EX_MEM_regFile.ALUoutput = ALU(operandA, operandB, 2).result;
                printf("        Operand 1: %d\n", operandA);
                printf("        Operand 2: %d\n", operandB);
                printf("        ALU Result: %d\n",EX_MEM_regFile.ALUoutput);

                break;
            case 3: // ADDI
                printf("    ADD Immediate is being executed\n");
                operandB = ID_EX_regFile.imm;
                EX_MEM_regFile.ALUoutput = ALU(operandA, operandB, 0).result;
                printf("        Operand 1: %d\n", operandA);
                printf("        Operand 2: %d\n", operandB);
                printf("        ALU Result: %d\n",EX_MEM_regFile.ALUoutput);

                break;
            case 4: // BNE
                if (forwardB == 0)
                    operandB = ID_EX_regFile.reg3; // value of reg1 is inside this reg
                else if (forwardB == 1)
                {
                    if (MEM_WB_regFile.memtoReg == 0)
                        operandB = MEM_WB_regFile.ALUoutput;
                    else
                        operandB = MEM_WB_regFile.readData;
                }
                else if (forwardB == 2)
                    operandB = EX_MEM_regFile.ALUoutput;
                ALUOutput out = ALU(operandA, operandB, 1);
                if (ID_EX_regFile.branch == 1 && out.zeroflag != 1)
                {
                    printf("    Branch on NOT EQUAL is being executed\n");
                    PC.regValue = ID_EX_regFile.PC + ID_EX_regFile.imm;
                    printf("        Branching to Address: %d\n",PC.regValue);
                    flushPipeline();
                }
                else
                    printf("    Branch on NOT EQUAL condition failed\n");
                break;
            case 5: // ANDI
                printf("    AND Immediate is being executed\n");
                operandB = ID_EX_regFile.imm;
                EX_MEM_regFile.ALUoutput = ALU(operandA, operandB, 3).result;
                printf("        Operand 1: %d\n", operandA);
                printf("        Operand 2: %d\n", operandB);
                printf("        ALU Result: %d\n",EX_MEM_regFile.ALUoutput);
                break;
            case 6: // ORI
                printf("    OR Immeediate is being executed\n");
                operandB = ID_EX_regFile.imm;
                EX_MEM_regFile.ALUoutput = ALU(operandA, operandB, 4).result;
                printf("        Operand 1: %d\n", operandA);
                printf("        Operand 2: %d\n", operandB);
                printf("        ALU Result: %d\n",EX_MEM_regFile.ALUoutput);
                break;
            case 7: // J
                if (ID_EX_regFile.jump == 1)
                {
                    printf("    JUMP is being executed\n");
                    PC.regValue = (ID_EX_regFile.PC & 0b11110000000000000000000000000000) | ID_EX_regFile.address;
                    printf("        Jumping to Address: %d\n",PC.regValue);
                    flushPipeline();
                }
                break;
            case 8: // SLL
                printf("    Shift Left Logical is being executed\n");
                operandB = ID_EX_regFile.shamt;
                EX_MEM_regFile.ALUoutput = ALU(operandA, operandB, 5).result;
                printf("        Operand 1: %d\n", operandA);
                printf("        Shift Amount: %d\n", operandB);
                printf("        ALU Result: %d\n",EX_MEM_regFile.ALUoutput);
                break;
            case 9: // SRL
                printf("    Shift Right Logical is being executed\n");
                operandB = ID_EX_regFile.shamt;
                EX_MEM_regFile.ALUoutput = ALU(operandA, operandB, 6).result;
                printf("        Operand 1: %d\n", operandA);
                printf("        Shift Amount: %d\n", operandB);
                printf("        ALU Result: %d\n",EX_MEM_regFile.ALUoutput);
                break;
            case 10: // LW
                printf("    Load Word is being executed\n");
                operandB = ID_EX_regFile.imm;
                EX_MEM_regFile.ALUoutput = ALU(operandA, operandB, 0).result;
                printf("        Base Address: %d\n", operandA);
                printf("        Offset Value: %d\n", operandB);
                printf("        Result Address: %d\n",EX_MEM_regFile.ALUoutput);
                break;
            case 11: // SW
                printf("    Store Word is being executed\n");
                operandB = ID_EX_regFile.imm;
                EX_MEM_regFile.ALUoutput = ALU(operandA, operandB, 0).result;
                printf("        Base Address: %d\n", operandA);
                printf("        Offset Value: %d\n", operandB);
                printf("        Result Address: %d\n",EX_MEM_regFile.ALUoutput);
                break;
        }
        printf(RESET);
        EX_MEM_regFile.reg1 = ID_EX_regFile.reg1;
        EX_MEM_regFile.reg3 = ID_EX_regFile.reg3;
        EX_MEM_regFile.memRead = ID_EX_regFile.memRead;
        EX_MEM_regFile.memWrite = ID_EX_regFile.memWrite;
        EX_MEM_regFile.memtoReg = ID_EX_regFile.memtoReg;
        EX_MEM_regFile.regWrite = ID_EX_regFile.regWrite; // check that these signals are initialized somewhere
        EX_MEM_regFile.active = 1;
        EX_MEM_regFile.instNum = ID_EX_regFile.instNum;
        // ID_EX_regFile.memRead = 0;
//        printf("memRead Signal: %d \n", EX_MEM_regFile.memRead);
//        printf("memWrite Signal: %d \n", EX_MEM_regFile.memWrite);
//        printf("memtoReg Signal: %d \n", EX_MEM_regFile.memtoReg);
//        printf("regWrite Signal: %d \n", EX_MEM_regFile.regWrite);
    }
}

void printDecode() {
    if (IF_ID_regFile.active) {
        printf(GRN"Decode Stage \n"RESET);
        printf(GRN"    Instruction %d started decoding \n"RESET,IF_ID_regFile.instNum);
    }
}

void printExecute() {
    if (ID_EX_regFile.active) {
        printf(MAG"Execute Stage\n"RESET);
        printf(MAG"    Instruction %d started executing \n"RESET,ID_EX_regFile.instNum);
    }
}

int main()
{
    printf("Start of Program\n");
    registerFile.registerArray = registerInit(32);
    parse();
    int i = 0;
    while (fetch_active || IF_ID_regFile.active || ID_EX_regFile.active || EX_MEM_regFile.active || MEM_WB_regFile.active)
    {
        printf(WHT"---------------------------\n");
        printf("Cycle %d:\n"RESET, ++i);
        if (i % 2 == 0)
        {
            memAccess(); //it won't work except when active and this triggered by the exec
            printExecute();
            printDecode();
        }
        else
        {
            writeBack(); //it won't work except when active and this triggered by the memAccess
            exec(); //it won't work except when active and this triggered by the decode
            decode(); //it won't work except when active and this triggered by the fetch
            fetch(); //it will stop working after reading the first 0 instruction
        }
    }

    for (int j = 0; j < 32; j++)
    {
        printf("Register Name: %s\n", registerFile.registerArray[j].regName);
        printf("Register Value: %d\n", registerFile.registerArray[j].regValue);
    }

    for(int j =0; j < 2048; j++)
    {
        if(i<1024)
            printf("Instruction Address: %d\n", j);
        else
            printf("Data Address: %d\n", j);
        printf("Memory Value: %d\n", mainMemory.mainMemory[j]);
    }
    printf("End of Program\n");

    return 0;
}
