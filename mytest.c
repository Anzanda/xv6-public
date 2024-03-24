#include "types.h"
#include "user.h"
#include "stat.h"

int
main(void)
{
    int i;
    for(i=1; i<11; i++) {
        printf(1, "%d: ", i);
        if(getpname(i)) {
            printf(1, "wrong pid\n");
        }
    }

    exit();
}