#include <stdio.h>

int a[3] = {1, 2, 3};
int b[3] = {2, 3, 2};
int x[3];
int i = 0;

void cfcss_error_handler() {
    printf("CFE detected, exiting program...\n");
    exit(1);
}

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
