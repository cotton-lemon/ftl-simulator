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
    int* bitmap;//ppn to lba  -1 invalid -2 free else valid (lba)
    int* mappingtable;//lba to ppn -1 not assigned
    int* freeblock;//0 free else timestamp
    int* validpage;
    // int* wear;
    int read(int lba, int iosize);
    int write(int lba, int iosize);
    int trim(int lba, int iosize);
    int gc();
    int needgc();
    int needgc2();

    int (DISK::*gcpolicy)();
    int gcpolicy0();//fifo
    int gcpolicy1();//greedy
    int gcpolicy2();//cost benefit
    int gcpolicy_random();

    int findnext();
    int blocknum;
    int nextgc = 0;

    int currentblock;
    int offset;
    int _rwrite(int lba);
};