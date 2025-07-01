//
// Created by parallels on 7/11/25.
//

#ifndef LOCK_H
#define LOCK_H
#include "defs.h"

void wait(enum RAID_TYPE raid,int diskn);
void signal(enum RAID_TYPE raid,int diskn);
//RAID0
void wait0(int diskn);
void signal0(int diskn);
//RAID1
void wait1(int diskn);
void signal1(int diskn);
//RAID4
void wait4(int diskn);
void signal4(int diskn);
//RAID5
void wait5(int diskn);
void signal5(int diskn);
//RAID1_0
void wait0_1(int diskn);
void signal0_1(int diskn);

#endif //LOCK_H
