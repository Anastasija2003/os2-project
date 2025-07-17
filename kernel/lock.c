//
// Created by parallels on 7/11/25.
//
#include "defs.h"
#include "lock.h"

// Fallback definition if DISKS is not defined by compiler
#ifndef DISKS
#define DISKS 6
#endif

struct lock_info lock;
struct spinlock raid_lock_p;

void init_locks(){
    initlock(&raid_lock_p, "global lock");
    for(int i = 0; i < DISKS; i++){
        lock.works[i] = 1;
        lock.busy[i] = 0;
        initsleeplock(&lock.locks[i], "disk lock");
    }
}

void require_all(){
    while (1) {
        int all_free = 1;
        acquire(&raid_lock_p);
        for (int i = 0; i < DISKS; i++) {
            if (lock.works[i]==1 && lock.busy[i] == 1) {
                all_free = 0;
                break;
            }
        }
        if (all_free) {
            for (int i = 0; i < DISKS ; i++) {
                if(lock.works[i]) lock.busy[i] = 1;
            }
            release(&raid_lock_p);
            break;
        }
        release(&raid_lock_p);
    }
    for (int i = 0; i < DISKS; i++) {
        if(lock.works[i]) acquiresleep(&lock.locks[i]);
    }        
}


void disk_flag(int diskn, int flag){
    acquire(&raid_lock_p);
    lock.works[diskn] = flag;
    if(flag==0) lock.busy[diskn] = 1;
    else lock.busy[diskn] = 0;
    release(&raid_lock_p);
}

void wait0(int *diskn, int reader){
    acquiresleep(&lock.locks[*diskn]);
}

void wait1(int *diskn, int reader) {
    if (reader == 1) {
        while (1) {
            for (int i = 0; i < DISKS; i++) {
                acquire(&raid_lock_p);
                if (lock.busy[i] == 0 && lock.works[i] == 1) {
                    lock.busy[i] = 1;
                    *diskn = i;
                    release(&raid_lock_p);
                    acquiresleep(&lock.locks[i]);
                    return;
                }
                release(&raid_lock_p);
            }
        }
    } else {
        require_all();
    }
}

void wait4(int *diskn, int reader) {
    if(reader){
        acquire(&raid_lock_p);
        if(lock.works[*diskn] == 1){
            while(1){
                while(lock.busy[*diskn]==1){
                    release(&raid_lock_p);
                    acquire(&raid_lock_p);
                }
                if(lock.busy[*diskn]==0){
                    lock.busy[*diskn] = 1;
                    break;
                }
            }
            release(&raid_lock_p);
            acquiresleep(&lock.locks[*diskn]);
            return;
        }
        release(&raid_lock_p);
        *diskn = -1;
        require_all();
    }else{
        acquire(&raid_lock_p);
        while(1){
            if(lock.works[*diskn] == 0 || lock.works[DISKS-1] == 0){
                release(&raid_lock_p);
                *diskn = -1;
                require_all();
                return;
            }
            
            if(lock.busy[*diskn] == 0 && lock.busy[DISKS-1] == 0){
                lock.busy[*diskn] = 1;
                lock.busy[DISKS-1] = 1;
                break;
            }
            
            release(&raid_lock_p);

            acquire(&raid_lock_p);
        }

        if(lock.works[*diskn] == 0 || lock.works[DISKS-1] == 0){
            lock.busy[*diskn] = 0;
            lock.busy[DISKS-1] = 0;
            release(&raid_lock_p);
            *diskn = -1;
            require_all();
            return;
        }
        release(&raid_lock_p);
        
        acquiresleep(&lock.locks[*diskn]);
        acquiresleep(&lock.locks[DISKS-1]);
        return;
    }
}

void wait5(int *diskn, int reader) {}
void wait0_1(int *diskn, int reader) {
    int backup_disk = (*diskn) + DISKS/2;
    if (reader == 1) {
        while (1) {
            acquire(&raid_lock_p);
            if (lock.busy[*diskn] == 0 && lock.works[*diskn] == 1) {
                lock.busy[*diskn] = 1;
                release(&raid_lock_p);
                acquiresleep(&lock.locks[*diskn]);
                return;
            }
            release(&raid_lock_p);
            acquire(&raid_lock_p);
            if(lock.busy[backup_disk]==0 && lock.works[backup_disk]==1){
                lock.busy[backup_disk] = 1;
                *diskn = backup_disk;
                release(&raid_lock_p);
                acquiresleep(&lock.locks[backup_disk]);
                return;
            }
            release(&raid_lock_p);
        }
    } else {
        while (1) {
            int all_free = 1;
            acquire(&raid_lock_p);
            if (lock.works[*diskn]==1 && lock.busy[*diskn] == 1) {
                all_free = 0;
            } 
            if (lock.works[backup_disk]==1 && lock.busy[backup_disk] == 1) {
                all_free = 0;
            }
            if (all_free) {
                if(lock.works[*diskn]) lock.busy[*diskn] = 1;
                if(lock.works[backup_disk]) lock.busy[backup_disk] = 1;
                release(&raid_lock_p);
                break;
            }
            release(&raid_lock_p);
        }
        acquiresleep(&lock.locks[*diskn]);
        acquiresleep(&lock.locks[backup_disk]);
    }
}

void signal0(int diskn, int reader){
    releasesleep(&lock.locks[diskn]);
}

void signal1(int diskn, int reader) {
    if (reader == 1) {
        acquire(&raid_lock_p);
        lock.busy[diskn] = 0;
        release(&raid_lock_p);
        releasesleep(&lock.locks[diskn]);
    } else {
        acquire(&raid_lock_p);
        for (int i = 0; i < DISKS; i++) {
            lock.busy[i] = 0;
        }
        release(&raid_lock_p);
        for (int i = 0; i < DISKS ; i++) {
            releasesleep(&lock.locks[i]);
        }
    }
}

void signal4(int diskn, int reader) {
    if(diskn == -1){
        acquire(&raid_lock_p);
        for (int i = 0; i < DISKS ; i++) {
            if(lock.works[i]) lock.busy[i] = 0;
        }
        release(&raid_lock_p);
        for (int i = 0; i < DISKS; i++) {
            if(lock.works[i]) releasesleep(&lock.locks[i]);
        }
    }else{
        acquire(&raid_lock_p);
        lock.busy[diskn] = 0;
        if(reader==0) lock.busy[DISKS-1] = 0;
        release(&raid_lock_p);
        releasesleep(&lock.locks[diskn]);
        if(reader==0) releasesleep(&lock.locks[DISKS-1]);
    }
}
void signal5(int diskn, int reader) {}
void signal0_1(int diskn, int reader) {
    if (reader == 1) {
        acquire(&raid_lock_p);
        lock.busy[diskn] = 0;
        release(&raid_lock_p);
        releasesleep(&lock.locks[diskn]);
    } else {
        acquire(&raid_lock_p);
        int backup_disk = diskn + DISKS/2;
        lock.busy[diskn] = 0;
        lock.busy[backup_disk] = 0;
        release(&raid_lock_p);
        releasesleep(&lock.locks[diskn]);
        releasesleep(&lock.locks[backup_disk]);
    }
}
