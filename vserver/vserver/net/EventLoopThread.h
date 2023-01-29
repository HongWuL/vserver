#ifndef __VS_NET_EVENTLOOPTHREAD_H
#define __VS_NET_EVENTLOOPTHREAD_H

#include "Noncopyable.h"
#include "Eventloop.h"
#include "Lock.h"
#include "Thread.h"

namespace vs {
namespace net {

class EventLoopThread : Noncopyable {
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    EventLoopThread(const std::string& name = std::string(),
                    const ThreadInitCallback& cb = ThreadInitCallback());
    ~EventLoopThread();
    EventLoop* startLoop();


private:
    void createLoopInThread();
    EventLoop* m_loop;
    bool m_exiting;
    Thread m_thread;
    Mutex m_mutex;
    Condition m_cond;
    ThreadInitCallback m_initCallback;
};

}
}

#endif
