#ifndef LOCK_H
#define LOCK_H

#include "spinlock.h"
#include "sleeplock.h"

// Struktura za lock info po disku
struct lock_info {
    int works[7];
    int busy[7];
    int parity;
    struct sleeplock locks[7];  // do 7 diskova podržano
};

extern struct lock_info lock;
extern struct spinlock raid_lock_p;

#endif // LOCK_H

