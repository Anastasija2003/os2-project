#ifndef LOCK_H
#define LOCK_H

#include <stdbool.h>

// Struktura za lock info po disku
struct lock_info {
    int readers;
    int writers;
    struct sleeplock* locks[7];  // do 7 diskova podržano
};

extern struct lock_info lock_disks;
#endif // LOCK_H

