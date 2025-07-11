//
// Created by parallels on 7/11/25.
//

#ifndef LOCK_H
#define LOCK_H
#include "defs.h"

void wait(enum RAID_TYPE raid,int diskn,bool reader);
void signal(enum RAID_TYPE raid,int diskn,bool reader);
//RAID0
void wait0(int diskn,bool reader);
void signal0(int diskn,bool reader);
//RAID1
void wait1(int diskn,bool reader);
void signal1(int diskn,bool reader);
//RAID4
void wait4(int diskn,bool reader);
void signal4(int diskn,bool reader);
//RAID5
void wait5(int diskn,bool reader);
void signal5(int diskn,bool reader);
//RAID1_0
void wait0_1(int diskn,bool reader);
void signal0_1(int diskn,bool reader);

#endif //LOCK_H
