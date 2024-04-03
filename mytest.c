#include "types.h"
#include "user.h"
#include "stat.h"

int
main(void)
{
    int i;
    printf(1, "=====ps test=====\n");
    for(int i=-1; i<11; i++) {
        ps(i);
    }

    printf(1, "=====getnice test=====\n");
    for(i=-1; i<11; i++) {
        printf(1, "nice: %d\n", getnice(i));
    }

    printf(1, "=====setnice test=====\n");
    for(i=-1; i<11; i++) {
        if(setnice(i, 24) == -1) {
            printf(1, "when pid: %d, error occured\n", i);
        }
    }

    printf(1, "=====ps test=====\n");
    for(int i=-1; i<11; i++) {
        ps(i);
    }
    exit();
}