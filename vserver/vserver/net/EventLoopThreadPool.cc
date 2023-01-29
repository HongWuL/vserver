#include "EventLoopThreadPool.h"
#include <assert.h>
#include "Log.h"

namespace vs {
namespace net {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop,
                                        const std::string& name) :
    m_name(name),
    m_baseLoop(baseLoop),
    m_started(false),
    m_numThreads(0),
    m_next(0) {
}

EventLoopThreadPool::~EventLoopThreadPool() {
}


void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
    assert(!m_started);
    m_baseLoop->assertInLoopThread();
    m_started = true;

    for (int i = 0; i < m_numThreads; ++i) {
        char buf[m_name.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", m_name.c_str(), i);
        EventLoopThread* t = new EventLoopThread(buf, cb);
        m_threads.push_back(std::unique_ptr<EventLoopThread>(t));
        EventLoop* loop = t->startLoop();
        m_loops.push_back(loop);
        
        m_loopInfo[loop] = {0, i};
        m_connectionCount.insert({0, i});
    }

    if (m_numThreads == 0 && cb) {
        cb(m_baseLoop);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {

    m_baseLoop->assertInLoopThread();
    assert(m_started);
    EventLoop* loop = m_baseLoop;

    if (!m_loops.empty()) {
        // round-robin
        loop = m_loops[m_next];
        ++m_next;
        if (implicit_cast<size_t>(m_next) >= m_loops.size()) {
            m_next = 0;
        }
    }
    return loop;
}

void EventLoopThreadPool::update(EventLoop* loop, int newCnt) {
    m_baseLoop->assertInLoopThread();
    assert(m_started);
    
    assert(m_loopInfo.count(loop));
    auto info = m_loopInfo[loop];

    int cnt = info.first, idx = info.second; 

    auto it = m_connectionCount.find({cnt, idx});
    assert(it != m_connectionCount.end());

    m_connectionCount.erase(it);
    m_connectionCount.insert({newCnt, idx});
    m_loopInfo[loop] = {newCnt, idx};
}

void EventLoopThreadPool::removeLoopConnection(EventLoop* loop) {
    GLOG_INFO << "EventLoopThreadPool::removeLoopConnection()";
    m_baseLoop->assertInLoopThread();
    assert(m_started);

    if (!m_connectionCount.empty()) {
        // min-connection
        // auto it = m_connectionCount.begin();
        auto info = m_loopInfo[loop];
        int cnt = info.first, idx = info.second; 

        update(loop, cnt - 1);
    }
}

EventLoop* EventLoopThreadPool::getNextLoopMinConnection() {
    GLOG_INFO << "EventLoopThreadPool::getNextLoopMinConnection()";

    m_baseLoop->assertInLoopThread();
    assert(m_started);
    EventLoop* loop = m_baseLoop;

    if (!m_connectionCount.empty()) {
        // min-connection
        auto it = m_connectionCount.begin();
        int cnt = it->first, idx = it->second;
        loop = m_loops[idx];
        update(loop, cnt + 1);
    }
    return loop;
}

EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode) {

    m_baseLoop->assertInLoopThread();
    EventLoop* loop = m_baseLoop;

    if (!m_loops.empty()) {
        loop = m_loops[hashCode % m_loops.size()];
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {

    m_baseLoop->assertInLoopThread();
    assert(m_started);
    if (m_loops.empty()) {
        return std::vector<EventLoop*>(1, m_baseLoop);
    }
    else {
        return m_loops;
    }
}


}
}