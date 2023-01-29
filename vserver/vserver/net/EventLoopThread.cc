#include "EventLoopThread.h"
#include <assert.h>

namespace vs {
namespace net{

EventLoopThread::EventLoopThread(const std::string& name,
                    const ThreadInitCallback& cb):
    m_loop(NULL),
    m_exiting(false),
    m_thread(std::bind(&EventLoopThread::createLoopInThread, this), name),
    m_initCallback(cb) {
}

EventLoopThread::~EventLoopThread() {
    std::cout << "EventLoopThread::~EventLoopThread()" << std::endl;
    m_exiting = true;
    if (m_loop != NULL)  {
        m_loop->quit();
        m_thread.join(); 
    }
}

EventLoop* EventLoopThread::startLoop() {

    assert(!m_thread.started());
    m_thread.start();

    EventLoop* loop = NULL;
    {
    MutexLockGuard lock(m_mutex);
    while (m_loop == NULL)
    {
        m_cond.wait(m_mutex);
    }
    loop = m_loop;
    }
    return loop;
}


void EventLoopThread::createLoopInThread() {

    EventLoop loop; //创建loop

    if (m_initCallback) {
        m_initCallback(&loop);
    }

    {
    MutexLockGuard lock(m_mutex);
    m_loop = &loop;
    m_cond.sigal();
    }

    loop.loop();    //start a new loop
    MutexLockGuard lock(m_mutex);
    m_loop = NULL;
}



}
}