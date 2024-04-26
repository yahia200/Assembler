#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

int8_t* operand1;
int8_t* operand2;
int16_t instMem[1024] = {0b1011000000000000,0b1011000001000001,0b0001000000000001, 62000};//halt = 62000
int8_t datatMem[2048]={0b10000000, 0b10000000};
int8_t regs[64]={0,10,5,6};
int fetched = 0;
int decoded = 0;
int executed = 0;
bool status[8];// 0=Zero, 1=Sign, 2=Negative, 3=Two’s Complement Overflow, 4=Carry
uint16_t* PC=instMem;
int8_t decodedOp[3];
uint16_t IR;
int cycle = 1;
bool halt = false;


int main(){
    while(!halt){
        printf("Cycle:  %d\n\n", cycle);
        if(decoded > 0)
            execute();
        if(fetched > 0)
            decode();
        fetch();
        endCycle();
    }
    end();


    return 0;
}

void init(){
    char const* const fileName = "code.txt";
    FILE* file = fopen(fileName, "r"); 
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        //n3ml 7aga f el "line"
        printf("%s", line); 
    }

    fclose(file);

    return 0;
}



void endCycle(){
    cycle++;
    println(150);
}


void fetch(){
    if(*PC!=62000){
    IR = *PC;
    printf("Fetching:  %d | Instruction: %d\n", ++fetched, IR);
    PC++;
    }
    else
        halt = true;
}


void decode(){
    printf("Decodinging: %d | ", ++decoded);
    decodedOp[0] = IR >> 12;
    decodedOp[1] = (IR & 0x03f0) >> 6;
    decodedOp[2] = (IR & 0x003f);
    operand1 = regs + (decodedOp[1]);
    operand2 = regs + (decodedOp[2]);
    
    printf("OPCode: %d | First Operand: %d | Second Operand: %d\n", decodedOp[0], decodedOp[1], decodedOp[2]);


}


void execute(){
    printf("Executing: %d | ", ++executed);
    int opCode = decodedOp[0];
    int imm;
    switch (opCode)
    {
    case 0://ADD
        status[4] = (((uint8_t)*operand1 + (uint8_t)*operand2)&(1<<8)) != 0;
        if(!((*operand1<0) ^ (*operand2<0))){
            status[3] = ((*operand1<0) ^ ((int8_t)(*operand2 + *operand1) < 0));
        }
        else status[3] = false;
        printf("REG%d: %d -> ", (int)operand1 - (int)regs, *operand1);
        *operand1 += *operand2;
        printf("%d\n", *operand1);
        status[0] = (*operand1==0);
        status[2] = (*operand1 < 0);
        status[1] = status[2] ^ status[3];

        break;

    case 1://SUB
        if(((*operand1<0) ^ (*operand2<0))){
            status[3] = ((*operand1<0) ^ ((int8_t)(*operand1 - *operand2) < 0));
        }
        else status[3] = false;
        printf("REG%d: %d -> ", (int)operand1 - (int)regs, *operand1);
        *operand1 -= *operand2;
        printf("%d\n", *operand1);
        status[0]= (*operand1==0);
        status[2] = (*operand1 < 0);
        status[1] = status[2] ^ status[3];
        break;

    case 2://MUL
        printf("REG%d: %d -> ", (int)operand1 - (int)regs, *operand1);
        *operand1 *= *operand2;
        printf("%d\n", *operand1);
        status[0]= (*operand1==0);
        status[2] = (*operand1 < 0);
        break;

    case 3://MOVI
        imm = decodedOp[2];
        twosComp(&imm);
        printf("REG%d: %d -> ", (int)operand1 - (int)regs, *operand1);
        *operand1 = imm;
        printf("%d\n", *operand1);
        break;

    case 4://BEQZ
        imm = decodedOp[2];
        twosComp(&imm);
        if(*operand1 == 0)
            PC += imm;
        break;

    case 5://ANDI
    imm = decodedOp[2];
    twosComp(&imm);
    printf("REG%d: %d -> ", (int)operand1 - (int)regs, *operand1);
    *operand1 &= imm;
    printf("%d\n", *operand1);
    status[0]= (*operand1==0);
    status[2] = (*operand1 < 0);
        break;

    case 6://EOR
    imm = decodedOp[2];
    twosComp(&imm);
    printf("REG%d: %d -> ", (int)operand1 - (int)regs, *operand1);
    *operand1 ^= imm;
    printf("%d\n", *operand1);
    status[0]= (*operand1==0);
    status[2] = (*operand1 < 0);
        break;
    

    case 7://BR
        int16_t target = (*operand1<<8);
        target |= *operand2;
        PC = target;
        break;
    
    case 8://SAL
        imm = decodedOp[2];
        twosComp(&imm);
        status[0]= (*operand1==0);
        status[2] = (*operand1 < 0);
        break;

    case 9://SAR
        imm = decodedOp[2];
        twosComp(&imm);
        printf("REG%d: %d -> ", (int)operand1 - (int)regs, *operand1);
        *operand1 >>= imm;
        printf("%d\n", *operand1);
        status[0]= (*operand1==0);
        status[2] = (*operand1 < 0);
        break;

    case 10://LDR
        operand2 = &datatMem[0] + (decodedOp[2]);
        printf("REG%d: %d -> ", (int)operand1 - (int)regs, *operand1);
        *operand1 = *operand2;
        printf("%d\n", *operand1);
        break;
    

    case 11://STR
        operand2 = &datatMem[0] + (decodedOp[2]);
        printf("MEM%d: %d -> ", (int)operand2 - (int)datatMem, *operand2);
        *operand2 = *operand1;
        printf("%d\n", *operand2);
        break;

    default:
        break;
    }
    
}


void twosComp(int8_t* num){
    const int negative = (*num & (1 << 5)) != 0;
    int nativeInt;

if (negative)
  *num |= 0xFFFFFFC0;
else
  nativeInt = num;
}


void printStatus(){
    for(int i=0;i<8;i++){
        printf("%d:  %d  |  ", i, status[i]);
    }
}


void end(){
    printf("Cycle:  %d\n\n", cycle);
    execute();
    println(300);
    printRegs();
    println(300);
    printDataMem();
    println(300);
    printInsMem();
    println(300);
}

void printRegs(){

    for(int i = 0; i<64; i++)
        printf("REG%d: %d | ", i, *(regs+i));
    printf("SREG: ");
    printStatus();
}

void printDataMem(){
    for(int i = 0; i<2048; i++)
        printf("DATA%d: %d | ", i, (int8_t)*(datatMem+i));
}

void printInsMem(){
    for(int i = 0; i<1024; i++)
        printf("INS%d: %d | ", i, (uint16_t)*(instMem+i));
}


void println(int x){
    printf("\n\n");
    for (int i = 0; i< x ;i++)
        printf("―");
    printf("\n\n");
}


