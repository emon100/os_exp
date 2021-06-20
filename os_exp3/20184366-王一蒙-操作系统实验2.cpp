#include <vector>
#include <queue>
#include <iostream>
using namespace std;

const int BUFSZ = 5;

int buf[BUFSZ] = {};
int emptyPtr=0;
int fullPtr=0;
// emptyPtr == fullPtr : totally empty.
// emptyPtr == fullPtr-1 : totally full.
//


struct Proc{
    bool isProducer=true;
    int waiting=0;
};

void exec(Proc &);

queue<Proc> currentRunning;

struct Semaphore{
    int q=0;
    queue<Proc> process;
    void add(){
        if(process.empty()){
            ++this->q;
        }else{
            exec(this->process.front());
            process.pop();
        }
    }
};

Semaphore bufFull;
Semaphore bufEmpty;

bool P(Semaphore &s){
    if(s.q>0){
        --s.q;
        return true;
    }else{
        cout<<"Blocked"<<'\n';
        return false;
    }
}

bool V(Semaphore &s){
    s.add();
    return true;
}

void producer(Proc &s){
    switch(s.waiting){
        case 0:
        if(!P(bufEmpty)){
            s.waiting=1;
            bufEmpty.process.push(s);
            return;
        }
        case 1:
        buf[emptyPtr%BUFSZ]=rand();
        cout<<"Producer produce: "<<buf[emptyPtr%BUFSZ]<<'\n';
        ++emptyPtr;
        V(bufFull);
    }
}

void consumer(Proc &s){
    switch(s.waiting){
        case 0:
        if(!P(bufFull)){
            s.waiting=1;
            bufFull.process.push(s);
            return;
        }
        case 1:
        cout<<"Consumer read: "<<buf[fullPtr%BUFSZ]<<'\n';
        ++fullPtr;
        V(bufEmpty);
    }
}

void exec(Proc &s){
    if(s.isProducer){
        producer(s);
    }else{
        consumer(s);
    }
}

int main(void){
    bufEmpty.q = 4;
    for(;;){
        for(int i=0;i<BUFSZ;++i){
            cout<<buf[i]<<' ';
        }
        cout<<'\n';
        cout<<"currentRunning proc queue size: "<<currentRunning.size()<<'\n';
        cout<<"bufEmpty proc queue size: "<<bufEmpty.process.size()<<'\n';
        cout<<"bufFull proc queue size: "<<bufFull.process.size()<<'\n';
        char c = cin.get();
        cin>>c;
        if(c=='p'){
            //Put a producer in current running proc.
            cout<<"Start a producer process"<<'\n';
            currentRunning.emplace();
            currentRunning.back().isProducer=true;
        }else if(c=='c'){
            //Put a consumer in current running proc.
            cout<<"Start a consumer process"<<'\n';
            currentRunning.emplace();
            currentRunning.back().isProducer=false;
        }else if(c=='e'){
            //execute current running proc.
            if(currentRunning.empty()){
                cout<<"Please start a process"<<"\n\n\n";
                continue;
            }
            exec(currentRunning.front());
            currentRunning.pop();
        }else{
            cout<<"input error, please retry."<<"\n\n\n";
        }
        cout<<"\n\nThis round finished\n\n\n\n";
    }
}
