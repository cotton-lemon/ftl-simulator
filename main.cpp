// ftl.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

//#include <iostream>
//#include <fstream>
//#include <string>
//#include <sstream>
//#include <stdlib.h>
//#include  <cstdio>
//using std::cout;
//using std::string;
//using std::ifstream;
//using std::endl;
//using namespace std;


#include "disk.h"

using namespace std;

int main()
{
    string line;
    ifstream file("C:\\users\\sg\\Downloads\\test-fio-small");
    //stringstream ss;

    int logical = 8;//GB
    int physical = 8;//GiB
    int blocksize = 4;//MB
    int pagesize = 4;//KB

    DISK disk1(8, 8, 4 , 4 );
    //DISK disk1(4, 4, 1, 1);


    string timestamp;
    int iotype;
    int lba;
    int iosize;
    int streamnumber;
    //for (int i = 0; i < 10000000; ++i) {
    //    file >> timestamp;
    //    file >> iotype;
    //    file >> lba;
    //    file >> iosize;
    //    file >> streamnumber;
    //    disk1.io(timestamp, iotype, lba, iosize, streamnumber);
    //    //cout << timestamp << " " << iotype << " " << lba << " " << iosize << streamnumber << endl;
    //    //disk1.summary();
    //    //disk1.io(timestamp, iotype, lba, iosize, streamnumber);
    //    //disk1.summary();

    //}
    for (int j = 0; j < 5; ++j) {
        for (int i = 0; i < 1953125;++i) {
            disk1.io("1", 1, i, 4096, 1);
            
        }
        for (int i = 1953125; i > 0; --i) {
            disk1.io("1", 1, 1, 4096, 1);
        }
    }

 /*   for (int i = 0; i < 2000000; ++i) {
        disk1.io("1", 1, 1, 4096, 0);
    }*/
    disk1.summary();
    
    return 0;
}