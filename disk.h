#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include  <cstdio>
#include <algorithm>
using std::cout;
using std::string;
using std::ifstream;
using std::endl;
using namespace std;

class DISK {
public:
    int io(string timestamp, int iotype, int lba, int iosize, int streamnumber);
    int summary();
    DISK(int logical, int physical, int block, int page);

private:

    //stat
    int totalwrite = 0;
    int totalrequestedwrite = 0;
    int tmpwrite = 0;
    int tmpreqeustedwrite = 0;
    int totalerase = 0;
    int freeblocknum;
    int usinglba = 0;//

    int logicalsize;//GB
    int physicalsize;//GiB
    int blocksize;//MiB
    int pagesize;//KiB

    int logicalpages;//# of logical page
    int physicalpages;//# of physical page

    int* bitmap;//ppn to lba  -1 invalid -2 free else valid (lba)
    int* mappingtable;//lba to ppn -1 not assigned
    bool* freeblock;//1 free 0 used 2 using??

    int pageperblock;

    int read(int lba, int iosize);
    int write(int lba, int iosize);
    int trim(int lba, int iosize);


    int translate(int lba);
    int needgc();
    int gc();
    int invalidate(int ppn);
    bool gcpolicy();
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