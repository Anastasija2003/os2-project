#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
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

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
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

  argint(0, &pid);
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

uint64
sys_init_raid(void){
  int type;
  argint(0,&type);
  return init_raid(type);
}

uint64
sys_read_raid(void){
  int blkn;
  uint64 buf;
  argint(0,&blkn);
  argaddr(1,&buf);
  return read_raid(blkn,(uchar*)buf);
}

uint64
sys_write_raid(void){
  int blkn;
  uint64 buf;
  argint(0,&blkn);
  argaddr(1,&buf);
  return write_raid(blkn,(uchar*)buf);
}

uint64
sys_disk_fail_raid(void){
  int disk;
  argint(0,&disk);
  return disk_fail_raid(disk);
}

uint64
sys_disk_repaired_raid(void){
  int disk;
  argint(0,&disk);
  return disk_repaired_raid(disk);
}

uint64
sys_info_raid(void){
  uint64 blkn;
  uint64 blks;
  uint64 diskn;
  argaddr(0,&blkn);
  argaddr(1,&blks);
  argaddr(2,&diskn);
  return info_raid((uint*)blkn,(uint*)blks,(uint*)diskn);
}

uint64
sys_destroy_raid(void){
  return destroy_raid();
}