//
// Created by parallels on 7/11/25.
//
void wait(enum RAID_TYPE raid, int diskn){
    switch(raid){
        case RAID0: wait0(diskn); break;
        case RAID1: wait1(diskn); break;
        case RAID4: wait4(diskn); break;
        case RAID5: wait5(diskn); break;
        case RAID0_1: wait0_1(diskn); break;
    }
}

void signal(enum RAID_TYPE raid,int diskn){
    switch(raid){
        case RAID0: signal0(diskn); break;
        case RAID1: signal1(diskn); break;
        case RAID4: signal4(diskn); break;
        case RAID5: signal5(diskn); break;
        case RAID0_1: signal0_1(diskn); break;
    }
}

void wait0(int diskn){

}
void wait1(int diskn){}
void wait4(int diskn){}
void wait5(int diskn){}
void wait0_1(int diskn){}

void signal0(int diskn){
}
void signal1(int diskn){}
void signal4(int diskn){}
void signal5(int diskn){}
void wait0_1(int diskn){}