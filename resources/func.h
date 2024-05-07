#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <limits.h>

int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

void twosComp(int8_t* num){
    const int negative = (*num & (1 << 5)) != 0;

if (negative)
  *num |= 0xFFFFFFC0;
}

void println(int x){
    printf("\n\n");
    for (int i = 0; i< x ;i++)
        printf("―");
    printf("\n\n");
}

void split(char str1[], char splitLine[3][10]){
    int i, j, ctr;
    int f = 1;// true on first char
    j = 0;
    ctr = 0;

    for (i = 0; i <= (strlen(str1)); i++) {
        if (str1[i] == ' ' || str1[i] == '\0') {
            f = 1;
            splitLine[ctr][j] = '\0';
            ctr++;
            j = 0;
        } else {
            if(ctr > 0 && j == 0 && str1[i] == 'R' && f){
                f = 0;
                continue;
            }
            splitLine[ctr][j] = str1[i];
            j++;
        }
    }
}

int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}


void initIns(uint16_t* instMem){
    for (int i=0;i<1024;i++){
        instMem[i] = -1;
    }
}


void clearLine(char splitLine[3][10]){
    for (int i=0;i<3;i++){
        for (int j=0;j<10;j++){
            splitLine[i][j] = '^';
    }
    }
}

int isInt(char i){
    return (i >= '0' && i <= '9');
}

int intob(uint16_t n)
{
    char b[(sizeof n * CHAR_BIT) + 1];
    char *p = b + sizeof b;
    *--p = '\0';
    for (; p-- > b;  n >>= 1) {
        *p = '0' + (char)(n & 1);
    }
    return puts(b);
}