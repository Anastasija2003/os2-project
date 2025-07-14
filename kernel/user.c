//
// Created by os on 12/17/24.
//
#include "defs.h"
#include "raid.h"
#include "fs.h"
#include "lock.h"
#include "param.h"
#include "proc.h"
#include <stdbool.h>
//uchar buf[BSIZE];
//uchar parity[BSIZE];
//uchar temp[BSIZE];

static int raid_lock_initialized = 0;

void ensure_raid_lock_initialized() {
  if (!raid_lock_initialized) {
    init_locks();
    initsleeplock(&raid_lock, "raid_lock");
    raid_lock_initialized = 1;
    printf("Inicijalizovano\n");
  }
}

void wait_disk(enum RAID_TYPE raid, int *diskn,int reader){
    switch(raid){
        case RAID0: wait0(diskn,reader); break;
        case RAID1: wait1(diskn,reader); break;
        case RAID4: wait4(diskn,reader); break;
        case RAID5: wait5(diskn,reader); break;
        case RAID0_1: wait0_1(diskn,reader); break;
    }
}

void signal_disk(enum RAID_TYPE raid,int diskn,int reader){
    switch(raid){
        case RAID0: signal0(diskn,reader); break;
        case RAID1: signal1(diskn,reader); break;
        case RAID4: signal4(diskn,reader); break;
        case RAID5: signal5(diskn,reader); break;
        case RAID0_1: signal0_1(diskn,reader); break;
    }
}

int init_raid(enum RAID_TYPE raid){ // NEMA PARALELIZACIJE
  read_block(1,0,(uchar*)&global_info_raid);
  ensure_raid_lock_initialized();	
  acquiresleep(&raid_lock);
  if(global_info_raid.initialized==1){
    printf("initialized already\n");
    if(global_info_raid.raid_type != raid) {
		  releasesleep(&raid_lock);
		  return -1;
	  }
	  releasesleep(&raid_lock);
    return 0;
  }
  if(DISKS%2 == 0 && raid == RAID0_1) return -1;
  if(DISKS==2 && raid == RAID4) return -1;
  if(DISKS<4 && raid == RAID5) return -1;
  global_info_raid.initialized = 1;
  global_info_raid.raid_type = raid;
  global_info_raid.broken = 0;
  global_info_raid.total_disks = DISKS-1;
  if(raid == RAID1) global_info_raid.total_disks = 1;
  else if(raid == RAID4) global_info_raid.total_disks--;
  else if(raid == RAID0_1) global_info_raid.total_disks/=2;
  global_info_raid.total_blocks = global_info_raid.total_disks*(DISK_SIZE/BSIZE)-1;
  if(raid == RAID5) global_info_raid.total_blocks-=DISK_SIZE/BSIZE;
  for(int i = 0;i<DISKS-1;i++){
    global_info_raid.disks[i] = 1;
  }
  write_block(1,0,(uchar*)&global_info_raid);
  releasesleep(&raid_lock);
  return 0;
}

int read_raid(int blkn, uchar *data){
  uchar temp[BSIZE], parity[BSIZE];
  uchar *buf = kalloc();
  if (!buf) return -1;
  ensure_raid_lock_initialized();	
  acquiresleep(&raid_lock);
  if(!global_info_raid.initialized || global_info_raid.broken==1) {
    releasesleep(&raid_lock);
    return -1;
  }
  if(blkn<0 || blkn>=global_info_raid.total_blocks) {
    releasesleep(&raid_lock);
    return -1;
  }
  enum RAID_TYPE raid = global_info_raid.raid_type;
  int total_disks = global_info_raid.total_disks;
  int disks[7];
  for(int i = 0;i<7;i++){
    disks[i] = global_info_raid.disks[i];
  }
  releasesleep(&raid_lock);
  blkn++;
  if(raid== RAID4 || raid == RAID5){
    int disk = blkn%total_disks;
    int block = blkn/total_disks;
    int parity_disk = DISKS;
    if(raid == RAID5){
      disk = blkn%(total_disks-1);
      block = blkn/(total_disks-1);
      parity_disk = DISKS-block%total_disks;
      if(disk>=parity_disk) disk++;
    }

    if(disks[disk]==0){
      memset(buf,0,BSIZE);
      for(int i = 0;i<total_disks;i++){
        if(i != disk){
          if(i == 0 && block == 0) continue;
          wait_disk(raid,&i,1);
          read_block(i+1,block,temp);
          signal_disk(raid,i,1);
          for(int j = 0;j<BSIZE;j++) buf[j] ^= temp[j];
        }
      }
      wait_disk(raid,&parity_disk,1);
      read_block(parity_disk,block,parity);
      signal_disk(raid,parity_disk,1);
      for(int i = 0;i<BSIZE;i++) buf[i] ^= parity[i];
    }else {
      wait_disk(raid,&disk,1);
      read_block(disk+1,block,buf);
      signal_disk(raid,disk,1);
    }
  }
  else if(raid == RAID1){
    for(int i = 0;i<DISKS-1;i++){
      if(disks[i]==1) {
        int disk = i;
        wait_disk(raid,&disk,1);
	      read_block(disk+1,blkn,buf);
        signal_disk(raid,disk,1);
	      break;
      }
    }
  }else{
    int disk = blkn%total_disks;
    int block = blkn/total_disks;
    wait_disk(raid,&disk,1);
    read_block(disk+1, block, buf);
    signal_disk(raid,disk,1);
  }

  struct proc *p = myproc();
  copyout(p->pagetable,(uint64)data,(char*)buf,BSIZE);
  kfree(buf);
  return 0;
}

int write_raid(int blkn, uchar *data){
  uchar temp[BSIZE], parity[BSIZE];
  uchar *buf = kalloc();
  if (!buf) return -1;
  ensure_raid_lock_initialized();	
  acquiresleep(&raid_lock);
  if(!global_info_raid.initialized || global_info_raid.broken==1) {
    releasesleep(&raid_lock);
	  return -2;
  }
  if(blkn<0 || blkn>=global_info_raid.total_blocks) {
    releasesleep(&raid_lock);
  	return -1;
  }
  enum RAID_TYPE raid = global_info_raid.raid_type;
  int total_disks = global_info_raid.total_disks;
  int disks[7];
  for(int i = 0;i<DISKS-1;i++){
    disks[i] = global_info_raid.disks[i];
  }
  releasesleep(&raid_lock);
  blkn++;
  struct proc *p = myproc();
  copyin(p->pagetable,(char*)buf,(uint64)data,BSIZE);
  if(raid == RAID4 || raid == RAID5){
    int disk = blkn%total_disks;
    int block = blkn/total_disks;
    int parity_disk = DISKS;
    if(raid == RAID5){
      disk = blkn%(total_disks-1);
      block = blkn/(total_disks-1);
      parity_disk = DISKS-block%total_disks;
      if(disk>=parity_disk) disk++;
    }
    wait_disk(raid,&disk,1);
    read_block(disk+1,block,temp);
    signal_disk(raid,disk,1);
    for(int i = 0;i<BSIZE;i++) parity[i] ^= buf[i]^temp[i];
    wait_disk(raid,&disk,0);
    write_block(disk+1,block,buf);
    write_block(parity_disk,block,parity);
    signal_disk(raid,disk,0);
  }else if(raid == RAID1){
    int dummy = 2;
    wait_disk(raid, &dummy, 0);
    for(int i = 1; i < DISKS; i++) {
      if(disks[i-1]==1) write_block(i, blkn, buf);
    }
    signal_disk(raid, dummy, 0);
  }else{
    int disk = blkn%total_disks;
    int block = blkn/total_disks;
    if(raid == RAID0){
      wait_disk(raid,&disk,0);
      write_block(disk+1, block, buf);
      signal_disk(raid,disk,0);
    }else{
      int backup = total_disks+disk;
      wait_disk(raid,&disk,0);
      if(disks[disk] == 1) write_block(disk+1, block, buf);
      if(disks[backup] == 1) write_block(backup+1, block, buf);
      signal_disk(raid,disk,0);
    }
  }
  kfree(buf);
  return 0;
}

int disk_fail_raid(int diskn){
  ensure_raid_lock_initialized();
  acquiresleep(&raid_lock);
  if(diskn<1 || diskn>DISKS || global_info_raid.initialized == 0 || global_info_raid.broken==1) {
      releasesleep(&raid_lock);
      return -1;
  }
  if(global_info_raid.disks[diskn-1] == 0){
     releasesleep(&raid_lock);
     return 0;
  }
  global_info_raid.disks[diskn-1] = 0;
  disk_flag(diskn-1,0);
  if(global_info_raid.raid_type == RAID0) global_info_raid.broken = 1;
  else if(global_info_raid.raid_type == RAID4 || global_info_raid.raid_type == RAID5 || global_info_raid.raid_type == RAID0_1){
    if(global_info_raid.broken==2) global_info_raid.broken = 1;
    else global_info_raid.broken = 2;
  }
  else if(global_info_raid.raid_type == RAID1){
    bool broken = true;
    for(int i = 0;i<DISKS-1;i++) {
      if(global_info_raid.disks[i] == 1) {
          broken = false;
          break;
      }
    }
    if(broken) global_info_raid.broken = 1;
  }
  int d = 0;
  wait_disk(global_info_raid.raid_type,&d,0);
  write_block(1,0,(uchar*)&global_info_raid);
  signal_disk(global_info_raid.raid_type,d,0);
  releasesleep(&raid_lock);
  return 0;
}

int disk_repaired_raid(int diskn){
  uchar temp[BSIZE], parity[BSIZE];
  uchar *buf = kalloc();
  if(!buf) return -1;
  ensure_raid_lock_initialized();	
  acquiresleep(&raid_lock);
  if(diskn<1 || diskn>global_info_raid.total_disks || global_info_raid.initialized == 0 || global_info_raid.broken==1) {
    releasesleep(&raid_lock);
	  return -1;
  }
  if(global_info_raid.disks[diskn-1] != 0) {
    releasesleep(&raid_lock);
	  return 0;
  }
  enum RAID_TYPE raid = global_info_raid.raid_type;
  int total_disks = global_info_raid.total_disks;
  int disks[7];
  for(int i = 0;i<7;i++){
    disks[i] = global_info_raid.disks[i];
  }
  int i = 0;
  if(diskn==1) i++;
  int disk = diskn-1;
  wait_disk(raid,&disk,0);
  while(i<DISK_SIZE/BSIZE){
    if(raid == RAID4 || raid == RAID5){
      memset(buf,0,BSIZE);
      int parity_disk = DISKS-i%total_disks;
      for(int j = 0;j<total_disks;j++){
        if(j != diskn-1){
          if(i==0 && j==0) continue;
          if(raid==RAID5 && i ==parity_disk) continue;
          read_block(j+1,i,temp);
          for(int k = 0;k<BSIZE;k++) buf[k] ^= temp[k];
        }
      }
      if(raid == RAID4) read_block(DISKS,i,parity);
      else read_block(parity_disk,i,parity);
      for(int k = 0;k<BSIZE;k++) buf[k] ^= parity[k];
    }
    else if(raid == RAID1){
      for(int j = 0;j<DISKS-1;j++) {
        if(disks[j] == 1){
          read_block(j+1,i,buf);
          break;
        }
      }
    }
    else {
		    read_block(diskn+total_disks,i,buf);
    }
    write_block(diskn,i,buf);
    i++;
  }
  disk_flag(diskn-1,1);
  signal_disk(raid,disk,0);
  global_info_raid.disks[diskn-1] = 1;
  global_info_raid.broken = 0;
  disk = 0;
  wait_disk(raid,&disk,0);
  write_block(1,0,(uchar*)&global_info_raid);
  signal_disk(raid,disk,0);
  kfree(buf);
  releasesleep(&raid_lock);
  return 0;
}

int info_raid(uint* blkn,uint *blks,uint*diskn){
  ensure_raid_lock_initialized();	
  acquiresleep(&raid_lock);
  struct proc *p = myproc();
  if(!global_info_raid.initialized || global_info_raid.broken==1) {
    releasesleep(&raid_lock);
    return -1;
  }
  uint64 b = BSIZE;
  copyout(p->pagetable,(uint64)blkn,(char*)&global_info_raid.total_blocks,sizeof(uint));
  copyout(p->pagetable,(uint64)blks,(char*)&b,sizeof(uint));
  copyout(p->pagetable,(uint64)diskn,(char*)&global_info_raid.total_disks,sizeof(uint));
  releasesleep(&raid_lock);
  return 0;
}

int destroy_raid(){ 
  ensure_raid_lock_initialized();	
  acquiresleep(&raid_lock);
  if(global_info_raid.initialized == 0) {
    releasesleep(&raid_lock);
    return -1;
  }
  global_info_raid.initialized = 0;
  raid_lock_initialized = 0;
  int disk = 0;
  wait_disk(global_info_raid.raid_type,&disk,0);
  write_block(1,0,(uchar*)&global_info_raid);
  signal_disk(global_info_raid.raid_type,disk,0);
  releasesleep(&raid_lock);
  return 0;
}