#include "Thread.h"
#include <assert.h>
#include <thread>
#include <iostream>
#include <string.h>
#include "Utils.h"
#include "Log.h"

namespace vs {

struct ThreadData {
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc m_func;
    std::string m_name;
    pid_t* m_tid;
    CountDownLatch* m_latch;

    ThreadData(ThreadFunc func,
            const std::string& name,
            pid_t* tid,
            CountDownLatch* latch) :
        m_func(std::move(func)),
        m_name(name),
        m_tid(tid),
        m_latch(latch) {
    }
    void runInThread() {
        *m_tid = GetThreadId();
        m_tid = NULL;
        m_latch->countDown();
        m_latch = NULL;
        m_func();
    }
};

void* startThread(void* obj) {
    GLOG_INFO << "Thread::start()";
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return NULL;
}

Thread::Thread(ThreadFunc func, const std::string& name):
    m_func(std::move(func)),
    m_name(name),
    m_started(false),
    m_joined(false),
    m_pthreadId(0),
    m_tid(0),
    m_lacth(1) {
}

Thread::~Thread() {
    if(m_started && !m_joined) {
        pthread_detach(m_pthreadId);
    }
}

void Thread::start() {
    assert(!m_started);
    ThreadData *data = new ThreadData(m_func, m_name, &m_tid, &m_lacth);
    m_started = true;
    int n = pthread_create(&m_pthreadId, NULL, &startThread, data);
    if( n != 0) {
        GLOG_ERROR << "pthread_create error. " << strerror(errno);
        m_started = false;
    }
    else {
        m_lacth.wait();
    }
}

void Thread::run() {
    if(m_started && m_func) {
        m_func();
    }
}

void Thread::join() {
    assert(m_started);
    assert(!m_joined);
    m_joined = true;
    int n = pthread_join(m_pthreadId, NULL);
    if( n != 0) {
        GLOG_ERROR << strerror(errno);
        m_joined = false;
    }
}


}