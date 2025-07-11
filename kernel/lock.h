#ifndef LOCK_H
#define LOCK_H

#include "types.h"
#include "sleeplock.h"
#include <stdbool.h>  // za 'bool'

// Glavni API
void wait(enum RAID_TYPE raid, int diskn, bool reader);
void signal(enum RAID_TYPE raid, int diskn, bool reader);

// RAID-specific varijante
void wait0(int diskn, bool reader);
void signal0(int diskn, bool reader);

void wait1(int diskn, bool reader);
void signal1(int diskn, bool reader);

void wait4(int diskn, bool reader);
void signal4(int diskn, bool reader);

void wait5(int diskn, bool reader);
void signal5(int diskn, bool reader);

void wait0_1(int diskn, bool reader);
void signal0_1(int diskn, bool reader);

// Struktura za lock info po disku
struct lock_info {
    int readers;
    int writers;
    struct sleeplock* locks[7];  // do 7 diskova podržano
};

#endif // LOCK_H
v
