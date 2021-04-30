#include <stdio.h>

int a[3] = {1, 2, 3};
int b[3] = {2, 3, 2};
int x[3];
int i = 0;

int main () {
    while (i < 3) {
        if (a[i] < b[i]) {
            x[i] = a[i];
        }
        else {
            x[i] = b[i];
        }
        i++;
    }

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