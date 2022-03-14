#include <stdlib.h>
#include <stdio.h>

void f0();
void f1();
void f2();
void f3();

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("Please specify program.\n");
        return 0;
    }
    int pr = atoi(argv[1]);
    switch (pr) {
        case 0: 
            f0();
            break;
        case 1:
            f1();
            break;
        case 2:
            f2();
            break;
        case 3:
            f3();
            break;
    }
}

void f0() {
    char* x = malloc(1);
    *x = 'a';
    *(x+1) = 'r';
    *(x+2) = 'm';
    *(x+3) = 's';
    *(x+4) = '\0';
    *(x+320000000) = 'k';
    printf("%s\n", x);
    free(x);
}

void f1() {
}

void f2() {

}

void f3() {

}