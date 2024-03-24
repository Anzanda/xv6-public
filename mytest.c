#include "types.h"
#include "user.h"
#include "stat.h"

int
main(void)
{
    int i;
    for(i=1; i<11; i++) {
        if(setnice(i, 24) == -1)
            printf(1, "when pid: %d, error occured!\n", i);
    }

    exit();
}