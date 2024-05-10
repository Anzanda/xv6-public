#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


int
sys_getpname(void)
{
  int pid;
  
  if(argint(0, &pid) < 0)
    return -1;
  
  return getpname(pid);
}

int
sys_ps(void)
{
  int pid;
  
  if(argint(0, &pid) < 0)
    return -1;
    
  ps(pid);
  return 0;
}

int
sys_getnice(void)
{
  int pid;
  
  if(argint(0, &pid) < 0)
    return -1;
  
  return getnice(pid);
}

int
sys_setnice(void)
{
  int pid;
  int val;
  
  if(argint(0, &pid) < 0)
    return -1;
  if(argint(1, &val) < 0)
    return -1;
  
  return setnice(pid, val);
}

int
sys_mmap(void)
{
  int addr;
  int length;
  int prot;
  int flags;
  int fd;
  int offset;

  if(argint(0, &addr) < 0)
    return -1;
  if(argint(1, &length) < 0)
    return -1;
  if(argint(2, &prot) < 0)
    return -1;
  if(argint(3, &flags) < 0)
    return -1;
  if(argint(4, &fd) < 0)
    return -1;
  if(argint(5, &offset) < 0)
    return -1;

  return mmap(addr, length, prot, flags, fd, offset);
}

int
sys_freemem(void)
{
  return freemem();
}

int
sys_munmap(void)
{
  uint addr;
  if(argint(0, &addr) < 0)
    return -1;

  return munmap(addr);  
}