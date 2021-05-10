#include <stdio.h>

int a[3] = {1, 2, 3};
int b[3] = {2, 3, 2};
int x[3];
int i = 0;

void cfcss_error_handler() {
    printf("CFCSS detected CFE, exiting program...\n");
    exit(1);
}

void print_line() {
    printf("Hello world!\n");
}

int main () {
    printf("1\n");
    while (i < 3) {
        printf("2\n");
        if (a[i] < b[i]) {
            printf("3\n");
            x[i] = a[i];
        }
        else {
            printf("4\n");
            x[i] = b[i];
        }
        printf("5\n");
        i++;
    }
    printf("6\n");
    print_line();
    printf("7\n");
    return 0;
}

// void setXA() {
//     x[i] = a[i];
// }

// void setXB() {
//     x[i] = b[i];
// }

// int main () {
//     while (i < 3) {
//         if (a[i] < b[i]) {
//             setXA();
//         }
//         else {
//             setXB();
//         }
//         i++;
//     }

//     return 0;
// }