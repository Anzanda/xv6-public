#include "types.h"
#include "user.h"
#include "stat.h"
#include "param.h"


void
test_file()
{
    uint addr = 0;
    int length = 4096*4; //2286
    int prot = PROT_READ;
    int flags = 0;
    int fd = open("README", 0);
    printf(1, "fd: %d\n", fd);
    int offset = 1;
 
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "a[0]: %c\n", a[0]);
    printf(1, "a[1]: %c\n", a[1]);
    printf(1, "a[2]: %c\n", a[2]);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "a[2287]: %c\n", a[2287]);
    printf(1, "a[2288]: %c\n", a[2288]);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "a[4097]: %c\n", a[4097]);
    printf(1, "a[4098]: %c\n", a[4098]);
    printf(1, "freemem: %d\n", freemem());
    close(fd);
}
void
test_anonymous()
{

}

int
main(void)
{
    // uint addr = 0;
    // int length = 4096*1; //2286
    // int prot = -1;
    // int flags = MAP_ANONYMOUS;
    // int fd = -1;
    // int offset = -1;

    // char *a = (char*)mmap(addr, length, prot, flags, fd, offset);
    // printf(1, "a[0]: %c\n", a[0]);
    
    test_file();
    exit();
}