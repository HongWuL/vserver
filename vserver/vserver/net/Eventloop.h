#ifndef __VS_EVENTLOOP_H
#define __VS_EVENTLOOP_H

#include <memory>
#include <vector>
#include <functional>
#include "Noncopyable.h"
#include "Utils.h"
#include "Lock.h"
#include "Timer.h"
#include "Channel.h"
#include "Poller.h"

namespace vs {
namespace net {

class Channel;
class Poller;
class TimerManager;
class Timer;

class EventLoop : Noncopyable {
public:
    typedef std::function<void()> Func;
    typedef uint64_t time_type;

    EventLoop();
    ~EventLoop();  // force out-line dtor, for std::unique_ptr members.

    void loop();
    void quit();

    void updateChannel(Channel* channel);

    /// Runs callback immediately in the loop thread.
    /// It wakes up the loop, and run the cb.
    /// If in the same loop thread, cb is run within the function.
    /// Safe to call from other threads.
    void runInLoop(Func cb);
    /// Queues callback in the loop thread.
    /// Runs after finish pooling.
    /// Safe to call from other threads.
    void queueInLoop(Func cb);

    Timer::id_type runAt(time_type time, Func cb);
    Timer::id_type runAfter(time_type delay, Func cb);
    void cancel(Timer::id_type timer_id);

    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const { return m_threadId == GetThreadId(); }

    void removeChannel(Channel* channel);

    // void addConnection() { m_numConns.increment(); }
    // void removeConnection() { m_numConns.decrement(); }
    // int getNumConnection() { return m_numConns.get(); }

private:
    typedef std::vector<Channel*> ChannelList;
    static const int kPollTimeMs = 10000;

    void abortNotInLoopThread();
    void handleRead();  // waked up
    void doPendingFunctors();
    void wakeup();

    const pid_t m_threadId;
    bool m_looping;
    bool m_quit;

    std::unique_ptr<Poller> m_poller;
    ChannelList m_activeChannels;
    
    int m_wakeupfd;
    std::unique_ptr<Channel> m_wakeupChannel;

    // 线程安全相关
    Mutex m_mutex;
    std::vector<Func> m_pendingFuncs;
    bool m_callingPendingFuncs;

    //定时器相关
    std::unique_ptr<TimerManager> m_timerManager;
    
    // //记录连接数
    // AtomicIntegerT<int> m_numConns;

};

}
}

#endif