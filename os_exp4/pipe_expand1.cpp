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
    lockf(fd[1],1,0);

    //Create child1
    if((thisPid=fork())==-1){
        return EXIT_FAILURE;
    }

    if(thisPid==0){//child1
        lockf(fd[1],1,0);
        if(read(fd[0],r,500)==-1){
            return EXIT_FAILURE;
        }else{
            printf("Child 1 receives: %s",r);
        }
        return EXIT_SUCCESS;
    }

    //father
    if((thisPid=fork())==-1){
        return EXIT_FAILURE;
    }

    //child2
    if(thisPid==0){
        lockf(fd[1],1,0);
        if(read(fd[0],r,500)==-1){
            return EXIT_FAILURE;
        }else{
            printf("Child 2 receives: %s",r);
        }
        return EXIT_SUCCESS;
    }

    //father
    if((thisPid=fork())==-1){
        return EXIT_FAILURE;
    }

    //child3
    if(thisPid==0){
        lockf(fd[1],1,0);
        if(read(fd[0],r,500)==-1){
            return EXIT_FAILURE;
        }else{
            printf("Child 3 receives: %s",r);
        }
        return EXIT_SUCCESS;
    }

    //father
    sprintf(buf,"Father is sending a message.\n");
    write(fd[1],buf,500);
    lockf(fd[1],0,0);
    return EXIT_SUCCESS;
}
