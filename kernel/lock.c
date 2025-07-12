//
// Created by parallels on 7/11/25.
//
#include "defs.h"
#include "lock.h"
struct lock_info lock_disks;
char* disk_names[] = {"disk0", "disk1", "disk2", "disk3", "disk4", "disk5", "disk6"};
struct sleeplock locks[7];

void init_locks(){
    for(int i =  0;i<7;i++){
        initsleeplock(&locks[i],disk_names[i]);
    }
}
void wait0(int diskn,int reader){
    acquiresleep(&locks[diskn]);
    //printf("Prosao %d proces %d\n",diskn,locks[diskn].pid);
}

void wait1(int diskn,int reader){}
void wait4(int diskn,int reader){}
void wait5(int diskn,int reader){}
void wait0_1(int diskn,int reader){}

void signal0(int diskn,int reader){
    //printf("Gotovo %d proces %d\n",diskn,locks[diskn].pid);
    releasesleep(&locks[diskn]);
}
void signal1(int diskn,int reader){}
void signal4(int diskn,int reader){}
void signal5(int diskn,int reader){}
void signal0_1(int diskn,int reader){}
