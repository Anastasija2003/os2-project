//
// Created by os on 12/17/24.
//
#include "raid.h"
#include "fs.h"
#include "spinlock.h"
#include "param.h"
#include "proc.h"

uchar buf[BSIZE];
uchar parity[BSIZE];
uchar temp[BSIZE];

int init_raid(enum RAID_TYPE raid){ //uradjeno
  read_block(1,0,(uchar*)&global_info_raid);

  if(global_info_raid.initialized==1){
    printf("initialized already\n");
    if(global_info_raid.raid_type != raid) return -1;
    return 0;
  }
  if(DISKS%2 == 0 && (raid == RAID1 || raid == RAID0_1)) return -1;
  if(DISKS==2 && raid == RAID4) return -1;
  if(DISKS<4 && raid == RAID5) return -1;
  global_info_raid.initialized = 1;
  global_info_raid.raid_type = raid;
  global_info_raid.broken = 0;
  global_info_raid.total_disks = DISKS-1;
  if(raid==RAID4) global_info_raid.total_disks--;
  else if(global_info_raid.raid_type !=RAID0) global_info_raid.total_disks/=2;
  global_info_raid.total_blocks = global_info_raid.total_disks*(DISK_SIZE/BSIZE)-1;
  if(raid == RAID5) global_info_raid.total_blocks-=DISK_SIZE/BSIZE;
  for(int i = 0;i<DISKS-1;i++){
    global_info_raid.disks[i] = 1;
  }
  write_block(1,0,(uchar*)&global_info_raid);
  return 0;
}

int read_raid(int blkn, uchar *data){
  read_block(1,0,(uchar*)&global_info_raid);
  if(!global_info_raid.initialized || global_info_raid.broken==1) return -1;
  if(blkn<0 || blkn>=global_info_raid.total_blocks) return -1;
  blkn++;
  if(global_info_raid.raid_type == RAID4 || global_info_raid.raid_type == RAID5){
    int disk = blkn%global_info_raid.total_disks;
    int block = blkn/global_info_raid.total_disks;
    int parity_disk = DISKS;
    if(global_info_raid.raid_type == RAID5){
      disk = blkn%(global_info_raid.total_disks-1);
      block = blkn/(global_info_raid.total_disks-1);
      parity_disk = DISKS-block%global_info_raid.total_disks;
      if(disk>=parity_disk) disk++;
    }

    if(global_info_raid.disks[disk]==0){
      memset(buf,0,BSIZE);
      for(int i = 0;i<global_info_raid.total_disks;i++){
        if(i != disk){
          if(i == 0 && block == 0) continue;
          if(i == parity_disk) continue;
          read_block(i+1,block,temp);
          for(int j = 0;j<BSIZE;j++) buf[j] ^= temp[j];
        }
      }
      read_block(parity_disk,block,parity);
      for(int i = 0;i<BSIZE;i++) buf[i] ^= parity[i];
    }else read_block(disk+1,block,buf);
  }
  else if(global_info_raid.raid_type == RAID1){
    int num = DISK_SIZE/BSIZE;
    int disk = blkn/num;
    int block = blkn%num;
    if(global_info_raid.disks[disk] != 1){
      read_block(global_info_raid.total_disks + disk+1, block, buf);
    }else{
      read_block(disk+1, block, buf);
    }
  }else{
     int disk = blkn%global_info_raid.total_disks;
     int block = blkn/global_info_raid.total_disks;
     if(global_info_raid.disks[disk] == 0 && global_info_raid.raid_type == RAID0_1){
       read_block(global_info_raid.total_disks + disk+1, block, buf);
     }else{
       read_block(disk+1, block, buf);
     }
  }
  struct proc *p = myproc();
  copyout(p->pagetable,(uint64)data,(char*)buf,BSIZE);
  return 0;
}

int write_raid(int blkn, uchar *data){
  read_block(1,0,(uchar*)&global_info_raid);
  if(!global_info_raid.initialized || global_info_raid.broken==1) return -1;
  if(blkn<0 || blkn>=global_info_raid.total_blocks) return -1;
  blkn++;
  struct proc *p = myproc();
  copyin(p->pagetable,(char*)buf,(uint64)data,BSIZE);
  if(global_info_raid.raid_type == RAID4 || global_info_raid.raid_type == RAID5){
    int disk = blkn%global_info_raid.total_disks;
    int block = blkn/global_info_raid.total_disks;
    int parity_disk = DISKS;
    if(global_info_raid.raid_type == RAID5){
      disk = blkn%(global_info_raid.total_disks-1);
      block = blkn/(global_info_raid.total_disks-1);
      parity_disk = DISKS-block%global_info_raid.total_disks;
      if(disk>=parity_disk) disk++;
    }

    read_block(parity_disk,block,parity);

    read_block(disk+1,block,temp);
    for(int i = 0;i<BSIZE;i++) parity[i] ^= buf[i]^temp[i];
    write_block(disk+1,block,buf);

	write_block(parity_disk,block,parity);
  }else if(global_info_raid.raid_type == RAID1){
    int num = DISK_SIZE/BSIZE;
    int disk = blkn/num;
    int block = blkn%num;
    write_block(global_info_raid.total_disks + disk+1, block, buf);
    if(global_info_raid.disks[disk]) write_block(disk+1, block, buf);
  }else{
    int disk = blkn%global_info_raid.total_disks;
    int block = blkn/global_info_raid.total_disks;
    if(global_info_raid.raid_type == RAID0_1){
      write_block(global_info_raid.total_disks +disk+1, block, buf);
    }
    if(global_info_raid.disks[disk]) write_block(disk+1, block, buf);
  }
  return 0;
}
int disk_fail_raid(int diskn){ //uradjeno
  read_block(1,0,(uchar*)&global_info_raid);
  if(diskn<1 || diskn>DISKS || global_info_raid.initialized == 0 || global_info_raid.broken==1) return -1;
  global_info_raid.disks[diskn-1] = 0;
  if(global_info_raid.raid_type == RAID0) global_info_raid.broken = 1;
  else if(global_info_raid.raid_type == RAID4 || global_info_raid.raid_type == RAID5){
    if(global_info_raid.broken==2) global_info_raid.broken = 1;
    else global_info_raid.broken = 2;
  }
  write_block(1,0,(uchar*)&global_info_raid);
  return 0;
}

int disk_repaired_raid(int diskn){
  read_block(1,0,(uchar*)&global_info_raid);
  if(diskn<1 || diskn>global_info_raid.total_disks || global_info_raid.initialized == 0 || global_info_raid.broken==1) return -1;
  if(global_info_raid.disks[diskn-1] != 0) return 0;
  global_info_raid.disks[diskn-1] = 1;
  if(global_info_raid.broken==2) global_info_raid.broken = 0;
  int i = 0;
  if(diskn==1) i++;
  while(i<DISK_SIZE/BSIZE){
    if(global_info_raid.raid_type == RAID4 || global_info_raid.raid_type == RAID5){
      memset(buf,0,BSIZE);
      int parity_disk = DISKS-i%global_info_raid.total_disks;
      for(int j = 0;j<global_info_raid.total_disks;j++){
        if(j != diskn-1){
          if(i==0 && j==0) continue;
          if(global_info_raid.raid_type==RAID5 && i ==parity_disk) continue;
          read_block(j+1,i,temp);
          for(int k = 0;k<BSIZE;k++) buf[k] ^= temp[k];
        }
      }
      if(global_info_raid.raid_type == RAID4) read_block(DISKS,i,parity);
      else read_block(parity_disk,i,parity);
      for(int k = 0;k<BSIZE;k++) buf[k] ^= parity[k];
    }
    else read_block(diskn+global_info_raid.total_disks,i,buf);
    write_block(diskn,i,buf);
    i++;
  }
  write_block(1,0,(uchar*)&global_info_raid);
  return 0;
}

int info_raid(uint* blkn,uint *blks,uint*diskn){  //uradjeno
  struct proc *p = myproc();

  read_block(1,0,(uchar*)&global_info_raid);
  if(!global_info_raid.initialized || global_info_raid.broken==1) return -1;

  uint64 b = BSIZE;
  copyout(p->pagetable,(uint64)blkn,(char*)&global_info_raid.total_blocks,sizeof(uint));
  copyout(p->pagetable,(uint64)blks,(char*)&b,sizeof(uint));
  copyout(p->pagetable,(uint64)diskn,(char*)&global_info_raid.total_disks,sizeof(uint));

  return 0;
}

int destroy_raid(){    //uradjeno

  read_block(1,0,(uchar*)&global_info_raid);
  if(global_info_raid.initialized == 0) return -1;
  global_info_raid.initialized = 0;

  write_block(1,0,(uchar*)&global_info_raid);
  return 0;
}