#include "unistd.h"
#include <iostream>
#include <cstdlib>

#define TOTAL_INS 10

#define PAGE_NUM_LIMIT 5 
#define PAGE_TABLE_LIMIT 3

int access_series[TOTAL_INS];

struct PageTableEntry {

};

void LRU(){
    int 
}

void CLOCK(){
}
int main(){
    for(int i=0;i<10;++i){
        access_series[i]=rand()%PAGE_NUM_LIMIT;
        std::cout<<access_series[i];
    }

    LRU(); 
    CLOCK();



}
