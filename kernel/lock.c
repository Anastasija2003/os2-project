//
// Created by parallels on 7/11/25.
//
#include "defs.h"
#include "lock.h"

struct lock_info lock;
struct spinlock raid_lock_p;

void init_locks(){
    initlock(&raid_lock_p, "global lock");
    for(int i = 0; i < DISKS - 1; i++){
        lock.works[i] = 1;
        lock.busy[i] = 0;
        initsleeplock(&lock.locks[i], "disk lock");
    }
}

void disk_flag(int diskn, int flag){
    acquire(&raid_lock_p);
    lock.works[diskn] = flag;
    lock.busy[diskn] = 0;
    release(&raid_lock_p);
}

void wait0(int *diskn, int reader){
    acquiresleep(&lock.locks[*diskn]);
}

void wait1(int *diskn, int reader) {
    if (reader == 1) {
        while (1) {
            for (int i = 0; i < DISKS - 1; i++) {
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
        while (1) {
            int all_free = 1;
            acquire(&raid_lock_p);
            for (int i = 0; i < DISKS - 1; i++) {
                if (lock.busy[i] == 1) {
                    all_free = 0;
                    break;
                }
            }
            if (all_free) {
                for (int i = 0; i < DISKS - 1; i++) {
                    if(lock.works[i]) lock.busy[i] = 1;
                }
                release(&raid_lock_p);
                break;
            }
            release(&raid_lock_p);
        }
        for (int i = 0; i < DISKS - 1; i++) {
            acquiresleep(&lock.locks[i]);
        }
    }
}

void wait4(int *diskn, int reader) {}
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
            if (lock.busy[*diskn] == 1 || lock.busy[backup_disk]==1) {
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
        for (int i = 0; i < DISKS - 1; i++) {
            lock.busy[i] = 0;
        }
        release(&raid_lock_p);
        for (int i = 0; i < DISKS - 1; i++) {
            releasesleep(&lock.locks[i]);
        }
    }
}

void signal4(int diskn, int reader) {}
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
