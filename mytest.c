#include "types.h"
#include "user.h"
#include "stat.h"
#include "param.h"
#include "fcntl.h"
#include "memlayout.h"

void
test_fd_when_anonymous()
{
    uint addr = 0;
    int length = 4096; //2286
    int prot = PROT_READ;
    int flags = MAP_ANONYMOUS;
    int fd = 3;
    int offset = 1;
 
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "expected a: 0\n");
    printf(1, "actual a: %d\n", a);

    close(fd);
}
void
test_offset_when_anonymous()
{
    uint addr = 0;
    int length = 4096; //2286
    int prot = PROT_READ;
    int flags = MAP_ANONYMOUS;
    int fd = -1;
    int offset = 1;
 
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "expected a: 0\n");
    printf(1, "actual a: %d\n", a);

    close(fd);

}
void
test_fd_when_not_anonymous()
{
    uint addr = 0;
    int length = 4096; //2286
    int prot = PROT_READ;
    int flags = 0;
    int fd = -1;
    int offset = 1;
 
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "expected a: 0\n");
    printf(1, "actual a: %d\n", a);

    close(fd);
}
// 주어지는 addr가 page aligned하지 않음.
void
test_mmap_when_not_page_aligned()
{
    uint addr = 0+1024;
    int length = 4096; //2286
    int prot = PROT_READ;
    int flags = 0;
    int fd = -1;
    int offset = 1;
 
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "expected a: 0\n");
    printf(1, "actual a: %d\n", a);

    close(fd);

}
// 주어지는 length가 multiple size of page가 아님.
void
test_mmap_when_not_page_aligned2()
{
    uint addr = 0;
    int length = 2286; //2286
    int prot = PROT_READ;
    int flags = 0;
    int fd = -1;
    int offset = 1;
 
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "expected a: 0\n");
    printf(1, "actual a: %d\n", a);

    close(fd);
}
// mmap의 권한이 부족해
void
test_file_permission()
{
    uint addr = 0;
    int length = 4096; //2286
    int prot = PROT_READ;
    int flags = 0;
    int fd = open("README", O_RDWR);
    int offset = 1;
 
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "expected a: %d\n", addr + MMAPBASE);
    printf(1, "actual a: %d\n", a);

    close(fd);
}
// mmap이랑 권한 딱맞아
void
test_file_permission2()
{
    uint addr = 0;
    int length = 4096; //2286
    int prot = PROT_READ | PROT_WRITE;
    int flags = 0;
    int fd = open("README", O_RDWR);
    int offset = 1;
 
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "expected a: %d\n", addr + MMAPBASE);
    printf(1, "actual a: %d\n", a);

    close(fd);
}
// mmap이 권한 더 많아.
void
test_file_permission3()
{
    uint addr = 0;
    int length = 4096; //2286
    int prot = PROT_READ | PROT_WRITE;
    int flags = 0;
    int fd = open("README", O_RDONLY);
    int offset = 1;
 
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "expected a: %d\n", 0);
    printf(1, "actual a: %d\n", a);

    close(fd);
}

// mmap의 file mapping을 read-only로 설정하고, write를 했을 때 process가 종료돼야함.
void
test_mmap_with_RDONLY()
{
    uint addr = 0;
    int length = 4096; //2286
    int prot = PROT_READ;
    int flags = 0;
    int fd = open("README", O_RDONLY);
    int offset = 1;
 
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    a[0] = 'S';
    a[1] = 'K';
    a[2] = 'K';
    a[3] = 'U';

    close(fd);
}
void
test_mmap_when_anonymous_with_populate()
{
    uint addr = 0;
    int length = 4096*2; //2286
    int prot = PROT_READ;
    int flags = MAP_ANONYMOUS | MAP_POPULATE;
    int fd = -1;
    int offset = 0;
 
    printf(1, "before mmap freemem: %d\n", freemem());
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "a: %d\n", a);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "a[0]: %c\n", a[0]);
    printf(1, "a[1]: %c\n", a[1]);
    printf(1, "a[2]: %c\n", a[2]);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "should not be freemem increased after a[2287*2+0]\n");
    printf(1, "freemem: %d\n", freemem());
    printf(1, "a[2287*2+0]: %c\n", a[2287*2+0]);
    printf(1, "a[2287*2+1]: %c\n", a[2287*2+1]);
    printf(1, "freemem: %d\n", freemem());
}
void
test_mmap_when_anonymous_without_populate()
{
    uint addr = 0;
    int length = 4096*2; //2286
    int prot = PROT_READ;
    int flags = MAP_ANONYMOUS;
    int fd = -1;
    int offset = 0;
 
    printf(1, "before mmap freemem: %d\n", freemem());
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "a: %d\n", a);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "a[0]: %c\n", a[0]);
    printf(1, "a[1]: %c\n", a[1]);
    printf(1, "a[2]: %c\n", a[2]);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "should be freemem increased after a[2287*2+0]\n");
    printf(1, "freemem: %d\n", freemem());
    printf(1, "a[2287*2+0]: %c\n", a[2287*2+0]);
    printf(1, "a[2287*2+1]: %c\n", a[2287*2+1]);
    printf(1, "freemem: %d\n", freemem());
}
// page fault에서 read만 가능한데, write를 하셨을 때.
void
test_page_fault_when_perm_not_equal()
{
    uint addr = 0;
    int length = 4096; //2286
    int prot =  PROT_READ;
    int flags = 0;
    int fd = open("README", O_RDONLY);
    int offset = 1;
 
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    a[0] = 'S';
    a[1] = 'K';
    a[2] = 'K';
    a[3] = 'U';

    close(fd);
}


// page fault에서 write만 가능한데, read를 하셨을 때.
void
test_page_fault_when_perm_not_equal2()
{
    uint addr = 0;
    int length = 4096; //2286
    int prot =  PROT_WRITE;
    int flags = 0;
    int fd = open("README", O_WRONLY);
    int offset = 1;
 
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "a[0]: %c\n", a[0]);

    close(fd);
}

// page_fault가 발생하는데 mmap_area를 벗어난 영역에 접근했을때.
void
test_page_fault_out_of_bound()
{
    uint addr = 0;
    int length = 4096; //2286
    int prot =  PROT_READ;
    int flags = 0;
    int fd = open("README", O_RDWR);
    int offset = 1;
 
    // 유효범위: [0, 4095]
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "a[4096]: %c\n", a[4096]);
    printf(1, "a[4097]: %c\n", a[4097]);

    close(fd);

}

// 같은 파일을 2개의 mmap에서 control함.
void
test_mmap_when_one_file_another_mmap()
{
    uint addr = 0;
    int length = 4096; //2286
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
    printf(1, "a[2200]: %c\n", a[2200]);
    printf(1, "a[2201]: %c\n", a[2201]);
    printf(1, "freemem: %d\n", freemem());

    // printf(1, "munmap return value: %d\n", munmap((uint)a));
    // printf(1, "after munmap freemem: %d\n", freemem());

    char* b = (char*)mmap(addr+length, length, prot, flags, fd, offset);
    printf(1, "b[0]: %c\n", b[0]);
    printf(1, "b[1]: %c\n", b[1]);
    printf(1, "b[2]: %c\n", b[2]);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "b[2200]: %c\n", b[2200]);
    printf(1, "b[2201]: %c\n", b[2201]);
    printf(1, "freemem: %d\n", freemem());
    
    printf(1, "b - a: %d\n", b - a);

    close(fd);
}

// page fault가 base_addr와 다른 위치에서 발생함.
void
test_page_fault_not_same_page_with_base_addr()
{
    uint addr = 0;
    int length = 4096*2; //2286
    int prot = PROT_READ;
    int flags = 0;
    int fd = open("README", 0);
    int offset = 1;
 
    printf(1, "before mmap freemem: %d\n", freemem());
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "a[0]: %c\n", a[0]);
    printf(1, "a[1]: %c\n", a[1]);
    printf(1, "a[2]: %c\n", a[2]);
    printf(1, "freemem: %d\n", freemem());

    //2287부터 시작
    printf(1, "a[2200]: %c\n", a[2287 + 0]);
    printf(1, "a[2201]: %c\n", a[2287 + 1]);
    printf(1, "freemem: %d\n", freemem());

    printf(1, "a[4572]: %c\n", a[2287*2 + 0]);
    printf(1, "a[4573]: %c\n", a[2287*2 + 1]);
    printf(1, "freemem: %d\n", freemem());

    close(fd);
}

// munmap 성공한 경우.
void
test_success_munmap()
{
    uint addr = 0;
    int length = 4096*2; //2286
    int prot = PROT_READ;
    int flags = MAP_POPULATE;
    int fd = open("README", 0);
    int offset = 1;
 
    printf(1, "before mmap freemem: %d\n", freemem());
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "a[0]: %c\n", a[0]);
    printf(1, "a[1]: %c\n", a[1]);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "a[4096]: %c\n", a[4096]);
    printf(1, "a[4097]: %c\n", a[4097]);
    printf(1, "freemem: %d\n", freemem());

    printf(1, "munmap return value: %d\n", munmap((uint)a));
    printf(1, "after munmap freemem: %d\n", freemem());

    close(fd);
}

// munmap에서 page aligned하지 않은 주소가 주어졌을 때.
void
test_munmap_when_not_page_aligned()
{
    printf(1, "expected: 0\n");
    printf(1, "actual: %d\n", munmap((uint)MMAPBASE+1));
}

// munmap()을 하고 해당 주소에 다시 접근하면 page fault가 나야함. 그런데 발생하지 않는다..
void
test_munmap_and_re_page_fault()
{
    uint addr = 0;
    int length = 4096; //2286
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
    printf(1, "a[2200]: %c\n", a[2200]);
    printf(1, "a[2201]: %c\n", a[2201]);
    printf(1, "freemem: %d\n", freemem());

    printf(1, "munmap return value: %d\n", munmap((uint)a));
    printf(1, "after munmap freemem: %d\n", freemem());

    printf(1, "a[0]: %c\n", a[0]);
    printf(1, "a[1]: %c\n", a[1]);
    printf(1, "a[2]: %c\n", a[2]);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "a[2200]: %c\n", a[2200]);
    printf(1, "a[2201]: %c\n", a[2201]);
    printf(1, "freemem: %d\n", freemem());

    close(fd);

}
// populated로 주면 자동으로 다 받아와야됨.
void
test_fork_when_populated()
{
    uint addr = 0;
    int length = 4096*2; //2286
    int prot = PROT_READ;
    int flags = MAP_POPULATE;
    int fd = open("README", 0);
    int offset = 1;

    printf(1, "before mmap freemem: %d\n", freemem());
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "after mmap freemem: %d\n", freemem());
    if (fork() == 0)
    {
        printf(1, "freemem: %d\n", freemem());
        printf(1, "a[0]: %c\n", a[0]);
        printf(1, "a[1]: %c\n", a[1]);
        printf(1, "a[2]: %c\n", a[2]);
        printf(1, "freemem: %d\n", freemem());
        exit();
    }
    else
    {
        wait();
        printf(1, "freemem: %d\n", freemem());
        printf(1, "a[0]: %c\n", a[0]);
        printf(1, "a[1]: %c\n", a[1]);
        printf(1, "a[2]: %c\n", a[2]);
        printf(1, "freemem: %d\n", freemem());
    }

    close(fd);
}
// unpopulated주고 자식에서 page fault났을 때 대처가 잘 되는가?
void
test_fork_when_page_fault()
{
    uint addr = 0;
    int length = 4096*2; //2286
    int prot = PROT_READ;
    int flags = 0;
    int fd = open("README", 0);
    int offset = 1;

    printf(1, "before mmap freemem: %d\n", freemem());
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    if (fork() == 0)
    {
        printf(1, "freemem: %d\n", freemem());
        printf(1, "a[0]: %c\n", a[0]);
        printf(1, "a[1]: %c\n", a[1]);
        printf(1, "a[2]: %c\n", a[2]);
        printf(1, "freemem: %d\n", freemem());
        exit();
    }
    else
    {
        wait();
        printf(1, "freemem: %d\n", freemem());
        printf(1, "a[0]: %c\n", a[0]);
        printf(1, "a[1]: %c\n", a[1]);
        printf(1, "a[2]: %c\n", a[2]);
        printf(1, "freemem: %d\n", freemem());
    }

    close(fd);
}
// file mapping | unpopulated에서 페이지 1번만 page fault로 불러오고 fork()했을 때,
void
test_fork_when_occur_page_fault_in_parent_before_fork()
{
    uint addr = 0;
    int length = 4096*2; //2286
    int prot = PROT_READ;
    int flags = 0;
    int fd = open("README", 0);
    int offset = 1;

    printf(1, "before mmap freemem: %d\n", freemem());
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "a[0]: %c\n", a[0]);
    printf(1, "a[1]: %c\n", a[1]);
    printf(1, "a[2]: %c\n", a[2]);
    printf(1, "freemem: %d\n", freemem());
    if (fork() == 0)
    {
        printf(1, "should not trap page fault in child\n");
        printf(1, "freemem: %d\n", freemem());
        printf(1, "a[0]: %c\n", a[0]);
        printf(1, "a[1]: %c\n", a[1]);
        printf(1, "a[2]: %c\n", a[2]);
        printf(1, "freemem: %d\n", freemem());

        printf(1, "But this time should trap page fault in child!!!\n");
        printf(1, "freemem: %d\n", freemem());
        printf(1, "a[2287*2+0]: %c\n", a[2287*2+0]);
        printf(1, "a[2287*2+1]: %c\n", a[2287*2+1]);
        printf(1, "freemem: %d\n", freemem());
        exit();
    }
    else
    {
        wait();
    }

    close(fd);
}
void
test_fork_when_parent_mmap_more_than_once()
{
    uint addr = 0;
    int length = 4096; //2286
    int prot = PROT_READ;
    int flags = 0;
    int fd = open("README", 0);
    int offset = 1;

    printf(1, "before mmap freemem: %d\n", freemem());
    char* a = (char*)mmap(addr, length, prot, flags, fd, offset);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "a[0]: %c\n", a[0]);
    printf(1, "a[1]: %c\n", a[1]);
    printf(1, "a[2]: %c\n", a[2]);
    printf(1, "freemem: %d\n", freemem());

    char* b = (char*)mmap(addr+length, length, prot, flags, fd, offset);
    printf(1, "freemem: %d\n", freemem());
    printf(1, "b[0]: %c\n", b[0]);
    printf(1, "b[1]: %c\n", b[1]);
    printf(1, "freemem: %d\n", freemem());
    if (fork() == 0)
    {
        printf(1, "should not trap page fault in child\n");
        printf(1, "freemem: %d\n", freemem());
        printf(1, "a[0]: %c\n", a[0]);
        printf(1, "a[1]: %c\n", a[1]);
        printf(1, "b[0]: %c\n", b[0]);
        printf(1, "b[1]: %c\n", b[1]);
        printf(1, "freemem: %d\n", freemem());
        exit();
    }
    else
    {
        wait();
    }

    close(fd);
}
int
main(void)
{
    // test_file();
    // test_fd_when_anonymous();
    // test_offset_when_anonymous();
    // test_fd_when_not_anonymous();
    // test_mmap_when_not_page_aligned();
    // test_mmap_when_not_page_aligned2();
    // test_file_permission();
    // test_file_permission2();
    // test_file_permission3();

    // test_mmap_with_RDONLY();
    // test_mmap_when_anonymous_with_populate();
    // test_mmap_when_anonymous_without_populate();
    // test_mmap_when_one_file_another_mmap();

    /* page fault 권한 체크 */
    // test_page_fault_when_perm_not_equal();
    // test_page_fault_when_perm_not_equal2();

    /* page fault 에러 체크*/
    // test_page_fault_out_of_bound();
    // test_page_fault_not_same_page_with_base_addr();

    /* munmap 테스트 */
    // test_success_munmap();
    // test_munmap_when_not_page_aligned();
    // test_munmap_and_re_page_fault();

    // /* fork 테스트 */
    // test_fork_when_populated();
    // test_fork_when_page_fault();
    // test_fork_when_occur_page_fault_in_parent_before_fork();
    // test_fork_when_parent_mmap_more_than_once();
    exit();
}