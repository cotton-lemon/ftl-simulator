#define GLO
#include "disk.h"

using namespace std;

int main(int argc, char* argv[])
{
    ifstream file("/home/lemonos22/tlc/test-fio-small_stream");
    if (file.fail()){
        return -1;
    }
    else{
        printf("succed");
    }
    int needgcth=3;
    cout<<"argc: "<<argc<<endl;
    if (argc>1){
        needgcth=stoi(argv[1]);

    }
    if (argc>2){
        ENABLE_MULTI_STREAM=stoi(argv[2]);
    }
    if (argc>3){
        GC_WITH_STREAM=stoi(argv[3]);
    }
    printf("need gc2 threshold %d\n",needgcth);
    printf("enable multi stream %d\n",ENABLE_MULTI_STREAM);
    printf(" gc with stream %d ]n",GC_WITH_STREAM);
    
    //stringstream ss;

    int logical = 8;//GB
    int physical = 8;//GiB
    
    int blocksize = 4;//MB
    int pagesize = 4;//KB

    DISK disk1(8, 8, 4 , 4 ,1,needgcth);
    //DISK disk1(4, 4, 1, 1);


    string timestamp;
    int iotype;
    int lba;
    int iosize;
    int streamnumber;
    // for (int i = 0; i < 100; ++i)
    int linenum=0;
    int loop=0;
    while(!file.eof()) {
       file >> timestamp;
       file >> iotype;
       file >> lba;
       file >> iosize;
       file >> streamnumber;
       
    //    cout << timestamp << " " << iotype << " " << lba << " " << iosize << streamnumber << endl;
        if (ENABLE_MULTI_STREAM==0){
            streamnumber=0;
        }
       disk1.io(timestamp, iotype, lba, iosize, streamnumber);
       linenum++;
       if (linenum%(1<<21)==0){
        printf("loop %d\n",loop);
        linenum=0;
        loop++;
        disk1.summary();
        disk1.summary2();
        // disk1.resetsummary();
       }
       if (linenum%((1<<19)*25)==0){
        disk1.resetsummary();
       }

    }
    printf("linenum %d loop %d\n",linenum,loop);
    // for (int j = 0; j < 5; ++j) {
    //     for (int i = 0; i < 1953125;++i) {
    //         disk1.io("1", 1, i, 4096, 1);
            
    //     }
    //     for (int i = 1953125; i > 0; --i) {
    //         disk1.io("1", 1, 1, 4096, 1);
    //     }
    // }

 /*   for (int i = 0; i < 2000000; ++i) {
        disk1.io("1", 1, 1, 4096, 0);
    }*/
    disk1.summary();
    disk1.summary2();
    disk1.summary3();
    return 0;
}

//todo freeblock queue
//page 단위 age