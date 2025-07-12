struct raid_info{
  enum RAID_TYPE raid_type;
  int initialized;
  int broken;
  int disks[7];
  int total_disks;
  int total_blocks;
};

extern struct raid_info global_info_raid;

