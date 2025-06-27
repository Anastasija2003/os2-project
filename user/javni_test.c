#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void check_data(uint blocks, uchar *blk, uint block_size);

int
main(int argc, char *argv[])
{
  
  int ret = init_raid(RAID0);
  if(ret < 0){
    printf("init_raid failed 1\n");
    return -1;
  }
  uint disk_num, block_num, block_size;
  info_raid(&block_num, &block_size, &disk_num);
  uint blocks = (512 > block_num ? block_num : 512);

  uchar* blk = malloc(block_size);
  for (uint i = 0; i < blocks; i++) {
    for (uint j = 0; j < block_size; j++) {
      blk[j] = j + i;
    }
    write_raid(i, blk);
  }

  check_data(blocks, blk, block_size);
  printf("Before fail\n");

  disk_fail_raid(2);

  check_data(blocks, blk, block_size);
  printf("After fail\n");

  disk_repaired_raid(2);

  check_data(blocks, blk, block_size);
  printf("After repaired\n");

  free(blk);

  exit(0);
}

void check_data(uint blocks, uchar *blk, uint block_size)
{
  for (uint i = 0; i < blocks; i++)
  {
    read_raid(i, blk);
    for (uint j = 0; j < block_size; j++)
    {
      if ((uchar)(j + i) != blk[j])
      {
        printf("expected=%d got=%d", j + i, blk[j]);
        printf("Data in the block %d faulty\n", i);
        break;
      }/*else if(i == blocks-1){
        printf("expected=%c got=%c", (uchar)(j + i), blk[j]);
        printf("Data in the block %d good\n", i);
      }*/
    }
  }
}
