#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(void){
    char *string;

	string = strdup("Hi");

    while(1){
        printf("%s\n",string);
        sleep(4);
    }
}