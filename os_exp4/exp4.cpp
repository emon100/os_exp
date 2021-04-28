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
        lockf(fd[1],1,0);
        sprintf(buf,"Child process1 is sending a message.\n");
        write(fd[1],buf,500);
        return EXIT_SUCCESS;
    }

    //father
    if((thisPid=fork())==-1){
        return EXIT_FAILURE;
    }

    //child2
    if(thisPid==0){
        lockf(fd[1],1,0);
        sprintf(buf,"Child process2 is sending a message.\n");
        write(fd[1],buf,500);
        return EXIT_SUCCESS;
    }

    //father
    if(read(fd[0],r,500)==-1){
        return EXIT_FAILURE;
    }else{
        printf("%s\n",r);
        lockf(fd[1],0,0);
    }

    if(read(fd[0],r,500)==-1){
        return EXIT_FAILURE;
    }else{
        printf("%s\n",r);
        lockf(fd[1],0,0);
    }

    return EXIT_SUCCESS;
}
