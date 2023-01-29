#include <iostream>
#include <unistd.h>
#include <functional>
#include "vserver/Threadpool.h"
#include "vserver/Utils.h"
#include "vserver/Latch.h"

void print() {
    std::cout << "tid = " << vs::GetThreadId() << std::endl;
}

void printString(const std::string& str) {
    std::cout << str << std::endl;
    usleep(1000*1000);
}

void test(int maxSize, int poolSize) {
    vs::ThreadPool pool(maxSize);
    pool.setInitCallback(print);
    pool.start(poolSize);

    for (int i = 0; i < 100; ++i) {
        char buf[32];
        snprintf(buf, sizeof buf, "task %d", i);
        pool.addTask(std::bind(printString, std::string(buf)));
    }

    vs::CountDownLatch latch(1);
    pool.addTask(std::bind(&vs::CountDownLatch::countDown, &latch));
    latch.wait();
    
    pool.stop();

    // run after stop
    pool.addTask(print);
}

int main(int argc, char const *argv[]) {
    /* code */
    if(argc != 3) {
        std::cout << "maxSize and poolSize are needed!" << std::endl;
        return -1;
    }
    test(atoi(argv[1]), atoi(argv[2]));
    return 0;
}
