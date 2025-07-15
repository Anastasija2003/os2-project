#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

uchar
noise(int i, int j)
{
  int n = i + j;
  switch (n%3)
  {
    case 0:
      return '&' + (i & 3);
    case 1:
      return 'F' + (j & 3);
    default:
      return '<' + (i & 7);
  }
}

volatile int fail = 0;

int
main(void)
{
  printf("Starting..\n");

  if (init_raid(RAID0_1) < 0)
  {
    printf("Fatal error on init_raid\n");
    exit(-1);
  }

  uint diskn, blkn, blks;
  if (info_raid(&blkn, &blks, &diskn) < 0)
  {
    printf("Fatal error on info_raid\n");
    exit(-1);
  }

  printf("blkn=%d, blks=%d, diskn=%d, pid=%d\n", blkn, blks, diskn, getpid());

  //uchar data[blks];
  uchar* data = malloc(blks);
  int nproc = 10; //10
  int step = 110; //110
  int l = 0, u = step;
  int pids[nproc];
  //printf("l = %d, u = %d\n", l, u);


  pids[0] = getpid();
  for (int i = 1; i < nproc; i++) {
    int pid = fork();
    if (pid == 0)
    {
      pids[i] = pid;
      l = step * (i + 0);
      u = step * (i + 1);
      //printf("l = %d, u = %d\n", l, u);
      break;
    }
  }


  for (int i = l; i < u && i < blkn; i++)
  {
    for (int j = 0; j < blks; j++)
      data[j] = noise(i, j);
    int ret = 0;
    if(i % 50 == 0) printf("write_%d\n", i);
    if ((ret = write_raid(i, data)) < 0)
    {
      printf("blkn = %d\n", i);
      printf("Fatal error in write_raid pid = %d, ret = %d\n", getpid(), ret);
      exit(-2);
    }
    if(pids[0] == getpid() && i == 4) {
      disk_fail_raid(3);
      printf("izaso fail 3\n");
    }
    if(i == 100){
      disk_fail_raid(2);
      printf("izaso fail 1\n");
    }
    if(i == 720) { //500
      printf("USOREPAIR\n");
      //while(!fail);
      if((ret = disk_repaired_raid(3)) < 0) {
        printf("Fatal error in repaired_raid pid = %d, ret = %d\n", getpid(), ret);
      }
    }
  }

  for (int i = l; i < u && i < blkn; i++)
  {
    for (int j = 0; j < blks; j++)
      data[j] = 'c';
    if(i % 50 == 0) printf("read_%d\n", i);
    if (read_raid(i, data) < 0)
    {
      printf("Fatal error in read_raid\n");
      exit(-3);
    }

    for (int j = 0; j < blks; j++)
      if (data[j] != noise(i, j))
      {
        printf("Bad read. i = %d, j = %d, pid = %d, l = %d, u = %d\n", i, j, getpid(), l, u);
        exit(-4);
      }
  }

  printf("Good read, pid = %d\n", getpid());

  int status = 0;
  if(pids[0] == getpid()) {
    for(int i = 1; i < nproc; i++) {
      wait(&status);
    }
    free(data);
    destroy_raid();
  }

  exit(0);
}