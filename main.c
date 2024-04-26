#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

int instMem[1024] = {0b0011000000111010,0b0000000001000010, 0b0001000001000011,0b1100000000000000};//halt = 49152
int datatMem[2048];
int regs[65]={0,10,5,4}; //[64] = STUTUS REG
uint16_t* PC=instMem;
int decodedOp[3];
uint16_t IR;
bool halt = false;
int tet= 0b00000000000000000000000000111010;


int main(){

    twosComp(&tet);
    printf("%d", tet);
    // while(!halt){
    // fetch();
    // printf("IR: %d\n", IR);
    // decode();
    // printf("op: %d\n", decodedOp[0]);
    // printf("reg1: %d\n", decodedOp[1]);
    // printf("reg1 data: %d\n", regs[decodedOp[1]]);
    // printf("reg2: %d\n", decodedOp[2]);
    // printf("reg2 data: %d\n", regs[2]);
    // execute();
    // printf("reg1: %d\n", regs[decodedOp[1]]);
    // incrmentPC();
    // }

    // printf("%d", regs[0]);
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
    int* operand1;
    int* operand2;
    int* status = &regs[64];
    int imm;
    switch (opCode)
    {
    case 0://add
        operand1 = &regs[0] + (decodedOp[1]);
        operand2 = &regs[0] + (decodedOp[2]);
        *operand1 += *operand2;
        if(*operand1 == 0)
            *status |= 0x0001;
        else
            *status &= 0x111E;

        break;

    case 1://sub
        operand1 = regs + decodedOp[1];
        operand2 = regs + decodedOp[2];
        *operand1 -= *operand2;
        if(*operand1 == 0)
            *status |= 0x0001;
        else
            *status &= 0x111E;
        break;

    case 2://mul
        operand1 = regs + decodedOp[1];
        operand2 = regs + decodedOp[2];
        *operand1 *= *operand2;
        if(*operand1 == 0)
            *status |= 0x0001;
        else
            *status &= 0x111E;
        break;

    case 3://mov imm
    operand1 = &regs[0] + (decodedOp[1]);
    imm = decodedOp[2];
    *operand1 = imm;
    if(*operand1 == 0)
            *status |= 0x0001;
        else
            *status &= 0x111E;
        break;

    case 4://branch if = 0
        operand1 = &regs[0] + (decodedOp[1]);
        imm = decodedOp[2];
        if(operand1 == 0)
            PC += imm;
        break;

    case 5://and imm
    operand1 = &regs[0] + (decodedOp[1]);
    imm = decodedOp[2];
    *operand1 &= imm;
    if(*operand1 == 0)
            *status |= 0x0001;
        else
            *status &= 0x111E;
        break;

    case 6://xor
    operand1 = &regs[0] + (decodedOp[1]);
    imm = decodedOp[2];
    *operand1 ^= imm;
    if(*operand1 == 0)
            *status |= 0x0001;
        else
            *status &= 0x111E;
        break;
    

    case 7://branch reg
        operand1 = &regs[0] + (decodedOp[1]);
        operand2 = &regs[0] + (decodedOp[2]);
        PC = *operand1 || *operand2;
        break;
    
    case 8://shift arith l
        operand1 = &regs[0] + (decodedOp[1]);
        imm = decodedOp[2];
        *operand1 <<= imm;
        if(*operand1 == 0)
            *status |= 0x0001;
        else
            *status &= 0x111E;
        break;

    case 9://shift arith r
        operand1 = &regs[0] + (decodedOp[1]);
        imm = decodedOp[2];
        *operand1 >>= imm;
        if(*operand1 == 0)
            *status |= 0x0001;
        else
            *status &= 0x111E;
        break;

    case 10://ld to reg
        operand1 = &regs[0] + (decodedOp[1]);
        operand2 = &datatMem[0] + (decodedOp[2]);
        *operand1 = *operand2;
        if(*operand1 == 0)
            *status |= 0x0001;
        else
            *status &= 0x111E;
        break;
    

    case 11://str from reg
        operand1 = &regs[0] + (decodedOp[1]);
        operand2 = &datatMem[0] + (decodedOp[2]);
        *operand2 = *operand1;
        //zero flag ?????
        break;

    case 12://halt
        halt = true;
        break;

    default:
        break;
    }
}

void twosComp(int* smallInt){
    const int negative = (*smallInt & (1 << 5)) != 0;
    printf("%d     twoooooooooooooo\n", negative);
    int nativeInt;

if (negative)
  *smallInt |= 0xFFFFFFC0;
else
  nativeInt = smallInt;
}


