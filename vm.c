#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"
#include "spinlock.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"

struct mmap_area
{
  struct file *f;
  uint addr;
  int length;
  int prot;
  int flags;
  int fd;
  int offset;
  struct proc *p; // the process with this mmap_area

  int is_used;
};

struct
{
  struct spinlock lock;
  struct mmap_area areas[NMMAP];
} mtable;



extern char data[]; // defined by kernel.ld
pde_t *kpgdir;      // for use in scheduler()

void
minit(void)
{
  initlock(&mtable.lock, "mtable");
}

// Set up CPU's kernel segment descriptors.
// Run once on entry on each CPU.
void seginit(void)
{
  struct cpu *c;

  // Map "logical" addresses to virtual addresses using identity map.
  // Cannot share a CODE descriptor for both kernel and user
  // because it would have to have DPL_USR, but the CPU forbids
  // an interrupt from CPL=0 to DPL=3.
  c = &cpus[cpuid()];
  c->gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0, 0xffffffff, 0);
  c->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
  c->gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0, 0xffffffff, DPL_USER);
  c->gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
  lgdt(c->gdt, sizeof(c->gdt));
}

// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page table pages.
static pte_t *
walkpgdir(pde_t *pgdir, const void *va, int alloc)
{
  pde_t *pde;
  pte_t *pgtab;

  pde = &pgdir[PDX(va)];
  if (*pde & PTE_P)
  {
    pgtab = (pte_t *)P2V(PTE_ADDR(*pde));
  }
  else
  {
    if (!alloc || (pgtab = (pte_t *)kalloc()) == 0)
      return 0;
    // Make sure all those PTE_P bits are zero.
    memset(pgtab, 0, PGSIZE);
    // The permissions here are overly generous, but they can
    // be further restricted by the permissions in the page table
    // entries, if necessary.
    *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
  }
  return &pgtab[PTX(va)];
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.
static int
mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)
{
  char *a, *last;
  pte_t *pte;

  a = (char *)PGROUNDDOWN((uint)va);
  last = (char *)PGROUNDDOWN(((uint)va) + size - 1);
  for (;;)
  {
    if ((pte = walkpgdir(pgdir, a, 1)) == 0)
      return -1;
    if (*pte & PTE_P)
      panic("remap");
    *pte = pa | perm | PTE_P;
    if (a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}

// There is one page table per process, plus one that's used when
// a CPU is not running any process (kpgdir). The kernel uses the
// current process's page table during system calls and interrupts;
// page protection bits prevent user code from using the kernel's
// mappings.
//
// setupkvm() and exec() set up every page table like this:
//
//   0..KERNBASE: user memory (text+data+stack+heap), mapped to
//                phys memory allocated by the kernel
//   KERNBASE..KERNBASE+EXTMEM: mapped to 0..EXTMEM (for I/O space)
//   KERNBASE+EXTMEM..data: mapped to EXTMEM..V2P(data)
//                for the kernel's instructions and r/o data
//   data..KERNBASE+PHYSTOP: mapped to V2P(data)..PHYSTOP,
//                                  rw data + free physical memory
//   0xfe000000..0: mapped direct (devices such as ioapic)
//
// The kernel allocates physical memory for its heap and for user memory
// between V2P(end) and the end of physical memory (PHYSTOP)
// (directly addressable from end..P2V(PHYSTOP)).

// This table defines the kernel's mappings, which are present in
// every process's page table.
static struct kmap
{
  void *virt;
  uint phys_start;
  uint phys_end;
  int perm;
} kmap[] = {
    {(void *)KERNBASE, 0, EXTMEM, PTE_W},            // I/O space
    {(void *)KERNLINK, V2P(KERNLINK), V2P(data), 0}, // kern text+rodata
    {(void *)data, V2P(data), PHYSTOP, PTE_W},       // kern data+memory
    {(void *)DEVSPACE, DEVSPACE, 0, PTE_W},          // more devices
};

// Set up kernel part of a page table.
pde_t *
setupkvm(void)
{
  pde_t *pgdir;
  struct kmap *k;

  if ((pgdir = (pde_t *)kalloc()) == 0)
    return 0;
  memset(pgdir, 0, PGSIZE);
  if (P2V(PHYSTOP) > (void *)DEVSPACE)
    panic("PHYSTOP too high");
  for (k = kmap; k < &kmap[NELEM(kmap)]; k++)
    if (mappages(pgdir, k->virt, k->phys_end - k->phys_start,
                 (uint)k->phys_start, k->perm) < 0)
    {
      freevm(pgdir);
      return 0;
    }
  return pgdir;
}

// Allocate one page table for the machine for the kernel address
// space for scheduler processes.
void kvmalloc(void)
{
  kpgdir = setupkvm();
  switchkvm();
}

// Switch h/w page table register to the kernel-only page table,
// for when no process is running.
void switchkvm(void)
{
  lcr3(V2P(kpgdir)); // switch to the kernel page table
}

// Switch TSS and h/w page table to correspond to process p.
void switchuvm(struct proc *p)
{
  if (p == 0)
    panic("switchuvm: no process");
  if (p->kstack == 0)
    panic("switchuvm: no kstack");
  if (p->pgdir == 0)
    panic("switchuvm: no pgdir");

  pushcli();
  mycpu()->gdt[SEG_TSS] = SEG16(STS_T32A, &mycpu()->ts,
                                sizeof(mycpu()->ts) - 1, 0);
  mycpu()->gdt[SEG_TSS].s = 0;
  mycpu()->ts.ss0 = SEG_KDATA << 3;
  mycpu()->ts.esp0 = (uint)p->kstack + KSTACKSIZE;
  // setting IOPL=0 in eflags *and* iomb beyond the tss segment limit
  // forbids I/O instructions (e.g., inb and outb) from user space
  mycpu()->ts.iomb = (ushort)0xFFFF;
  ltr(SEG_TSS << 3);
  lcr3(V2P(p->pgdir)); // switch to process's address space
  popcli();
}

// Load the initcode into address 0 of pgdir.
// sz must be less than a page.
void inituvm(pde_t *pgdir, char *init, uint sz)
{
  char *mem;

  if (sz >= PGSIZE)
    panic("inituvm: more than a page");
  mem = kalloc();
  memset(mem, 0, PGSIZE);
  mappages(pgdir, 0, PGSIZE, V2P(mem), PTE_W | PTE_U);
  memmove(mem, init, sz);
}

// Load a program segment into pgdir.  addr must be page-aligned
// and the pages from addr to addr+sz must already be mapped.
int loaduvm(pde_t *pgdir, char *addr, struct inode *ip, uint offset, uint sz)
{
  uint i, pa, n;
  pte_t *pte;

  if ((uint)addr % PGSIZE != 0)
    panic("loaduvm: addr must be page aligned");
  for (i = 0; i < sz; i += PGSIZE)
  {
    if ((pte = walkpgdir(pgdir, addr + i, 0)) == 0)
      panic("loaduvm: address should exist");
    pa = PTE_ADDR(*pte);
    if (sz - i < PGSIZE)
      n = sz - i;
    else
      n = PGSIZE;
    if (readi(ip, P2V(pa), offset + i, n) != n)
      return -1;
  }
  return 0;
}

// Allocate page tables and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
int allocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
  char *mem;
  uint a;

  if (newsz >= KERNBASE)
    return 0;
  if (newsz < oldsz)
    return oldsz;

  a = PGROUNDUP(oldsz);
  for (; a < newsz; a += PGSIZE)
  {
    mem = kalloc();
    if (mem == 0)
    {
      cprintf("allocuvm out of memory\n");
      deallocuvm(pgdir, newsz, oldsz);
      return 0;
    }
    memset(mem, 0, PGSIZE);
    if (mappages(pgdir, (char *)a, PGSIZE, V2P(mem), PTE_W | PTE_U) < 0)
    {
      cprintf("allocuvm out of memory (2)\n");
      deallocuvm(pgdir, newsz, oldsz);
      kfree(mem);
      return 0;
    }
  }
  return newsz;
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
int deallocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
  pte_t *pte;
  uint a, pa;

  if (newsz >= oldsz)
    return oldsz;

  a = PGROUNDUP(newsz);
  for (; a < oldsz; a += PGSIZE)
  {
    pte = walkpgdir(pgdir, (char *)a, 0);
    if (!pte)
      a = PGADDR(PDX(a) + 1, 0, 0) - PGSIZE;
    else if ((*pte & PTE_P) != 0)
    {
      pa = PTE_ADDR(*pte);
      if (pa == 0)
        panic("kfree");
      char *v = P2V(pa);
      kfree(v);
      *pte = 0;
    }
  }
  return newsz;
}

// Free a page table and all the physical memory pages
// in the user part.
void freevm(pde_t *pgdir)
{
  uint i;

  if (pgdir == 0)
    panic("freevm: no pgdir");
  deallocuvm(pgdir, KERNBASE, 0);
  for (i = 0; i < NPDENTRIES; i++)
  {
    if (pgdir[i] & PTE_P)
    {
      char *v = P2V(PTE_ADDR(pgdir[i]));
      kfree(v);
    }
  }
  kfree((char *)pgdir);
}

// Clear PTE_U on a page. Used to create an inaccessible
// page beneath the user stack.
void clearpteu(pde_t *pgdir, char *uva)
{
  pte_t *pte;

  pte = walkpgdir(pgdir, uva, 0);
  if (pte == 0)
    panic("clearpteu");
  *pte &= ~PTE_U;
}

// Given a parent process's page table, create a copy
// of it for a child.
pde_t *
copyuvm(pde_t *pgdir, uint sz)
{
  pde_t *d;
  pte_t *pte;
  uint pa, i, flags;
  char *mem;

  if ((d = setupkvm()) == 0)
    return 0;
  for (i = 0; i < sz; i += PGSIZE)
  {
    if ((pte = walkpgdir(pgdir, (void *)i, 0)) == 0)
      panic("copyuvm: pte should exist");
    if (!(*pte & PTE_P))
      panic("copyuvm: page not present");
    pa = PTE_ADDR(*pte);
    flags = PTE_FLAGS(*pte);
    if ((mem = kalloc()) == 0)
      goto bad;
    memmove(mem, (char *)P2V(pa), PGSIZE);
    if (mappages(d, (void *)i, PGSIZE, V2P(mem), flags) < 0)
    {
      kfree(mem);
      goto bad;
    }
  }
  return d;

bad:
  freevm(d);
  return 0;
}

// PAGEBREAK!
//  Map user virtual address to kernel address.
char *
uva2ka(pde_t *pgdir, char *uva)
{
  pte_t *pte;

  pte = walkpgdir(pgdir, uva, 0);
  if ((*pte & PTE_P) == 0)
    return 0;
  if ((*pte & PTE_U) == 0)
    return 0;
  return (char *)P2V(PTE_ADDR(*pte));
}

// Copy len bytes from p to user address va in page table pgdir.
// Most useful when pgdir is not the current page table.
// uva2ka ensures this only works for PTE_U pages.
int copyout(pde_t *pgdir, uint va, void *p, uint len)
{
  char *buf, *pa0;
  uint n, va0;

  buf = (char *)p;
  while (len > 0)
  {
    va0 = (uint)PGROUNDDOWN(va);
    pa0 = uva2ka(pgdir, (char *)va0);
    if (pa0 == 0)
      return -1;
    n = PGSIZE - (va - va0);
    if (n > len)
      n = len;
    memmove(pa0 + (va - va0), buf, n);
    len -= n;
    buf += n;
    va = va0 + PGSIZE;
  }
  return 0;
}

// private
static int map_file(struct mmap_area *m)
{
  m->f = m->p->ofile[m->fd]; 
  if (m->f == 0)
    return -1;
  if ((m->f->readable != (m->prot & PROT_READ)) || (m->f->writable != (m->prot & PROT_WRITE)))
    return -1;

  m->f->off = m->offset;
  return 0;
}

// public
uint
mmap(uint addr, int length, int prot, int flags, int fd, int offset)
{
  if(NOT_INCLUDE(flags, MAP_ANONYMOUS) && fd == -1) return 0; // anonymous가 아닌데 fd가 -1

  struct mmap_area *m;

  acquire(&mtable.lock);

  for(m = mtable.areas; m < &mtable.areas[NMMAP]; m++)
    if(!m->is_used)
      goto found;

  release(&mtable.lock);
  return 0; // 없을 때.
  
found:
/**
 * TODO
 * - file 불러오기
 * - POPULATE옵션을 안 주면? 일단 PTE를 만들고 present bit를 0으로 준다?
 * - 
 * - POPULATE 안 줬을 때 page fault는 어디서 발생시킬 것인가?
 *   - page fault를 발생시키려면 page정보가 있어야된다...
 *   - 뇌피셜로는 walkpgdir에서 page valid가 꺼져있으면 page fault 발생시키는 게 맞지 않을까 싶은데..
*/
  m->is_used = 1;
  m->addr = addr;
  m->length = length;
  m->prot = prot;
  m->flags = flags;
  m->fd = fd;
  m->offset = offset;
  m->p = myproc();

  release(&mtable.lock);

  if (NOT_INCLUDE(m->flags, MAP_ANONYMOUS) && (map_file(m) == -1))
    goto bad;

  if (NOT_INCLUDE(m->flags, MAP_POPULATE))
    goto finish;

  pde_t *pgdir = m->p->pgdir;
  uint base_addr = m->addr + MMAPBASE;
  uint curr_addr;
  char *mem;
  for (curr_addr = base_addr; curr_addr < PGROUNDUP(base_addr); curr_addr += PGSIZE)
  {
    if ((mem = kalloc()) == 0)
      goto bad;
    if (flags & MAP_ANONYMOUS)
      memset(mem, 0, PGSIZE);
    else
      fileread(m->f, mem + (curr_addr - PGROUNDDOWN(curr_addr)), (PGROUNDUP(curr_addr) - curr_addr));
    if (mappages(pgdir, (void *)curr_addr, (PGROUNDUP(curr_addr) - curr_addr), V2P(mem), PTE_W | PTE_U) < 0)
    {
      kfree(mem);
      goto bad;
    }
  }
  for (curr_addr = PGROUNDDOWN(curr_addr); curr_addr < base_addr + length; curr_addr += PGSIZE)
  {
    if ((mem = kalloc()) == 0)
      goto bad;
    if (flags & MAP_ANONYMOUS)
      memset(mem, 0, PGSIZE);
    else
      fileread(m->f, mem, PGSIZE);
    if (mappages(pgdir, (void *)curr_addr, PGSIZE, V2P(mem), PTE_W | PTE_U) < 0)
    {
      kfree(mem);
      goto bad;
    }
  }

finish:
  return addr + MMAPBASE;

bad:
  // TODO: bad로직이 더 있을까? is_used??
  acquire(&mtable.lock);
  m->is_used = 0;
  release(&mtable.lock);
  return 0;
}

int
handle_page_fault()
{
  uint fault_addr = rcr2();

  struct mmap_area *m;

  acquire(&mtable.lock);
  
  for (m = mtable.areas; m < &mtable.areas[NMMAP]; m++)
  {
    if (!m->is_used)
      continue;
    uint base_addr = m->addr + MMAPBASE;
    if (base_addr <= fault_addr && fault_addr <= base_addr + m->length)
      goto found;
  }
  release(&mtable.lock);
  return -1;

found:  
  release(&mtable.lock);

  // If fault was write whilte mmap_area is write prohibited.
  if (((myproc()->tf->err) & (2)) && (!(m->prot&PROT_WRITE)))
    return -1;

  pde_t *pgdir = m->p->pgdir;
  uint base_addr = m->addr + MMAPBASE;
  char *mem;
  if ((mem = kalloc()) == 0)
    return -1;
  if (m->flags & MAP_ANONYMOUS)
    memset(mem, 0, PGSIZE);
  else
  {
    // TODO: fileread의 size가 file의 size보다 클 때?
    // 1. fault_addr가 start_addr와 같은 페이지 내에 존재.
    if (PGROUNDDOWN(fault_addr) == PGROUNDDOWN(base_addr))
    {
      m->f->off = m->offset;
      uint size = PGROUNDUP(base_addr) - base_addr;
      if (size == 0)
        size = PGSIZE;
      fileread(m->f, mem + (base_addr - PGROUNDDOWN(base_addr)), size);
    }
    // 2. fault_addr가 다른 페이지 내에 존재.
    else
    {
      m->f->off = m->offset + (PGROUNDDOWN(fault_addr) - base_addr);
      fileread(m->f, mem, PGSIZE);
    }
  }
  // TODO: writable condition나누기. mmap에도 해줘야됨.
  uint size = PGROUNDUP(fault_addr) - fault_addr;
  if (size == 0)
    size = PGSIZE;
  if (mappages(pgdir, (void *)fault_addr, size, V2P(mem), PTE_W | PTE_U) < 0)
  {
    kfree(mem);
    return -1;
  }

  return 0;
}

int
freemem(void)
{
  return kfreemem();
}

int
munmap(uint addr)
{
  struct mmap_area *m; 

  acquire(&mtable.lock);
  
  for (m = mtable.areas; m < &mtable.areas[NMMAP]; m++)
  {
    if (!m->is_used)
      continue;
    if (m->addr+MMAPBASE == addr)
      goto found;
  }

  release(&mtable.lock);
  return -1;

found:
  release(&mtable.lock);
  /**
   * 1. mmap_area가 존재한다면, 이거 remove 해줘야됨.
   * 2. physical page, page table이 할당 됐다면, deallocate해줘야됨.
   * 3. 만약 할당 되지 않았다면, mmap_area만 없애 주면 됨.
   * 
  */
  pte_t *pte;
  uint a, pa;

  uint base_addr = m->addr + MMAPBASE;
  a = PGROUNDDOWN(base_addr);
  for (; a < base_addr + m->length; a += PGSIZE) 
  {
    pte = walkpgdir(m->p->pgdir, (char *)a, 0);
    if (!pte)
      a = PGADDR(PDX(a) + 1, 0, 0) - PGSIZE;
    else if ((*pte & PTE_P) != 0)
    {
      pa = PTE_ADDR(*pte);
      if (pa == 0)
        panic("kfree");
      char *v = P2V(pa);
      kfree(v);
      *pte = 0;
    }
  }

  // TODO: page table 청소
  acquire(&mtable.lock);

  m->is_used = 0;
  m->f = 0;
  m->addr = 0;
  m->length = 0;
  m->prot = 0;
  m->flags = 0;
  m->fd = 0;
  m->offset = 0;
  m->p = 0;

  release(&mtable.lock);

  return 1;
}

// PAGEBREAK!
//  Blank page.
// PAGEBREAK!
//  Blank page.
// PAGEBREAK!
//  Blank page.
