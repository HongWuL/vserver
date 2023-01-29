#include "Lock.h"
#include <iostream>
#include <vector>
#include <assert.h>
#include <pthread.h>
/**
 * 生产者模型
*/
    
vs::Mutex mutex;
vs::Condition cond;

int n, cap;

void* producer(void* args) {
    while(1) {
        mutex.lock();
        while(n == cap) {
            cond.wait(mutex);
        }
        std::cout << "(";
        n ++;
        cond.boradcast();
        mutex.unlock();
    } 
}

void* consumer(void* args) {
    while(1) {
        mutex.lock();
        while(n == 0) {
            cond.wait(mutex);
        }
        std::cout << ")";
        n --;
        cond.boradcast();
        mutex.unlock();
    }
}

pthread_t create(void *(*f)(void*)) {
    int err;
    pthread_t tid;
    err =  pthread_create(&tid, NULL, f, NULL);
    assert(err == 0);
    return tid;
}
 
int main(int argc, char const *argv[])
{
    /* code */
    if(argc != 2) {
        std::cout << "Buffer size if required" << std::endl;
        return 0;
    }

    cap = atoi(argv[1]);


    std::vector<pthread_t> pool;

    for(int i = 0; i < 4; i ++) {
        pthread_t t1 = create(producer);
        pthread_t t2 = create(consumer);
        pool.push_back(t1);
        pool.push_back(t2);
    }

    for(auto &t: pool) pthread_join(t, NULL);
    
    return 0;
}
