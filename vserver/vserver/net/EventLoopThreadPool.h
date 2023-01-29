#ifndef __VS_NET_EVENTLOOPTHREADPOOL_H
#define __VS_NET_EVENTLOOPTHREADPOOL_H

#include "Noncopyable.h"
#include <functional>
#include "Eventloop.h"
#include "EventLoopThread.h"
#include <memory>
#include <map>

namespace vs {
namespace net {
class EventLoopThreadPool : Noncopyable {
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& name);
    ~EventLoopThreadPool();

    void start(const ThreadInitCallback& cb = ThreadInitCallback()); 
    EventLoop* getNextLoop();
    EventLoop* getNextLoopMinConnection();
    void removeLoopConnection(EventLoop* loop);

    EventLoop* getLoopForHash(size_t hashCode);
    std::vector<EventLoop*> getAllLoops();

    void setThreadNum(int numThreads) { m_numThreads = numThreads; }
    bool started() const { return m_started; }
    const std::string& name() const { return m_name; }
    
private:
    void update(EventLoop* loop, int newCnt);

    EventLoop* m_baseLoop;
    std::string m_name;
    bool m_started;
    int m_numThreads;
    int m_next;
    std::vector<std::unique_ptr<EventLoopThread>> m_threads;
    std::vector<EventLoop*> m_loops;

    std::set<std::pair<int, int>> m_connectionCount;   //conut, idx
    std::unordered_map<EventLoop*, std::pair<int,int>> m_loopInfo;    //loop: cnt, idx

};

}
} // namespace vs


#endif