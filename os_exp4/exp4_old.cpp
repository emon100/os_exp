#include <cstdio>
#include "unistd.h"
#include "wait.h"
#include <cstdlib>


int main(){
    int thisPid = 0;
    int fd[2]={};
    char buf[500]={};
    char r[500]={};
    if(pipe(fd)==-1){
        return EXIT_FAILURE;
    }
    if((thisPid=fork())==-1){
        return EXIT_FAILURE;
    }

    if(thisPid==0){//child1
        lockf(fd[1],1,0);
        sprintf(buf,"Child process p1 is sending\n");
        write(fd[1],buf,500);
        sleep(5);//Sleep here because we want to make sure that the sequence is child1->father->child2.
        lockf(fd[1],0,0);
        return EXIT_SUCCESS;
    }

    //father
    if((thisPid=fork())==-1){
        return EXIT_FAILURE;
    }
    if(thisPid==0){//child2
        lockf(fd[1],1,0);
        sprintf(buf,"Child process p2 is sending\n");
        write(fd[1],buf,500);
        sleep(5);
        lockf(fd[1],0,0);
        return EXIT_SUCCESS;
    }
    //father
    wait(0);
    if(read(fd[0],r,500)==-1){
        return EXIT_FAILURE;
    }else{
        printf("%s\n",r);
    }

    wait(0);
    if(read(fd[0],r,500)==-1){
        return EXIT_FAILURE;
    }else{
        printf("%s\n",r);
    }

    return EXIT_SUCCESS;
}
