#include "unistd.h"
#include "wait.h"
#include <cstdio>
#include <cstdlib>


int main(){
    int thisPid = 0;
    int fd[2]={};
    char buf[500]={};
    char r[500]={};
    //Create pipe.
    if(pipe(fd)==-1){
        return EXIT_FAILURE;
    }

    //Create child1
    if((thisPid=fork())==-1){
        return EXIT_FAILURE;
    }

    if(thisPid==0){//child1
        for(int i=0;i<10;++i){
            write(fd[1],"1",2);
            sleep(0);
        }
        return EXIT_SUCCESS;
    }

    //father
    if((thisPid=fork())==-1){
        return EXIT_FAILURE;
    }

    //child2
    if(thisPid==0){
        for(int i=0;i<10;++i){
            write(fd[1],"2",2);
            sleep(0);
        }
        return EXIT_SUCCESS;
    }
    for(int i=0;i<20;++i){
        if(read(fd[0],r,2)!=-1){
            printf(r);
        }
    }

    return EXIT_SUCCESS;
}
