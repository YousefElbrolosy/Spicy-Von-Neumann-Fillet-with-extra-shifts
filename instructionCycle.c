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
    int jump;
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

typedef struct
{
    int result;
    int zeroflag;
}ALUOutput;

// Global Initialisation
MainMemory mainMemory;
RegisterFile registerFile;
Register PC = {"PC", 0};
DecodedValues decodedValues;
ControlUnit CU;
ALUOutput ALUoutput;
int MDR;
int zeroFlag = 0;
int* dest;
int writeBackVal;

// will use Flag to know whether an instruction is valid fe kol pipeline stage
int instructionValid[3] = {1, 1, 1}; // 1st index->Fetch, 2nd->Decode,3rd-> Execute


void flushPipeline() {
    //queue inside it in that order {instruction7,instruction6, instruction5, instruction4,instruction3, instruction2, instruction1}
    instructionValid[0] = 0; // Flush fetch 
    instructionValid[1] = 0; // Flush decode 
}

int checkControlHazard(int instruction)
{
    int opCode = instruction >> 28;
    if (opCode == 0b0101 || opCode == 0b0111)
    {
        PC.regValue = decodedValues.address; // Update PC bl calculated branch address
        flushPipeline(); // Flush instructions in decode and fetch stages
        return 1;
    }
    return 0;
}

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
    if (!instructionValid[0]){ 
        return -1;
    }
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
    CU.jump= opcode == 7;

}

void ALU(int operandA, int operandB, int operation)
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
            output = operandA >> operandB;
            break;
    }
    ALUoutput.result = output;
    ALUoutput.zeroflag = (output == 0);
}

int memAccess(){
   if(CU.memRead==1){
        MDR=mainMemory.mainMemory[writeBackVal];
    }
}

int writeBack(int address, int regNum){
    if(dest){
        if (CU.memWrite==1){
            mainMemory.mainMemory[*dest]= writeBackVal;
        }
        else if(CU.memtoReg==1){
            *dest = MDR;
        }
        else if (CU.regWrite==1 || CU.branch==1 || CU.jump == 1){ 
            *dest=writeBackVal;
        }
    }
}


void exec()
{
    if (!instructionValid[2]) 
        return; 
    if (decodedValues.opcode == 0b0101 || decodedValues.opcode == 0b0111){ 
        flushPipeline(); 
        PC.regValue = decodedValues.address; // Update el PC w flush pipeline
        instructionValid[0]=1;
        instructionValid[1]=1;
        instructionValid[2]=1; //aizeen neraga3 el validity of f/d/e ba3d el branching
    }
    int opCode = decodedValues.opcode;
    int operandA;
    int operandB;
    int zeroFlag;
    switch (opCode)
    {
        case 0: // ADD
            operandA = registerFile.registerArray[decodedValues.r2].regValue;
            operandB = registerFile.registerArray[decodedValues.r3].regValue;
            dest = &registerFile.registerArray[decodedValues.r1].regValue;
            ALU(operandA,operandB,0);
            writeBackVal=ALUoutput.result;
            break;
        case 1: // SUB
            operandA = registerFile.registerArray[decodedValues.r2].regValue;
            operandB = registerFile.registerArray[decodedValues.r3].regValue;
            dest = &registerFile.registerArray[decodedValues.r1].regValue;
            ALU(operandA,operandB,1);
            writeBackVal=ALUoutput.result;
            break;
        case 2: // MULI
            operandA = registerFile.registerArray[decodedValues.r2].regValue;
            operandB = decodedValues.imm;
            dest = &registerFile.registerArray[decodedValues.r1].regValue;
            ALU(operandA,operandB,2);
            writeBackVal=ALUoutput.result;
            break;
        case 3: // ADDI
            operandA = registerFile.registerArray[decodedValues.r2].regValue;
            operandB = decodedValues.imm;
            dest = &registerFile.registerArray[decodedValues.r1].regValue;
            ALU(operandA,operandB,0);
            writeBackVal=ALUoutput.result;
            break;
        case 4: // BNE
            operandA = registerFile.registerArray[decodedValues.r1].regValue;
            operandB = registerFile.registerArray[decodedValues.r2].regValue;
            ALU(operandA,operandB,1);
            if(!ALUoutput.zeroflag){
                CU.branch=1;
                dest =&PC.regValue;
                writeBackVal=decodedValues.imm;
            }
            break;
        case 5: // ANDI
            operandA = registerFile.registerArray[decodedValues.r2].regValue;
            operandB = decodedValues.imm;
            dest = &registerFile.registerArray[decodedValues.r1].regValue;
            ALU(operandA,operandB,3);
            writeBackVal=ALUoutput.result;
            break;
        case 6: // ORI
            operandA = registerFile.registerArray[decodedValues.r2].regValue;
            operandB = decodedValues.imm;
            dest = &registerFile.registerArray[decodedValues.r1].regValue;
            ALU(operandA,operandB,4);
            writeBackVal=ALUoutput.result;
            break;
        case 7: //J
            dest = &PC.regValue;
            writeBackVal= (PC.regValue & 0b11110000000000000000000000000000 )|| decodedValues.address;
            break;
        case 8: //SLL
            operandA = registerFile.registerArray[decodedValues.r2].regValue;
            operandB = decodedValues.shamt;
            dest = &registerFile.registerArray[decodedValues.r1].regValue;
            ALU(operandA,operandB,5);
            writeBackVal=ALUoutput.result;
            break;
        case 9: //SRL
            operandA = registerFile.registerArray[decodedValues.r2].regValue;
            operandB = decodedValues.shamt;
            dest = &registerFile.registerArray[decodedValues.r1].regValue;
            ALU(operandA,operandB,6);
            writeBackVal=ALUoutput.result;
            break;
        case 10://LW  ()
            operandA = registerFile.registerArray[decodedValues.r2].regValue;
            operandB = decodedValues.imm;
            dest = &registerFile.registerArray[decodedValues.r1].regValue;
            ALU(operandA,operandB,0);
            writeBackVal=&ALUoutput.result;
            break;
        case 11: //SW
            operandA = registerFile.registerArray[decodedValues.r2].regValue;
            operandB = decodedValues.imm;
            ALU(operandA,operandB,0);
            dest = &ALUoutput.result; //get index
            writeBackVal=registerFile.registerArray[decodedValues.r1].regValue;
            break;
    }

}

int main()
{
    int instructionTest;


    //pipeline

    // for(int i=0;i<512;i++){
    //     if(i%2==0){
    //         instructionTest = fetch(mainMemory.mainMemory[i]);
    //     }
    // }

    registerFile.registerArray = registerInit(32);
    printf("%d", registerFile.registerArray[32].regValue);
    fetch(registerFile);
    printf("%d", registerFile.registerArray[32].regValue);
    return 0;
}

