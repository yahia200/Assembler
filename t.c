#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
pthread_t t1;
void c(){
    printf("asd");
}
int main(){
    pthread_create(&t1, NULL, c, NULL);
    pthread_join(t1, NULL);
}