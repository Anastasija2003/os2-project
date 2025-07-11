//
// Created by parallels on 7/11/25.
//
void wait(enum RAID_TYPE raid, int diskn,bool reader);{
    switch(raid){
        case RAID0: wait0(diskn,reader); break;
        case RAID1: wait1(diskn,reader); break;
        case RAID4: wait4(diskn,reader); break;
        case RAID5: wait5(diskn,reader); break;
        case RAID0_1: wait0_1(diskn,reader); break;
    }
}

void signal(enum RAID_TYPE raid,int diskn){
    switch(raid){
        case RAID0: signal0(diskn,reader); break;
        case RAID1: signal1(diskn,reader); break;
        case RAID4: signal4(diskn,reader); break;
        case RAID5: signal5(diskn,reader); break;
        case RAID0_1: signal0_1(diskn,reader); break;
    }
}

void wait0(int diskn,bool reader){

}
void wait1(int diskn,bool reader){}
void wait4(int diskn,bool reader){}
void wait5(int diskn,bool reader){}
void wait0_1(int diskn,bool reader){}

void signal0(int diskn,bool reader){
}
void signal1(int diskn,bool reader){}
void signal4(int diskn,bool reader){}
void signal5(int diskn,bool reader){}
void wait0_1(int diskn,bool reader){}