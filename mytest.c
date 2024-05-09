#include "types.h"
#include "user.h"
#include "stat.h"


int
main(void)
{
    printf(1, "freemem: %d\n", freemem());
    malloc(1);
    printf(1, "freemem: %d\n", freemem());
    exit();
}