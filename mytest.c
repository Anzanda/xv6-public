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
 
    printf(1, "before mmap freemem: %d\n", freemem());
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

    printf(1, "munmap return value: %d\n", munmap(a));
    printf(1, "after munmap freemem: %d\n", freemem());
    close(fd);
}
void
test_munmap()
{
    uint addr = 0;
    int length = 4096*4; //2286
    int prot = PROT_READ;
    int flags = 0;
    int fd = open("README", 0);
    printf(1, "fd: %d\n", fd);
    int offset = 1;
 
    printf(1, "before mmap freemem: %d\n", freemem());
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "a[0]: %c\n", a[0]);
    printf(1, "a[1]: %c\n", a[1]);
    printf(1, "a[2]: %c\n", a[2]);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "a[4097]: %c\n", a[4097]);
    printf(1, "a[4098]: %c\n", a[4098]);
    printf(1, "freemem: %d\n", freemem());

    printf(1, "munmap return value: %d\n", munmap(a));
    printf(1, "after munmap freemem: %d\n", freemem());

    char* b = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "b[0]: %c\n", b[0]);
    printf(1, "b[1]: %c\n", b[1]);
    printf(1, "b[2]: %c\n", b[2]);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "b[4097]: %c\n", b[4097]);
    printf(1, "b[4098]: %c\n", b[4098]);
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
    // test_file();
    test_munmap();
    exit();
}