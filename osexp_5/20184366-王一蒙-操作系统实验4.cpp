#include "unistd.h"
#include "wait.h"
#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>

using namespace std;


int fd[2];
char sendBuff[1000];
int sBuffCount = 0;
char rBuff[1000];

const int SEQ_LENGTH = 12;

const int LOGIC_PAGE_COUNT = 5;

const int REAL_PAGE_COUNT = 3;

vector<int> seq;
void displayReal(vector<int> &real) {
    for (auto item : real) {
        // cout << item << ' ';
        sBuffCount = sprintf(sendBuff, "%d ", item);
        write(fd[1], sendBuff, sBuffCount);
    }
    sBuffCount = sprintf(sendBuff, "\n");
    write(fd[1], sendBuff, sBuffCount);
    // cout << '\n';
}

void shl(vector<int> &reg, int flag) {
    if (!reg.empty()) {
        reg.erase(reg.begin());
    }
    reg.push_back(flag);
}

int getVal(vector<int> &reg) {
    if (reg.size() > 31) {
        return -1;
    }
    int val = 0;
    for (int i = reg.size() - 1; i >= 0; --i) {
        val = val*2 + reg[i];
    }
    return val;
}

void initReg(vector<int> &reg) {
    reg.clear();
    for (int i = 0 ; i < LOGIC_PAGE_COUNT; ++i) {
        reg.push_back(0);
    }
}

int fifo() {
    vector<int> real;
    int hit = 0;
    for (auto item : seq) {
        auto tar = find(real.begin(), real.end(), item);
        if (tar == real.end()) {
            if (real.size() >= REAL_PAGE_COUNT) {
                real.erase(real.begin());
            }
            real.push_back(item);
        } else {
            hit++;
        }
        displayReal(real);
    }
    sBuffCount = sprintf(sendBuff, "FIFO: %d / %d\n", hit, SEQ_LENGTH);
    write(fd[1], sendBuff, sBuffCount);
    // cout << "FIFO: " << hit << " / " << SEQ_LENGTH << endl;
    return EXIT_SUCCESS;
}

int lru() {

    vector<vector<int> > regs(LOGIC_PAGE_COUNT, vector<int>(LOGIC_PAGE_COUNT,0));

    vector<int> real;
    int hit = 0;
    for (auto item : seq) {


        auto findOne = find(real.begin(), real.end(), item);
        if (findOne != real.end()) {
            for (int i = 0; i < regs.size(); ++i) {
                shl(regs[i], int(i == item));
            }
            ++hit;
            displayReal(real);
            continue;
        }


        if (real.size() < REAL_PAGE_COUNT) {
            real.push_back(item);
            for (int i = 0; i < regs.size(); ++i) {
                shl(regs[i], int(i == item));
            }
            displayReal(real);
            continue;
        }

        int tar = 0;
        int min = 0x7f7f7f7f;
        for (int i : real) {
            int curVal = getVal(regs[i]);
            if (curVal < min) {
                min = curVal;
                tar = i;
            }
        }
        real.erase(find(real.begin(), real.end(), tar));
        real.push_back(item);
        initReg(regs[tar]);

        for (int i = 0; i < regs.size(); ++i) {
            shl(regs[i], int(i == item));
        }

        displayReal(real);
    }
    sBuffCount = sprintf(sendBuff, "LRU: %d / %d\n", hit, SEQ_LENGTH);
    write(fd[1], sendBuff, sBuffCount);
    // cout << "LRU: " << hit << " / " << SEQ_LENGTH << endl;
    return EXIT_SUCCESS;
}

int main() {

    srand(time(nullptr));
    seq.clear();
    for (int i = 0 ; i < SEQ_LENGTH; ++i) {
        seq.push_back( rand() % LOGIC_PAGE_COUNT );
        cout << seq[i] << ' ';
    }
    cout << '\n';

    if (pipe(fd) == -1) {
        return EXIT_FAILURE;
    }

    int curPid = 0;

    if ((curPid=fork()) == -1) {
        return EXIT_FAILURE;
    }
    // child 1
    if (curPid == 0) {
        lockf(fd[1], 1, 0);
        return fifo();
    }

    if ((curPid=fork()) == -1) {
        return EXIT_FAILURE;
    }
    // child 2
    if (curPid == 0) {
        lockf(fd[1], 1, 0);
        return lru();
    }

    //father
    if (read(fd[0],rBuff,500) == -1) {
        return EXIT_FAILURE;
    } else {
        printf("%s\n", rBuff);
        lockf(fd[1],0,0);
    }

    wait(0);
    //father
    if (read(fd[0],rBuff,500) == -1) {
        return EXIT_FAILURE;
    } else {
        printf("%s\n", rBuff);
        lockf(fd[1],0,0);
    }

    return EXIT_SUCCESS;

}
