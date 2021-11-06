#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

int sys_fork(void)
{
  return fork();
}

int sys_exit(void)
{
  exit();
  return 0; // not reached
}

int sys_wait(void)
{
  return wait();
}

int sys_kill(void)
{
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int sys_getpid(void)
{
  return myproc()->pid;
}

int sys_sbrk(void)
{
  int addr;
  int n;

  if (argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

int sys_sleep(void)
{
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (myproc()->killed)
    {
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
int sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_halt(void)
{
  outw(0x604, 0x2000);
  return 0;
}

int sys_pinfo(void)
{
  int *size;
  struct proc *p;
  struct proc_info *procs;
  struct proc_info *temp;
  unsigned short i = 0;
  unsigned short j = 0;
  short sz = 0, pid = 0;

  if (argptr(0, (void *)&procs, sizeof(struct proc_info)) < 0 ||
      argptr(1, (void *)&size, sizeof(int)) < 0)
    return -1;

  struct ptable *ptab = proc_table();
  p = ptab->proc;
  *size = 0;
  temp = procs;

  acquire(&ptab->lock);

  for (i = 0; i < NPROC; ++i, ++p)
  {
    if (p->state != RUNNING && p->state != RUNNABLE)
      continue;

    temp->sz = p->sz;
    temp->pid = p->pid;
    ++temp;
    (*size)++;
    if (i == 0)
      continue;

    // bring the smaller size to the begin
    for (j = *size - 1; j >= 1; --j)
    {
      if (procs[j].sz >= procs[j - 1].sz)
        continue;

      // swap
      sz = procs[j - 1].sz;
      pid = procs[j - 1].pid;
      procs[j - 1].sz = 100;
      procs[j - 1].pid = procs[j].pid;
      procs[j].sz = sz;
      procs[j].pid = pid;
    }
  }

  release(&ptab->lock);

  return 0;
}
