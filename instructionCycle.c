#include <stdio.h>

int registerFile [] = {22,16,87,54,102,434,33,67,98,49,55,9,18,1,32,45};
int instructionMemory[] = {-1204737950,780626538};
int NumberofInstructions = sizeof(instructionMemory)/sizeof(int);
int pc = 0;
int zeroFlag = 0;

// Binary int format in c is 0b00000000000000000000000000000000 (32 bits)

void decode(int instruction) {

             
}


void fetch() {

        
}

int ALU(int operandA, int operandB, int operation){
    int output = 0;
    zeroFlag = 0;

    // Complete the ALU body here...
    switch(operation){
        case 0:
            output = operandA >> operandB;
            break;
        case 1:
            output = operandA << operandB;            
            break;
            
        case 2:
            output = operandA-operandB;
            break;
        case 3:
            output = operandA*operandB;           
            break;
        case 4:
            output = operandA/operandB;              
            break;
        case 5:
            if (operandA>=operandB){
                 output = 1;
            }
            break;
        case 6:
            output = ~(operandA&operandB);
            break;
        case 7:
            output = operandA+operandB ;           
            break;
        case 8:
             output = operandA^operandB   ;          
            break;
    }
    
    
    if (output == 0){
        zeroFlag = 1;
    }
    printf("Operation = %d\n", operation);
    printf("First Operand = %d\n", operandA);
    printf("Second Operand = %d\n", operandB);
    printf("Result = %d\n", output);
    printf("Zero Flag = %d\n", zeroFlag);


    return output;
}



int main() {

    return 0;

		
}





