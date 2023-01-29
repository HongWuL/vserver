#ifndef __VS_THREAD_H
#define __VS_THREAD_H

#include <pthread.h>
#include <functional>
#include "Noncopyable.h"
#include "Latch.h"

namespace vs {

class Thread: Noncopyable {
public:
    typedef std::function<void ()> ThreadFunc;

    Thread(ThreadFunc func, const std::string &name);
    ~Thread();

    void start();
    void join(); 
    void run();

    const std::string& name() const { return m_name; }
    pid_t tid() const { return m_tid; }
    bool started() const { return m_started; }

private:
    pthread_t   m_pthreadId;
    pid_t       m_tid;
    ThreadFunc  m_func;
    std::string m_name;
    bool        m_started;
    bool        m_joined;
    CountDownLatch m_lacth;
};

}

#endif