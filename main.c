#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

int8_t* operand1;
int8_t* operand2;
uint16_t instMem[1024];//halt = 62000
int8_t datatMem[2048]={7, 5};
int8_t regs[64];
int fetched = 0;
int decoded = 0;
int executed = 0;
bool status[8];// 0=Zero, 1=Sign, 2=Negative, 3=Two’s Complement Overflow, 4=Carry
uint16_t* PC=instMem;
int8_t decodedBuffer[2][3] = {{-1},{-1}};
uint16_t fetchedBuffer[2] = {-1, -1};
int cycle = 1;
bool halt = false;
pthread_t fetcher, decoder, executer;
int DEBUG = -1;



void init(){
    char splitLine[10][10];
    char const* const fileName = "code.txt"; /* should check that argc > 1 */
    FILE* file = fopen(fileName, "r"); /* should check the result */
    char line[256];
    int c = 0;

    while (fgets(line, sizeof(line), file)) {
        split(line, splitLine);
        addToInsMem(splitLine, c);
        c++;
        

        
    }
    instMem[c] = 62000;

    fclose(file);
    printf("Please Select Mode.\n0: run code.\n1: debug each cycle.\n");
    while(DEBUG < 0 || DEBUG > 1){
        printf("Mode: ");
        scanf("%d", &DEBUG);
    }
}


void split(char str1[], char splitLine[10][10]){
    int i, j, ctr;

    j = 0;
    ctr = 0;

    for (i = 0; i <= (strlen(str1)); i++) {
        if (str1[i] == ' ' || str1[i] == '\0') {
            splitLine[ctr][j] = '\0';
            ctr++;
            j = 0;
        } else {
            splitLine[ctr][j] = str1[i];
            j++;
        }
    }
}


int parseOP(char OP[10]){
    char OPs[12][10] = {"ADD","SUB", "MUL", "MOVI", "BEQZ", "ANDI", "EOR", "BR", "SAL", "SAR", "LDR", "STR"};

    for (int i = 0; i < 12; i++){
        if(strcmp(OP, OPs[i]) == 0)
            return (int)i;
    }
}


int parseOperand(char OP[10]){
    uintmax_t num = strtoumax(OP, NULL, 10);
    
}


void addToInsMem(char splitLine[10][10], int c){
        int16_t OP = parseOP(splitLine[0]);
        int16_t operand1 = parseOperand(splitLine[1]);
        int16_t operand2 = parseOperand(splitLine[2]);
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
}

void shiftBuffers(){
    fetchedBuffer[0] = fetchedBuffer[1];
    fetchedBuffer[1] = 0;
    decodedBuffer[0][0] = decodedBuffer[1][0];decodedBuffer[0][1] = decodedBuffer[1][1];decodedBuffer[0][2] = decodedBuffer[1][2];
    decodedBuffer[1][0] = 0;decodedBuffer[1][1] = 0;decodedBuffer[1][2] = 0;
}

void fetch(){
    if(*PC!=62000){
    fetchedBuffer[1] = *PC;
    printf("Fetched:  %d | Instruction: %d\n", ++fetched, fetchedBuffer[1]);
    PC++;
    }
    else{
        halt = true;
        printf("No Instruction TO Fetch\n");
    }
}


void decode(){
    printf("Decoded: %d | ", ++decoded);
    decodedBuffer[1][0] = fetchedBuffer[0] >> 12;
    decodedBuffer[1][1] = (fetchedBuffer[0] & 0x03f0) >> 6;
    decodedBuffer[1][2] = (fetchedBuffer[0] & 0x003f);
    
    printf("OPCode: %d | First Operand: %d | Second Operand: %d\n", decodedBuffer[1][0], decodedBuffer[1][1], decodedBuffer[1][2]);


}


void execute(){
    operand1 = regs + (decodedBuffer[0][1]);
    operand2 = regs + (decodedBuffer[0][2]);
    printf("Executed: %d | ", ++executed);
    int opCode = decodedBuffer[0][0];
    int imm;
    switch (opCode)
    {
    case 0://ADD
        status[4] = (((uint8_t)*operand1 + (uint8_t)*operand2)&(1<<8)) != 0;
        if(!((*operand1<0) ^ (*operand2<0))){
            status[3] = ((*operand1<0) ^ ((int8_t)(*operand2 + *operand1) < 0));
        }
        else status[3] = false;
        printf("ADD | REG[%d](%d) += REG[%d](%d) | ", (int)operand1 - (int)regs, *operand1, (int)operand2 - (int)regs, *operand2);
        printf("REG[%d] = ", (int)operand1 - (int)regs);
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
        printf("SUB | REG[%d](%d) -= REG[%d](%d) | ", (int)operand1 - (int)regs, *operand1, (int)operand2 - (int)regs, *operand2);
        printf("REG[%d] = ", (int)operand1 - (int)regs);
        *operand1 -= *operand2;
        printf("%d\n", *operand1);
        status[0]= (*operand1==0);
        status[2] = (*operand1 < 0);
        status[1] = status[2] ^ status[3];
        break;

    case 2://MUL
        printf("MUL | REG[%d](%d) *= REG[%d](%d) | ", (int)operand1 - (int)regs, *operand1, (int)operand2 - (int)regs, *operand2);
        printf("REG[%d] = ", (int)operand1 - (int)regs);
        *operand1 *= *operand2;
        printf("%d\n", *operand1);
        status[0]= (*operand1==0);
        status[2] = (*operand1 < 0);
        break;

    case 3://MOVI
        imm = decodedBuffer[0][2];
        twosComp(&imm);
        printf("MOVI | REG[%d](%d) = IMM(%d) | ", (int)operand1 - (int)regs, *operand1, imm);
        printf("REG[%d] = ", (int)operand1 - (int)regs);
        *operand1 = imm;
        printf("%d\n", *operand1);
        break;

    case 4://BEQZ
        imm = decodedBuffer[0][2];
        twosComp(&imm);
        if(*operand1 == 0){
            PC += imm;
            printf("BEQZ | BRANCHING TO: %d", PC);
        }
        printf("BEQZ | NO BRANCHING");
        break;

    case 5://ANDI
    imm = decodedBuffer[0][2];
    twosComp(&imm);
    printf("ANDI | REG[%d](%d) &= IMM(%d) | ", (int)operand1 - (int)regs, *operand1, imm);
    printf("REG[%d] = ", (int)operand1 - (int)regs);
    *operand1 &= imm;
    printf("%d\n", *operand1);
    status[0]= (*operand1==0);
    status[2] = (*operand1 < 0);
        break;

    case 6://EOR
    imm = decodedBuffer[0][2];
    twosComp(&imm);
    printf("EOR | REG[%d](%d) ^= IMM(%d) | ", (int)operand1 - (int)regs, *operand1, imm);
    printf("REG[%d] = ", (int)operand1 - (int)regs);
    *operand1 ^= imm;
    printf("%d\n", *operand1);
    status[0]= (*operand1==0);
    status[2] = (*operand1 < 0);
        break;
    

    case 7://BR
        int16_t target = (*operand1<<8);
        target |= *operand2;
        printf("BR | BRANCHING TO: %d = %d || %d", target, *operand1, *operand2);
        PC = target;
        break;
    
    case 8://SAL
        imm = decodedBuffer[0][2];
        twosComp(&imm);
        printf("SAL | REG[%d](%d) <<= IMM(%d) | ", (int)operand1 - (int)regs, *operand1, imm);
        printf("REG[%d] = ", (int)operand1 - (int)regs);
        *operand1 <<= imm;
        printf("%d\n", *operand1);
        status[0]= (*operand1==0);
        status[2] = (*operand1 < 0);
        break;

    case 9://SAR
        imm = decodedBuffer[0][2];
        twosComp(&imm);
        printf("SAL | REG[%d](%d) >>= IMM(%d) | ", (int)operand1 - (int)regs, *operand1, imm);
        printf("REG[%d] = ", (int)operand1 - (int)regs);
        *operand1 >>= imm;
        printf("%d\n", *operand1);
        status[0]= (*operand1==0);
        status[2] = (*operand1 < 0);
        break;

    case 10://LDR
        operand2 = &datatMem[0] + (decodedBuffer[0][2]);
        printf("LDR | REG[%d](%d) = MEM[%d](%d) | ", (int)operand1 - (int)regs, *operand1, (int)operand2 - (int)datatMem, *operand2);
        printf("REG[%d] = ", (int)operand1 - (int)regs);
        *operand1 = *operand2;
        printf("%d\n", *operand1);
        break;
    

    case 11://STR
        operand2 = &datatMem[0] + (decodedBuffer[0][2]);
        printf("STR | MEM[%d](%d) = REG[%d](%d) | ", (int)operand2 - (int)datatMem, *operand2, (int)operand1 - (int)regs, *operand1);
        printf("MEM[%d] = ", (int)operand2 - (int)datatMem);
        *operand2 = *operand1;
        printf("%d\n", *operand2);
        break;

    default:
        break;
    }
    
}


void twosComp(int8_t* num){
    const int negative = (*num & (1 << 5)) != 0;

if (negative)
  *num |= 0xFFFFFFC0;
}


void printStatus(){
    for(int i=0;i<8;i++){
        printf("%d:  %d  |  ", i, status[i]);
    }
}


void end(){
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


void printRegs(){

    for(int i = 0; i<64; i++)
        printf("REG[%d]: %d | ", i, *(regs+i));
    printf("SREG: ");
    printStatus();
}


void printDataMem(){
    for(int i = 0; i<2048; i++)
        printf("DATA[%d]: %d | ", i, (int8_t)*(datatMem+i));
}


void printInsMem(){
    for(int i = 0; i<1024; i++)
        printf("INS[%d]: %d | ", i, (uint16_t)*(instMem+i));
}


void println(int x){
    printf("\n\n");
    for (int i = 0; i< x ;i++)
        printf("―");
    printf("\n\n");
}




void startCycle(){
    char c;
    if(DEBUG){
        printf("Press Enter To Step Forward");
        scanf("%c", &c);
    }
    printf("Cycle:  %d\n\n", cycle);
}


int main(){
    init();
    while(!halt){
        startCycle();
        if(decodedBuffer[0][0] != -1)
            pthread_create(&executer, NULL, &execute, NULL);
        else
            printf("No Instruction To Execute\n");
        if((int16_t)fetchedBuffer[0] != -1)
            pthread_create(&decoder, NULL, &decode, NULL);
        else
            printf("No Instruction To Decode\n");
        pthread_create(&fetcher, NULL, &fetch, NULL);
        endCycle();
    }
    end();


    return 0;
}


