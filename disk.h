#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <cstdio>
#include <algorithm>


#include <random>
#include <cmath>


using std::cout;
using std::string;
using std::ifstream;
using std::endl;
using namespace std;

class DISK {
public:
    int io(string timestamp, int iotype, int lba, int iosize, int streamnumber);
    int summary();
    int summary2();
    int resetsummary();
    
    DISK(int logical, int physical, int block, int page,int policynum,int needgc2th);

private:

    //stat
    int totalwrite = 0;
    int totalrequestedwrite = 0;
    int tmpwrite = 0;
    int tmpreqeustedwrite = 0;
    int tmpgc=0;
    int tmperase=0;
    long long tmpvalid=0;
    int totalerase = 0;
    int totalgc=0;
    int freeblocknum;
    int usinglba = 0;//

    int logicalsize;//GB
    int physicalsize;//GiB
    int blocksize;//MiB
    int pagesize;//KiB
    int policy=1;
    int needgc2threshold=3;

    int logicalpages;//# of logical page
    int physicalpages;//# of physical page

    int* bitmap;//ppn to lba  -1 invalid -2 free else valid (lba)
    int* mappingtable;//lba to ppn -1 not assigned
    int* freeblock;//0 free else timestamp
    int* validpage;

    int* wear;
    // int* de_bitmap;

    int pageperblock;

    int read(int lba, int iosize);
    int write(int lba, int iosize);
    int trim(int lba, int iosize);


    int translate(int lba);
    int needgc();

    int needgc2();
    int gc();
    int (DISK::*qqq)();
    int gcpolicy0();//fifo
    int gcpolicy1();//greedy
    int gcpolicy2();//cost benefit
    int gcpolicy3();
    int gcpolicy_random();

    int invalidate(int ppn);
    void updatetable(int lba, int ppn);



    int findnext();
    int blocknum;
    int nextgc = 0;

    //int nextfreepage;

    int currentblock;
    int offset;


    int io_read(int ppn);
    int io_write(int ppn);
    int _rwrite(int lba);
};