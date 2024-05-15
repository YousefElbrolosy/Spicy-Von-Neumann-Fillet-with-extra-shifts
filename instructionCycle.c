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
} RegisterFile;

typedef struct
{
    int result;
    int zeroflag;
}ALUOutput;

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
}IF_ID;

typedef struct
{
    int reg1;    // number of reg 1 that will be the destination
    int reg2;    // value of reg 2
    int reg3;    // value of reg3
    int opcode;  // needed for execution
    int address; // needed for J-format
    int imm;     // needed for I-format
    int shamt;   // shift amount in R-format
    int PC;      // save PC value in this reg file since it'll be overwritten in the next cycle while pipelining
   
    int ALUSrc;  // control signals (needed in execution phase)
   
    int branch;  // control signals (needed for memory access phase)
    int jump;    // control signals (needed for memory access phase)
    int memRead; // control signals (needed for memory access phase)
    int memWrite;// control signals (needed for memory access phase)
    
    int memtoReg;// control signals (needed for write back phase)
    int regWrite;// control signals (needed for write back phase)

}ID_EX;

typedef struct
{
    int ALUoutput;  
    // int zeroFlag; 
    int reg1;    // number of reg 1 that will be the destination
    int reg3;    // value of reg3
    // int branchAddition;
    
    // int branch;  // control signals (needed for memory access phase)
    // int jump;    // control signals (needed for memory access phase)
    int memRead; // control signals (needed for memory access phase)
    int memWrite;// control signals (needed for memory access phase)
    
    int memtoReg;// control signals (needed for write back phase)
    int regWrite;// control signals (needed for write back phase)
}EX_MEM;

typedef struct 
{
    int ALUoutput; //this or the below will be written in the regFile
    int readData; //this or the above will be written in the regFile
    int reg1;  // number of reg 1 that will be the destination

    int memtoReg;// control signals (needed for write back phase)
    int regWrite;// control signals (needed for write back phase)
}MEM_WB;



// Global Initialisation
MainMemory mainMemory;
RegisterFile registerFile;
Register PC = {"PC", 0};
ControlUnit CU;

IF_ID IF_ID_regFile;
ID_EX ID_EX_regFile;
EX_MEM EX_MEM_regFile;
MEM_WB MEM_WB_regFile;


void flushPipeline() {  
    memset(&IF_ID_regFile,0,sizeof(IF_ID_regFile));
    //memset betakhod ptr (el block el ha fill),value (el ha fill beeh el block),
    //number of bytes to be set to the value
    memset(&ID_EX_regFile,0,sizeof(ID_EX_regFile));
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

void fetch()
{
    int instruction = mainMemory.mainMemory[PC.regValue];
    PC.regValue++;
    IF_ID_regFile.IR=instruction;
    IF_ID_regFile.PC=PC.regValue;
}

void decode()
{
    int instruction = IF_ID_regFile.IR;
    ID_EX_regFile.opcode = (instruction & 0b11110000000000000000000000000000) >> 28;
    ID_EX_regFile.reg1 = (instruction & 0b00001111100000000000000000000000) >> 23;
    int reg2Num = (instruction & 0b00000000011111000000000000000000) >> 18;
    int reg3Num = (instruction & 0b00000000000000111110000000000000) >> 13;
    ID_EX_regFile.shamt = (instruction & 0b00000000000000000001111111111111);
    ID_EX_regFile.imm = (instruction & 0b00000000000000111111111111111111);
    ID_EX_regFile.address = (instruction & 0b00001111111111111111111111111111);

    int opcode=ID_EX_regFile.opcode;

    ID_EX_regFile.reg2=registerFile.registerArray[reg2Num].regValue; // reads any operands required from the register file
    ID_EX_regFile.reg3= (opcode==4||opcode==11)? registerFile.registerArray[ID_EX_regFile.reg1].regValue :registerFile.registerArray[reg3Num].regValue; // reads any operands required from the register file
    ID_EX_regFile.PC=IF_ID_regFile.PC; //For pipelining; in case we needed PC value in BNE instruction in the execution phase

    controlUnitSignals(ID_EX_regFile.opcode);
    ID_EX_regFile.ALUSrc=CU.ALUSrc;
    ID_EX_regFile.branch=CU.branch;
    ID_EX_regFile.jump=CU.jump;
    ID_EX_regFile.memRead=CU.memRead;
    ID_EX_regFile.memWrite=CU.memWrite;
    ID_EX_regFile.memtoReg=CU.memtoReg;
    ID_EX_regFile.regWrite=CU.regWrite;

}

void controlUnitSignals(int opcode)
{
    CU.memRead = opcode == 10;
    CU.memWrite = opcode == 11;
    CU.ALUSrc = opcode == 2 || opcode == 3 || opcode == 5 || opcode == 6 || opcode == 9 || opcode == 10;
    CU.regWrite = opcode == 0 || opcode == 1 || opcode == 2 || opcode == 3 || opcode == 5 || opcode == 6 || opcode == 8 || opcode == 9 || opcode == 10;
    CU.memtoReg = opcode == 10;
    CU.branch = opcode == 4;
    CU.jump= opcode == 7;

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
            output = operandA >> operandB;
            break;
    }

    ALUOutput outputStruct;
    outputStruct.result = output;
    outputStruct.zeroflag = (output == 0);
    return outputStruct;
  
}

int memAccess()
{
    if(EX_MEM_regFile.memRead==1) 
        MEM_WB_regFile.readData=mainMemory.mainMemory[EX_MEM_regFile.ALUoutput];
    else if (EX_MEM_regFile.memWrite==1)
        mainMemory.mainMemory[EX_MEM_regFile.ALUoutput]=EX_MEM_regFile.reg3; //here I used the third register but this is because I assumed a fixed architecture where I saved the value of R1 in R3 in the reg file
    MEM_WB_regFile.ALUoutput=EX_MEM_regFile.ALUoutput;
    MEM_WB_regFile.memtoReg= EX_MEM_regFile.memtoReg;
    MEM_WB_regFile.reg1=EX_MEM_regFile.reg1;
    MEM_WB_regFile.regWrite=EX_MEM_regFile.regWrite;
}

int writeBack(){
        if (MEM_WB_regFile.regWrite==1){
            if (MEM_WB_regFile.memtoReg==0)
                registerFile.registerArray[MEM_WB_regFile.reg1].regValue=MEM_WB_regFile.ALUoutput;
            else
                registerFile.registerArray[MEM_WB_regFile.reg1].regValue=MEM_WB_regFile.readData;
        }
    
}


void exec()
{
    int opCode = ID_EX_regFile.opcode;
    int operandA=ID_EX_regFile.reg2;
    int operandB;
    int zeroFlag;
    //when checking with the group, the rhs should come from ID_EX_regFile and the lhs from EX_MEM_regFile
    switch (opCode)
    {
        case 0: // ADD
            operandB = ID_EX_regFile.reg3;
            EX_MEM_regFile.ALUoutput=ALU(operandA,operandB,0).result;
            break;
        case 1: // SUB
            operandB = ID_EX_regFile.reg3;
            EX_MEM_regFile.ALUoutput=ALU(operandA,operandB,1).result;
            break;
        case 2: // MULI
            operandB = ID_EX_regFile.reg3;
            EX_MEM_regFile.ALUoutput=ALU(operandA,operandB,2).result;
            break;
        case 3: // ADDI
            operandB = ID_EX_regFile.imm;
            EX_MEM_regFile.ALUoutput=ALU(operandA,operandB,0).result;
            break;
        case 4: // BNE
            operandB = ID_EX_regFile.reg3; // value of reg1 is inside this reg
            ALUOutput out= ALU(operandA,operandB,1);
            if (ID_EX_regFile.branch==1 && out.zeroflag != 1)
                PC.regValue= ID_EX_regFile.PC + out.result;
            break;
        case 5: // ANDI
            operandB = ID_EX_regFile.imm;
            EX_MEM_regFile.ALUoutput=ALU(operandA,operandB,3).result;
            break;
        case 6: // ORI
            operandB = ID_EX_regFile.imm;
            EX_MEM_regFile.ALUoutput=ALU(operandA,operandB,4).result;
            break;
        case 7: //J
            if (ID_EX_regFile.jump==1)
                PC.regValue=(ID_EX_regFile.PC & 0b11110000000000000000000000000000 ) | ID_EX_regFile.address;
            break;
        case 8: //SLL
            operandB = ID_EX_regFile.shamt;
            EX_MEM_regFile.ALUoutput=ALU(operandA,operandB,5).result;
            break;
        case 9: //SRL
            operandB = ID_EX_regFile.shamt;
            EX_MEM_regFile.ALUoutput=ALU(operandA,operandB,6).result;
            break;
        case 10://LW  
            operandB = ID_EX_regFile.imm;
            EX_MEM_regFile.ALUoutput=ALU(operandA,operandB,0).result;
            break;
        case 11: //SW
            operandB = ID_EX_regFile.imm;
            EX_MEM_regFile.ALUoutput=ALU(operandA,operandB,0).result;
            break;
    }
    EX_MEM_regFile.reg1 = ID_EX_regFile.reg1;
    EX_MEM_regFile.memRead = ID_EX_regFile.memRead;
    EX_MEM_regFile.memWrite = ID_EX_regFile.memWrite;
    EX_MEM_regFile.memtoReg = ID_EX_regFile.memtoReg;
    EX_MEM_regFile.regWrite = ID_EX_regFile.regWrite; // check that these signals are initialized somewhere

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

