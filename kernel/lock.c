//
// Created by parallels on 7/11/25.
//
#include "defs.h"
#include "lock.h"
struct lock_info lock;
struct spinlock raid_lock_p;
void init_locks(){
    initlock(&raid_lock_p, "global lock");
    for(int i =  0;i<DISKS-1;i++){
        lock.works[i] = 1;
        lock.busy[i] = 0;
        initsleeplock(&lock.locks[i],"disk lock");
    }
}

void disk_flag(int diskn, int flag){
    acquire(&raid_lock_p);
    lock.works[diskn] = flag;
    //printf("Disk %d set to %d\n", diskn, flag);
    release(&raid_lock_p);
}
void wait0(int *diskn,int reader){
    acquiresleep(&lock.locks[*diskn]);
}

void wait1(int *diskn, int reader) {
    if (reader == 1) {
        while (1) {
            for (int i = 0; i < DISKS - 1; i++) {
                acquire(&raid_lock_p);
                if (lock.busy[i] == 0 && lock.works[i]==1) {
                    lock.busy[i] = 1;
                    *diskn = i;
                    //printf("Got disk number: %d\n",i);
                    acquiresleep(&lock.locks[i]);
                    release(&raid_lock_p);
                    return;
                }
                release(&raid_lock_p);
            }
        }
    } else {
        while (1) {
            int all_free = 1;
            acquire(&raid_lock_p);
            for (int i = 0; i < DISKS - 1; i++) {
                if (lock.busy[i] == 1 && lock.works[i]==1) {
                    all_free = 0;
                    break;
                }
            }
            if (all_free) {
                for (int i = 0; i < DISKS - 1; i++) {
                     lock.busy[i] = 1;
                }
                break;
            }
            release(&raid_lock_p);
        }
        for (int i = 0; i < DISKS - 1; i++) {
            acquiresleep(&lock.locks[i]);
        }
        release(&raid_lock_p);
    }
}

void wait4(int *diskn,int reader){}
void wait5(int *diskn,int reader){}
void wait0_1(int *diskn,int reader){
    
}

void signal0(int diskn,int reader){
    releasesleep(&lock.locks[diskn]);
}
void signal1(int diskn, int reader) {
    if (reader == 1) {
        acquire(&raid_lock_p);
        lock.busy[diskn] = 0;
        releasesleep(&lock.locks[diskn]);
        release(&raid_lock_p);
    } else {
        acquire(&raid_lock_p);
        for (int i = 0; i < DISKS - 1; i++) {
            lock.busy[i] = 0;
        }
        for (int i = 0; i < DISKS - 1; i++) {
            releasesleep(&lock.locks[i]);
        }
        release(&raid_lock_p);
    }
}

void signal4(int diskn,int reader){}
void signal5(int diskn,int reader){}
void signal0_1(int diskn,int reader){}
