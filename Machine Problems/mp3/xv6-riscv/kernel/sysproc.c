#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
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

uint64
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

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


// for mp3
uint64
sys_thrdstop(void)
{
  int delay, thrdstop_context_id;
  uint64 handler, handler_arg;
  if (argint(0, &delay) < 0)
    return -1;
  if (argint(1, &thrdstop_context_id) < 0)
    return -1;
  if (argaddr(2, &handler) < 0)
    return -1;
  if (argaddr(3, &handler_arg) < 0)
    return -1;

  // store the context
  struct proc *p = myproc();
  if (thrdstop_context_id < 0)
  {
    // assign a new context id
    thrdstop_context_id = p->free_context_index;
    p->free_context_index++;
  }

  // printf("[%d] sys_thrdstop thrdstop_context_id=%d delay=%d\n", ticks, thrdstop_context_id, delay);

  // save the thrdstop_context_id
  p->delay = delay;
  p->consumed = 0;
  p->stop_context_id = thrdstop_context_id;
  p->stop_handler = handler;
  p->stop_handler_arg = handler_arg;
  p->store_only = 0;

  // set the bit as used
  p->context_id_bit_vector |= 1 << thrdstop_context_id;
  // clear backup bit
  // p->context_backup_bit_vector &= ~(1 << thrdstop_context_id);

  return thrdstop_context_id;
}

// for mp3
uint64
sys_cancelthrdstop(void)
{
  int thrdstop_context_id, is_exit;
  if (argint(0, &thrdstop_context_id) < 0)
    return -1;
  if (argint(1, &is_exit) < 0)
    return -1;

  struct proc *p = myproc();
  int ret = p->consumed;
  if (is_exit == 0 && thrdstop_context_id >= 0)
  {
    // store the context
    // printf("store the context now\n");
    p->consumed = 0;
    p->delay = 0;
    p->store_only = 1;
  }
  else if (is_exit > 0)
  {
    // clear the bit
    p->context_id_bit_vector &= ~(1 << thrdstop_context_id);
    p->context_backup_bit_vector &= ~(1 << thrdstop_context_id);
    p->stop_handler = 0;
    p->consumed = 0;
    p->delay = 0;
  }

  // printf("[%d] sys_cancelthrdstop thrdstop_context_id=%d is_exit=%d ret=%d\n", ticks, thrdstop_context_id, is_exit, ret);
  return ret;
}

// for mp3
uint64
sys_thrdresume(void)
{
  int  thrdstop_context_id;
  if (argint(0, &thrdstop_context_id) < 0)
    return -1;

  // printf("[%d] sys_thrdresume: thrdstop_context_id=%d\n", ticks, thrdstop_context_id);

  struct proc *p = myproc();
  // check if the thrdstop_context_id is valid
  if ((p->context_id_bit_vector & (1 << thrdstop_context_id)) == 0)
  {
    // printf("sys_thrdresume: thrdstop_context_id=%d is invalid\n", thrdstop_context_id);
    return -1;
  }

  // check whether the thrdstop_context_id is backup
  if ((p->context_backup_bit_vector & (1 << thrdstop_context_id)) == 0)
  {
    // printf("sys_thrdresume: thrdstop_context_id=%d is not backup\n", thrdstop_context_id);
    return -1;
  }
  
  // switch context back
  p->need_restore_context = 1;
  p->context_id_to_restore = thrdstop_context_id;

  // p->stop_handler = 0;
  // p->consumed = 0;

  return 0;
}
