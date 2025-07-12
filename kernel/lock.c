//
// Created by parallels on 7/11/25.
//
#include "defs.h"
#include "lock.h"

void wait0(int diskn,int reader){}
void wait1(int diskn,int reader){}
void wait4(int diskn,int reader){}
void wait5(int diskn,int reader){}
void wait0_1(int diskn,int reader){}

void signal0(int diskn,int reader){}
void signal1(int diskn,int reader){}
void signal4(int diskn,int reader){}
void signal5(int diskn,int reader){}
void signal0_1(int diskn,int reader){}

struct lock_info lock_disks;