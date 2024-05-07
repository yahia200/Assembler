#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include "resources/func.h"

int8_t* op1;
int8_t* op2;
int8_t operand1;
int8_t operand2;
int8_t* M;
uint16_t instMem[1024];
int8_t datatMem[2048]={-128};
int8_t regs[64];
bool status[8];// 0=Zero, 1=Sign, 2=Negative, 3=Two’s Complement Overflow, 4=Carry
uint16_t* PC=instMem;
uint8_t decodedBuffer[2][3] = {{-1},{-1}};
uint16_t fetchedBuffer[2] = {-1, -1};
uint16_t* xPC;// PC of next instruction to be executed
uint16_t* dPC;// PC of next instruction to be decoded
int8_t imm;
int cycle = 1;
pthread_t fetcher, decoder, executer;
bool immIns;
int DEBUG = -1;
bool halt = false;
long maxiter = 2000;// max number of Cycles befor the program automatically halts to avoid infinite loops
int c = 1;// Number of parsed instructions





// Parse code to Instructions and add it to Instruction Memory and select if in DEBUG mode
void init(){
    xPC = PC;
    dPC = PC;
    char splitLine[3][10];
    char const* const fileName = "code.txt";
    FILE* file = fopen(fileName, "r");
    char line[256];
    initIns(instMem);


    while (fgets(line, sizeof(line), file)) {// While the Assembly code has lines
        if(line[0] > 32){
        split(line, splitLine);// Split Code Line into [OPCode, Operand 1, Operand 2] and put it in the splitLine Array
        addToInsMem(splitLine, c-1);// Add parsed Instruction to Memory and increment c
        c++;
        }
        

        
    }

    fclose(file);

    printf("Please Select Mode.\n0: run code.\n1: debug each cycle.\n");
    while(DEBUG < 0 || DEBUG > 1){
        printf("Mode: ");
        DEBUG = getch() - '0';
        printf("\n");
    }
}

// Turn Instruction name into OPCode
int parseOP(char OP[10]){
    char OPs[12][10] = {"ADD","SUB", "MUL", "MOVI", "BEQZ", "ANDI", "EOR", "BR", "SAL", "SAR", "LDR", "STR"};

    for (int i = 0; i < 12; i++){
        if(strcicmp(OP, OPs[i]) == 0){// Compare input to possible instructions ignoring case
            if((i>=3 && i<=5) || i == 8 || i == 9)
                immIns = 1;
            else
                immIns = 0;
            return (int)i;//return index of instruction which is equivalent to its OPCode
        }    
    }
     printf("Unrecognized Instruction: %s ON LINE: %d\n", OP, c);//Check wrong Spelling
     exit(-1);

}


// Turn Operands from text into int
int parseOperand(char OP[10]){
    for(int i=0; i<10;i++){// Check if entered value wasn't int
        if (i == 0 && OP[i] == '-'){
            if(!immIns){
                printf("Operand Cant Be Negative For This Instruction: %s ON LINE: %d\n", OP, c);
                exit(-1);
            }
            else
                continue;

        }
        else if (!isInt(OP[i]) && OP[i] > 32){
            printf("Unrecognized Operand: %s ON LINE: %d\n", OP, c);
            exit(-1);
        }
    }
    return atoi(OP);
}

// Populate the Instruction Memory
void addToInsMem(char splitLine[3][10], int c){
        int8_t OP = parseOP(splitLine[0]);
        uint8_t operand1 = parseOperand(splitLine[1]);
        if (operand1 > 63 || operand1 < 0){// Check REG index
            printf("Register Range From (0-63) Can't Be: %d ON LINE: %d\n", (int8_t)operand1, c);
            exit(-1);
        }
        operand1 &= 0x003f;
        int8_t operand2 = parseOperand(splitLine[2]);
        operand2 &= 0x003f;
        if((uint8_t)operand2 & 0x003f > 127){// Check operand size
            printf("Operands Consist Of Only 6 BITS Can't Be: %d\n", operand1);
            exit(-1);
        }
        uint16_t ins = (OP<<12) | (operand1<<6) | (operand2);
        instMem[c] = ins;

}

 
void endCycle(){
    pthread_join(executer, NULL);
    pthread_join(decoder, NULL);
    pthread_join(fetcher, NULL);
    cycle++;
    shiftBuffers();
    println(150);
    if(*xPC == (uint16_t)-1)
        halt = true;
}

// Shift buffer data so the newly fetched/decoded instruction can be decoded/executed
void shiftBuffers(){
    fetchedBuffer[0] = fetchedBuffer[1];
    fetchedBuffer[1] = -1;
    decodedBuffer[0][0] = decodedBuffer[1][0];decodedBuffer[0][1] = decodedBuffer[1][1];decodedBuffer[0][2] = decodedBuffer[1][2];
    decodedBuffer[1][0] = -1;decodedBuffer[1][1] = -1;decodedBuffer[1][2] = -1;
}

// Puts the instruction in fetched buffer and increments PC
void fetch(){
    dPC = PC;
    if(*PC != (uint16_t)-1){
    fetchedBuffer[1] = *PC;
    printf("Fetched:  %d | Instruction: ", (PC - instMem) + 1);
    intob(fetchedBuffer[1]);
    PC++;
    }
    else{
        printf("No Instruction TO Fetch\n");
    }
}

// Splits instruction into [OPCode, Operand 1, Operand 2] by shifting and masking the instruction then puts in decoded Buffer
void decode(){
    ptod();
    printf("Decoded: %d | ", dPC - instMem);
    decodedBuffer[1][0] = fetchedBuffer[0] >> 12;
    decodedBuffer[1][1] = (fetchedBuffer[0] & 0xfc0) >> 6;
    decodedBuffer[1][2] = (fetchedBuffer[0] & 0x03f);
    twosComp((decodedBuffer[1]+2));
    
    printf("OPCode: %d | First Operand: %d | Second Operand: %d\n", decodedBuffer[1][0], decodedBuffer[1][1], (int8_t)decodedBuffer[1][2]);


}

// fetches decoded instruction from decoded buffer then using a Switch to select which operation to execute then executing it
void execute(){
    xPC++;
    printf("Executed: %d | ", xPC - instMem);
    int opCode = decodedBuffer[0][0];
    switch (opCode)
    {
    case 0://ADD
        status[4] = (((uint8_t)operand1 + (uint8_t)operand2)&(1<<8)) != 0;
        if(!((operand1<0) ^ (operand2<0))){
            status[3] = ((operand1<0) ^ ((int8_t)(operand2 + operand1) < 0));
        }
        else status[3] = false;
        printf("ADD | REG[%d](%d) += REG[%d](%d) | ", (int)op1 - (int)regs, *op1, op2 - (int)regs, *op2);
        printf("REG[%d] = ", (int)op1 - (int)regs);
        operand1 += operand2;
        *op1 = operand1;
        printf("%d\n", *op1);
        status[0] = (operand1==0);
        status[2] = (operand1 < 0);
        status[1] = status[2] ^ status[3];

        break;

    case 1://SUB
        if(((operand1<0) ^ (operand2<0))){
            status[3] = ((operand1<0) ^ ((int8_t)(operand1 - operand2) < 0));
        }
        else status[3] = false;
        printf("SUB | REG[%d](%d) -= REG[%d](%d) | ", (int)op1 - (int)regs, *op1, op2 - (int)regs, *op2);
        printf("REG[%d] = ", (int)op1 - (int)regs);
        operand1 -= operand2;
        *op1 = operand1;
        printf("%d\n", *op1);
        status[0]= (operand1==0);
        status[2] = (operand1 < 0);
        status[1] = status[2] ^ status[3];
        break;

    case 2://MUL
        printf("MUL | REG[%d](%d) *= REG[%d](%d) | ", (int)op1 - (int)regs, *op1, op2 - (int)regs, *op2);
        printf("REG[%d] = ", (int)op1 - (int)regs);
        operand1 *= operand2;
        *op1 = operand1;
        printf("%d\n", *op1);
        status[0]= (operand1==0);
        status[2] = (operand1 < 0);
        break;

    case 3://MOVI
        printf("MOVI | REG[%d](%d) = IMM(%d) | ", (int)op1 - (int)regs, *op1, imm);
        printf("REG[%d] = ", (int)op1 - (int)regs);
        operand1 = imm;
        *op1 = operand1;
        printf("%d\n", *op1);
        break;

    case 4://BEQZ
        if(operand1 == 0){
            dPC = PC;
            printf("%d\n", xPC);
            PC = xPC + imm;
            xPC+= imm;
            printf("BEQZ | BRANCHING TO: %d\n", (PC - instMem) + 1);
            clearBuffers();
            break;
        }
        printf("BEQZ | NO BRANCHING | REG = %d", *op1);
        break;

    case 5://ANDI
    printf("ANDI | REG[%d](%d) &= IMM(%d) | ", (int)op1 - (int)regs, *op1, imm);
    printf("REG[%d] = ", (int)op1 - (int)regs);
    operand1 &= imm;
    *op1 = operand1;
    printf("%d\n", *op1);
    status[0]= (operand1==0);
    status[2] = (operand1 < 0);
        break;

    case 6://EOR
    printf("EOR | REG[%d](%d) ^= IMM(%d) | ", (int)op1 - (int)regs, *op1, imm);
    printf("REG[%d] = ", (int)op1 - (int)regs);
    operand1 ^= imm;
    *op1 = operand1;
    printf("%d\n", *op1);
    status[0]= (operand1==0);
    status[2] = (operand1 < 0);
        break;
    

    case 7://BR
        int16_t target = (operand1<<8);
        target |= operand2;
        printf("BR | BRANCHING TO: %d = %d || %d", target, operand1, operand2);
        dPC = PC;
        PC = target;
        xPC = PC;
        clearBuffers();
        break;
    
    case 8://SAL
        printf("SAL | REG[%d](%d) <<= IMM(%d) | ", (int)op1 - (int)regs, *op1, imm);
        printf("REG[%d] = ", (int)op1 - (int)regs);
        operand1 <<= imm;
        *op1 = operand1;
        printf("%d\n", *op1);
        status[0]= (operand1==0);
        status[2] = (operand1 < 0);
        break;

    case 9://SAR
        printf("SAL | REG[%d](%d) >>= IMM(%d) | ", (int)op1 - (int)regs, *op1, imm);
        printf("REG[%d] = ", (int)op1 - (int)regs);
        operand1 >>= imm;
        *op1 = operand1;
        printf("%d\n", *op1);
        status[0]= (operand1==0);
        status[2] = (operand1 < 0);
        break;

    case 10://LDR
        printf("LDR | REG[%d](%d) = MEM[%d](%d) | ", (int)op1 - (int)regs, *op1, (int)M - (int)datatMem, *M);
        printf("REG[%d] = ", (int)op1 - (int)regs);
        *op1 = *M;
        printf("%d\n", *op1);
        break;
    

    case 11://STR
        printf("STR | MEM[%d](%d) = REG[%d](%d) | ", (int)M - (int)datatMem, *M, (int)op1 - (int)regs, operand1);
        printf("MEM[%d] = ", (int)M - (int)datatMem);
        *M = operand1;
        printf("%d\n", *M);
        break;

    default:
        break;
    }
    
}

// prints the SREG
void printStatus(){
    for(int i=0;i<8;i++){
        printf("%d:  %d  |  ", i, status[i]);
    }
}

// Prints the whole Memory and REGs
void end(){
    if ( cycle >= maxiter)
        printf("terminated due to loop\n");
    else{
    printf("Cycle:  %d\n\n", cycle);
    execute();
    printf("No Instruction To Decode\n");
    println(300);
    printRegs();
    println(300);
    printDataMem();
    println(300);
    printInsMem();
    println(300);
}
}

// Prints REGs
void printRegs(){

    for(int i = 0; i<64; i++)
        printf("REG[%d]: %d | ", i, *(regs+i));
    printf("SREG: ");
    printStatus();
}

// Prints DATA MEMORY
void printDataMem(){
    for(int i = 0; i<2048; i++)
        printf("DATA[%d]: %d | ", i, (int8_t)*(datatMem+i));
}

// Prints INS MEMORY
void printInsMem(){
    for(int i = 0; i<1024; i++)
        printf("INS[%d]: %d | ", i, (int16_t)*(instMem+i));
}

// Handles Debuging at the start of a cycle
void startCycle(){
    char c;
    if(DEBUG){
        printf("Press Any Boutton To Step Forward\n");
        getch();
    }
    printf("PC: %p\n", PC);
    printf("Cycle:  %d\n\n", cycle);
}


void clearBuffers(){
    for(int i=0; i<2; i++){
        fetchedBuffer[i] = -1;
        for (int j =0;j<3;j++)
            decodedBuffer[i][j] = -1;
    }
}


void ptod(){// pointer to data
    op1 = (regs + (decodedBuffer[0][1]));
            op2 = (regs + (decodedBuffer[0][2]));
            operand1 = *op1;
            operand2 = *op2;
            M = datatMem + (decodedBuffer[0][2]);
            imm = (decodedBuffer[0][2]);
            twosComp(&imm);
}

// Main program Flow
int main(){
    init();
    while(!halt && cycle < maxiter){
        startCycle();

        pthread_create(&fetcher, NULL, &fetch, NULL);

        if((int16_t)fetchedBuffer[0] != -1)
            pthread_create(&decoder, NULL, &decode, NULL);
        else if (c > 0){
          ptod();
          printf("No Instruction To Decode\n");  
        }
        else
            printf("No Instruction To Decode\n");
        
        if((int8_t)decodedBuffer[0][0] != -1)
            pthread_create(&executer, NULL, &execute, NULL);
        else
            printf("No Instruction To Execute\n");
        endCycle();
    }
    end();
    return 0;
}


