#include "disk.h"

DISK::DISK(int logical, int physical, int block, int page) {
    logicalsize = logical;
    physicalsize = physical;
    blocksize = block;
    pagesize = page;
    pageperblock = block * 1024 / page;
    logicalpages = 1953125 * logical / pagesize / 2;//GB/KiB
    physicalpages = 1024 * 1024 * physical / pagesize;
    blocknum = physicalpages / 1024 * page / block;
    printf("logical %d physical %d block %d\n", logicalpages, physicalpages, physicalpages * page / 1024 / block);
    mappingtable = new int[logicalpages] {0,};
    fill_n(mappingtable, logicalpages, -1);
    bitmap = new int[physicalpages] {0,};
    //fill_n(mappingtable, physicalpages, -2);
    //fill(mappingtable, mappingtable + physicalpages-1, -2);
    for (int q = 0; q < physicalpages; ++q) {
        bitmap[q] = -2;
    }
    freeblock = new bool[blocknum] {1};
    for (int q = 0; q < blocknum; ++q) {
        freeblock[q] = 1;
    }
    freeblocknum = blocknum;
    currentblock = 0;
    offset = 0;
}

int DISK::io(string timestamp, int iotype, int lba, int iosize, int streamnumber) {
    //printf("lba %d\n", lba);
    //printf("lba:     %d  current block %d offet %d\n", lba,currentblock,offset);
    if (lba > logicalpages) {
        printf("wrong lba\n");
        exit(1);
    }
    if (iotype == 0) {
        read(lba, iosize);
    }
    else if (iotype == 1) {
        write(lba, iosize);
    }
    else if (iotype == 3) {
        trim(lba, iosize);
    }
    else {
        cout << "ERROR not a valid io type\n";
        return -1;
    }
    return 0;
}

int DISK::read(int lba, int iosize) {
    iosize = iosize / pagesize / 1024;
    while (iosize > 0) {
        int ppn = translate(lba);
        //need exception?



        //io_read(ppn);

        iosize--;
        lba += 1;
    }
    return 0;
}

int DISK::trim(int lba, int iosize) {
    iosize = iosize / pagesize / 1024;
    while (iosize > 0) {

        int ppn = translate(lba);
        if (ppn != -1) {
            invalidate(ppn);
            usinglba -= 1;
        }

        iosize--;
        lba++;

    }
    return 0;
}

int DISK::write(int lba, int iosize) {
    iosize = iosize / pagesize / 1024;
    if (iosize != 1) {
        printf("panic iosize");
    }
    while (iosize > 0) {

        //totalwrite++;
        totalrequestedwrite++;
        tmpwrite++;
        tmpreqeustedwrite++;
        
        int ppn = translate(lba);
        int newppn=_rwrite(lba);//maybe gc
        if (lba>logicalpages){
            printf("panic! lba>logicalpages\n");
        }
        if(newppn!=translate(lba)){
            printf("panic!\n");

        }
        bitmap[newppn] = lba;//이게 문제가 될수도
        // if (lba==7815170){
        //     printf("panic! what the\n");
        //     exit(1);
        // }
        if (ppn >-1) {
            invalidate(ppn);
        }
        else {
            usinglba++;
        }
        updatetable(lba, newppn);
        //cout << "ppn" << ppn << endl;
        
        //findnext();

        iosize--;
        lba++;
    }
    if (needgc()) {
        //printf("write needgc\n");
        gc();
    }
    return 0;
}

int DISK::_rwrite(int lba) {//write and update mappingtable and find next
    totalwrite++;
    //cout << "offset " << offset << endl;
    if (offset == 0) {//start usin new block
        //cout << "offsetset0";
        freeblocknum -= 1;
        freeblock[currentblock] = 0;
    }
    int ppn = currentblock * pageperblock + offset;
    //io_write(ppn);
    if (bitmap[ppn] != -2) {
        printf("panic! _rwrite\n");
        printf("ppn %d lba %d\n",ppn, bitmap[ppn]);
        exit(1);
    }
    updatetable(lba, ppn);
    if (findnext() < 0) {
        printf("rwrite find next fail\n");
        summary();
        gc();
        findnext();
    }
    
    
    
    return ppn;
}

int DISK::findnext() {
    ++offset;
    if (offset < pageperblock) {
        return 0;
    }
    freeblock[currentblock] = 0;
    offset = 0;
    //printf("page per block %d\n", pageperblock);
    printf("finding new block %d\n", currentblock);
    for (int i = 0; i < physicalpages; ++i) {
        currentblock = (currentblock + 1) % blocknum;
        if (freeblock[currentblock] == 1) {
            printf("using new block %d\n", currentblock);
            return 0;
        }
    }
    printf("findnext fail\n");
    return - 1;
}

void DISK::updatetable(int lba, int ppn) {
    if (lba>logicalpages){
        printf("panic updatetable wrong lba");
        exit(1);
    }
    mappingtable[lba] = ppn;
}


int DISK::invalidate(int ppn) {
    // if (ppn==1841){
    //     printf("dfasdf");
        
    // }
    if (bitmap[ppn]==-2){
        return 0;
    }
    bitmap[ppn] = -1;
    return 0;
}
int DISK::translate(int lba) {
    return mappingtable[lba];
}

int DISK::gc() {
    printf("gc start===========================================\n");
    printf("gc next %d\n", nextgc);
    summary();
    //todo
    // 
    //findvictime
    //copy valid
    //erase
    //ppn = currentblock * pageperblock + offset;



    //todo
    // need to distinguish using block
    //freeblock?
    //timestamp?
    //int blockidx = 0;//nextgcblock으로 class변수 선언하면 fifo
    while (needgc()) {
        //printf("while\n");
        if (freeblock[nextgc] == 1 || nextgc==currentblock) {
            nextgc = (nextgc + 1) % blocknum;
            continue;
        }
        printf("gc block : %d\n", nextgc);
        int ppn = nextgc * pageperblock;
        int newppn = 0;
        int lba = 0;
        for (int i = 0; i < pageperblock; ++i) {
            lba = bitmap[ppn];
            if (lba >= 0) {
                //io_read(ppn);
                newppn=_rwrite(lba);
                updatetable(lba, newppn);
                bitmap[newppn]=lba;//todo 이거를 rwrite안으로?
                // if (findnext() < 0) {
                //     printf("panic gc findnext\n");
                //     exit(1);
                // }
            }
            ppn++;
        }
        printf("gcccc\n");
        freeblock[nextgc] = 1;
        ppn = nextgc * pageperblock;
        freeblocknum++;
        for (int i = 0; i < pageperblock; ++i) {
            bitmap[ppn] = -2;
            ppn++;
        }
        nextgc = (nextgc + 1) % blocknum;
        // nextgc++;
    }
    printf("gcend=================================\n");
    summary();
    //exit(1);
    return 0;
}

int DISK::needgc() {
    return freeblocknum < 3;
}

int DISK::summary() {
    printf("totalwrite %d requestwrite %d totalerase %d freeblock %d usinglba %d\n", totalwrite, totalrequestedwrite, totalerase, freeblocknum, usinglba);
    return 0;
}
//class Block {
//public:
//    Block();
//
//private:
//    int time;
//    int usedpagenum;
//    int state;
//    int erasedtime;
//    int* bitmap;
//    
//
//};
