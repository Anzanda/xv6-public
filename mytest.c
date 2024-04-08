#include "types.h"
#include "user.h"
#include "stat.h"


int MAX = 3e4;

int a[] = {1,3,2};
int b[30000];
int c[30000];

int
main(void)
{
    ps(0);
    for(int i=0; i<MAX; i++) {
        b[i] = a[(i%3)];
        c[i] = a[(i+1)%3];
    }

    if(fork() == 0) {
        printf(1, "I'm child\n");
        ps(0);
        for (int i = 0; i < MAX-1; i++) {
            for (int j = 0; j <MAX-i-1; j++) {
                if (b[j] > b[j+1]) {
                    int temp;
                    temp = b[j];
                    b[j] = b[j+1];
                    b[j+1] = temp;
                }
            }
        }
        ps(0);
        exit();
    } else {
        setnice(3, 30);
        printf(1, "I'm parent\n");
        ps(0);
        for (int i = 0; i < MAX-1; i++) {
            for (int j = 0; j <MAX-i-1; j++) {
                if (c[j] > c[j+1]) {
                    int temp;
                    temp = c[j];
                    c[j] = c[j+1];
                    c[j+1] = temp;
                }
            }
        }
        wait();
    }
     
    ps(0);
    exit();
}