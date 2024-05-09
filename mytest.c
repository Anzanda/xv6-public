#include "types.h"
#include "user.h"
#include "stat.h"


int
main(void)
{
    printf(1, "%d", mmap(0x80000000,0,0,0,0,0));
    exit();
}