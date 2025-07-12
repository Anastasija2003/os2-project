#ifndef LOCK_H
#define LOCK_H

#include "spinlock.h"
#include "sleeplock.h"

// Struktura za lock info po disku
struct lock_info {
    struct sleeplock locks[7];  // do 7 diskova podržano
};

extern struct lock_info lock_disks;
#endif // LOCK_H

