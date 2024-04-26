#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

int16_t instMem[1024] = {0b0100000000111010,0b0001000000000011};
int8_t datatMem[2048];
int8_t regs[64]={0,10,5,6};
bool status[8];// 0=Zero, 1=Sign, 2=Negative, 3=Two’s Complement Overflow, 4=Carry
uint16_t* PC=instMem;
int8_t decodedOp[3];
uint16_t IR;
int cycle = 0;
bool halt = false;


int main(){
    fetch();
    while(IR != 0){
        printf("Cycle: %d\n\n", cycle);
        printf("IR: %d\n", IR);
        decode();
        printf("op: %d\n", decodedOp[0]);
        printf("Operand1: %d\n", decodedOp[1]);
        printf("Operand2: %d\n", decodedOp[2]);
        execute();
        printStatus();
        incrmentPC();
        fetch();
        cycle++;
        printf("\n\n\n");
    }
    return 0;
}

void incrmentPC(){
    PC+=1;
}

void fetch(){
    IR = *PC;
}

void decode(){
    decodedOp[0] = IR >> 12;
    decodedOp[1] = (IR & 0x03f0) >> 6;
    twosComp(&decodedOp[1]);
    decodedOp[2] = (IR & 0x003f);
    twosComp(&decodedOp[2]);


}
void execute(){
    int opCode = decodedOp[0];
    int8_t* operand1;
    int8_t* operand2;
    int imm;
    switch (opCode)
    {
    case 1://ADD
        operand1 = &regs[0] + (decodedOp[1]);
        operand2 = &regs[0] + (decodedOp[2]);
        *operand1 += *operand2;
        status[0]= (*operand1==0);

        break;

    case 2://SUB
        operand1 = regs + decodedOp[1];
        operand2 = regs + decodedOp[2];
        *operand1 -= *operand2;
        status[0]= (*operand1==0);
        break;

    case 3://MUL
        operand1 = regs + decodedOp[1];
        operand2 = regs + decodedOp[2];
        *operand1 *= *operand2;
        status[0]= (*operand1==0);
        break;

    case 4://MOVI
    operand1 = &regs[0];
    imm = decodedOp[2];
    *operand1 = imm;
        break;

    case 5://BEQZ
        operand1 = &regs[0] + (decodedOp[1]);
        imm = decodedOp[2];
        if(*operand1 == 0)
            PC += imm;
        break;

    case 6://ANDI
    operand1 = &regs[0] + (decodedOp[1]);
    imm = decodedOp[2];
    *operand1 &= imm;
    status[0]= (*operand1==0);
        break;

    case 7://EOR
    operand1 = &regs[0] + (decodedOp[1]);
    imm = decodedOp[2];
    *operand1 ^= imm;
    status[0]= (*operand1==0);
        break;
    

    case 8://BR
        operand1 = &regs[0] + (decodedOp[1]);
        operand2 = &regs[0] + (decodedOp[2]);
        PC = *operand1 || *operand2;
        break;
    
    case 9://SAL
        operand1 = &regs[0] + (decodedOp[1]);
        imm = decodedOp[2];
        status[0]= (*operand1==0);
        break;

    case 10://SAR
        operand1 = &regs[0] + (decodedOp[1]);
        imm = decodedOp[2];
        *operand1 >>= imm;
        status[0]= (*operand1==0);
        break;

    case 11://LDR
        operand1 = &regs[0] + (decodedOp[1]);
        operand2 = &datatMem[0] + (decodedOp[2]);
        *operand1 = *operand2;
        break;
    

    case 12://STR
        operand1 = &regs[0] + (decodedOp[1]);
        operand2 = &datatMem[0] + (decodedOp[2]);
        *operand2 = *operand1;
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


