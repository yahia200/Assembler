#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>

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

int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}