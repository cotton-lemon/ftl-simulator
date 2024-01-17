#include "disk.h"


DISK::DISK(int logical, int physical, int block, int page,int policynum,int needgc2th) {
    logicalsize = logical;
    physicalsize = physical;
    blocksize = block;
    pagesize = page;
    policy=policynum;
    needgc2threshold=needgc2th;
    // qqq=&(gcpolicy0);



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
    validpage=new int[blocknum]{0,};
    // de_bitmap = new int[physicalpages] {0,};
    for (int q = 0; q < physicalpages; ++q) {
        bitmap[q] = -2;
    }
    // blocktime = new int[blocknum]{0,};
    wear= new int[blocknum]{0,};
    freeblock = new int[blocknum]{0,};
    // for (int q = 0; q < blocknum; ++q) {
    //     freeblock[q] = 1;
    // }
    for(int i=1;i<blocknum;++i){
        freeblockqueue.push(i);
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
    // int pp=0;
    // for (int block=0;block<2048;++block){
    //     int valid=0;
    //     for (int k=0;k<pageperblock;++k){
    //         if (bitmap[pp]>=0){
    //             valid+=1;
    //         }
    //         ++pp;
    //     }
    //     if ((valid!= validpage[block])&&(block!=currentblock)){
    //         printf("panic! validpage wrong block: %d valid: %d validpage[block]%d\n",block ,valid,validpage[block]);
    //         pp+=0;

    //     }
    // }
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
        totalrequestedwrite++;
        tmpreqeustedwrite++;
        
        int ppn = translate(lba);
        int newppn=_rwrite(lba);//maybe gc
        if (lba>logicalpages){
            printf("panic! lba>logicalpages\n");
        }
        if(newppn!=translate(lba)){
            printf("panic!\n");

        }
        // bitmap[newppn] = lba;//이게 문제가 될수도
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
    tmpwrite++;
    //cout << "offset " << offset << endl;

    validpage[currentblock]+=1;
    if (offset == 0) {//start usin new block
        //cout << "offsetset0";
        freeblocknum -= 1;
        // freeblock[currentblock] = 0;
    }
    int ppn = currentblock * pageperblock + offset;
    //io_write(ppn);

    if (bitmap[ppn] != -2) {
        printf("panic! _rwrite\n");
        printf("ppn %d lba %d\n",ppn, bitmap[ppn]);
        exit(1);
    }
    bitmap[ppn]=lba;
    // de_bitmap[ppn]=lba;
    if (lba<0){
        printf("panic!\n");
        exit(1);
    }
    updatetable(lba, ppn);
    if (findnext() < 0) {
        printf("rwrite find next fail\n");
        exit(1);
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
    freeblock[currentblock] = totalrequestedwrite;
    // blocktime[currentblock]=totalwrite;
    offset = 0;
    //printf("page per block %d\n", pageperblock);
    // printf("finding new block %d\n", currentblock);
    #if enable_freeblock_queue ==1

    currentblock=freeblockqueue.front();
    freeblockqueue.pop();
    if(freeblock[currentblock]!=0){
        printf("panic! findnext fail");
        exit(1);
    }
    return 0;
    #endif
    for (int i = 0; i < physicalpages; ++i) {
        currentblock = (currentblock + 1) % blocknum;
        if (freeblock[currentblock] == 0) {
            // printf("using new block %d\n", currentblock);
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
        printf("panic! invalidate freepage\n");
        exit(1);
        return 0;
    }
    if (bitmap[ppn]<0){
        printf("whatthe\n");
        exit(1);
    }
    bitmap[ppn] = -1;
    int block=ppn/pageperblock;
    validpage[block]-=1;
    if (block!=currentblock){
        // freeblock[block]=totalrequestedwrite;//updatetime stamp
    }

    return 0;
}
int DISK::translate(int lba) {
    return mappingtable[lba];
}

//garbage collection
int DISK::gc() {
    // printf("gc start===========================================\n");
    totalgc+=1;
    tmpgc+=1;
    while (needgc2()) {
        totalerase+=1;
        tmperase+=1;
        //printf("while\n");
        // while(true){
        // if (freeblock[nextgc] == 1 || nextgc==currentblock) {
        //     nextgc = (nextgc + 1) % blocknum;
        //     continue;
        // }
        // break;
        // }
        //choose policy
        // gcpolicy();
        // gcpolicy0();
        // gcpolicy1();
        gcpolicy2();
        
        // gcpolicy3();
        // gcpolicy_random();
        // printf("gc block : %d\n", nextgc);

        wear[nextgc]+=1;
        int ppn = nextgc * pageperblock;
        int newppn = 0;
        int lba = 0;
        int validpagenum=0;
        for (int i = 0; i < pageperblock; ++i) {
            lba = bitmap[ppn];
            if (lba >= 0) {
                validpagenum+=1;
                //io_read(ppn);
                newppn=_rwrite(lba);
                updatetable(lba, newppn);
                
                // if (findnext() < 0) {
                //     printf("panic gc findnext\n");
                //     exit(1);
                // }
            }
            ppn++;
        }
        if (validpage[nextgc]!=validpagenum){
            printf("panic! wrong validpage num\n");
            exit(1);
        }
        tmpvalid+=validpagenum;
        freeblock[nextgc] = 0;
        wear[nextgc]+=1;
        ppn = nextgc * pageperblock;
        freeblocknum++;
        for (int i = 0; i < pageperblock; ++i) {
            bitmap[ppn] = -2;
            ppn++;
        }

    # if enable_freeblock_queue==1
        freeblockqueue.push(nextgc);
    # endif
        validpage[nextgc]=0;
        nextgc = (nextgc + 1) % blocknum;
        // nextgc++;
        // if (needgc()){
        //     printf("needgc again %d\n",nextgc);
        // }

        // printf("valid ratio %.3f\n",(1.*validpagenum)/pageperblock);
    }
    

    // printf("gcend=================================\n");
    // summary();
    //exit(1);
    // int pp=0;
    // for (int block=0;block<2048;++block){
    //     int valid=0;
    //     for (int k=0;k<pageperblock;++k){
    //         if (bitmap[pp]>=0){
    //             valid+=1;
    //         }
    //         ++pp;
    //     }
    //     if ((valid!= validpage[block])&&(block!=currentblock)){
    //         printf("panic! validpage wrong block: %d valid: %d validpage[block]%d\n",block ,valid,validpage[block]);
    //         pp+=0;

    //     }
    // }
    return 0;
}

//FIFO
int DISK::gcpolicy0(){
    while(true){
        if (freeblock[nextgc] ==0 || nextgc==currentblock) {
            nextgc = (nextgc + 1) % blocknum;
            continue;
        }
        return 0;
        }
}
//greedy
int DISK::gcpolicy1(){
    nextgc=0;
    int minvalid=2048;
    for (int i=0; i<blocknum;++i){
        if (validpage[i]<minvalid&&(i!=currentblock)&&(freeblock[i]!=0)){
            nextgc=i;
            minvalid=validpage[i];
        }
    }
    return 0;
}
//Cost benefit
int DISK::gcpolicy2(){
    nextgc=0;
    float maxvalue=0;
    for (int i=0; i<blocknum;++i){
        float t=(static_cast<float>(totalrequestedwrite-freeblock[i])*static_cast<float>(pageperblock-validpage[i])/(2*validpage[i]));
        // if (t==maxvalue){
        //     printf("same maxvalue\n");
        // }
        if (t>maxvalue&&(i!=currentblock)&&(freeblock[i]!=0)){
            nextgc=i;
            maxvalue=t;
        }
    }
    return 0;
}
//costbenefit with log
int DISK::gcpolicy3(){
    nextgc=0;
    float maxvalue=0;
    for (int i=0; i<blocknum;++i){
        if (freeblock[i]==0){
            continue;
        }
        // float t=((totalrequestedwrite-freeblock[i])*0.01+(pageperblock-validpage[i]));

        float t=(log(totalrequestedwrite-freeblock[i])*(pageperblock-validpage[i])/(2*validpage[i]));
        // printf("%d %d\n",totalrequestedwrite-freeblock[i],pageperblock-validpage[i]);
        if (t>maxvalue&&(i!=currentblock)){
            nextgc=i;
            maxvalue=t;
        }
    }
    // exit(1);
    return 0;
}

int DISK::gcpolicy_random(){
  std::random_device rd;

  std::mt19937 gen(rd());

  std::uniform_int_distribution<int> dis(0, blocknum);
  nextgc=dis(gen);
  while (freeblock[nextgc]==0 || nextgc==currentblock){
    nextgc=dis(gen);
  }
  return 0;
}
int DISK::needgc() {
    return freeblocknum < 2;
}
//ngc
int DISK::needgc2() {
    return freeblocknum < needgc2threshold;
}
int DISK::summary() {
    // printf("totalwrite %d requestwrite %d totalerase %d freeblock %d usinglba %d\n", totalwrite, totalrequestedwrite, totalerase, freeblocknum, usinglba);
    // return 0;
    // [Progress: 8GiB] WAF: 1.012, TMP_WAF: 1.024, Utilization: 1.000
// GROUP 0[2046]: 0.02 (ERASE: 1030)
    printf("[Progress: %d GiB] WAF: %.3f, TMP_WAF: %.3f, Utilization: %.3f\n" ,totalrequestedwrite*pagesize/(1024*1024),1.*totalwrite/totalrequestedwrite,1.*tmpwrite/tmpreqeustedwrite,1.*usinglba/logicalpages);
    printf("GROUP 0[%d]:%.6f  (ERASE: %d)\n",blocknum-freeblocknum,1.*tmpvalid/(tmperase*pageperblock),tmperase);
    // tmpwrite=0;
    // tmpreqeustedwrite=0;
    // tmperase=0;
    // tmpvalid=0;
    return 0;
}

int DISK::resetsummary(){
    tmpwrite=0;
    tmpreqeustedwrite=0;
    tmperase=0;
    tmpvalid=0;
    tmpgc=0;
    return 0;
}
int DISK::summary2(){
    printf("totalwrite %d requestwrite %d totalerase %d freeblock %d usinglba %d\n", totalwrite, totalrequestedwrite, totalerase, freeblocknum, usinglba);
    printf("total gc %d tmpgc %d total erase per gc %.3f, tmp %.3f\n",totalgc,tmpgc,static_cast<double>(totalerase)/totalgc,static_cast<double>(tmperase)/tmpgc);
    return 0;
}
int DISK::summary3(){
    for (int i=0; i<blocknum;++i){
        cout<< wear[i]<<' ';
    }
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
