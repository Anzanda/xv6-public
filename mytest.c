#include "types.h"
#include "user.h"
#include "stat.h"

int
main(void)
{
    int i;
    for(i=1; i<11; i++) {
        int nice = getnice(i);
        if(nice != -1)
            printf(1, "nice: %d\n", nice);
    }

    exit();
}