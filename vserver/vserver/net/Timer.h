#ifndef __VS_NET_TIMER_H
#define __VS_NET_TIMER_H

#include "Noncopyable.h"
#include <functional>
#include <iostream>
#include <set>
#include <unordered_map>
#include <memory>
#include <vector>
#include <algorithm>
#include <queue>
#include "Channel.h"
#include "SocketUtils.h"
#include "Utils.h"

namespace vs {
namespace net {

class TimerManager;

class Timer : Noncopyable {
friend TimerManager;
public:
    typedef std::function<void()> TimerCallback;
    typedef std::shared_ptr<Timer> ptr;
    typedef uint64_t id_type;

    Timer(TimerCallback cb, time_type expire, time_type interval = 0);

    void run() const {
        m_cb();
    }

    bool restart(time_type now); //调整下一次调用时间
private:
    struct Comparator {
        bool operator() (const Timer::ptr &lhs, const Timer::ptr &rhs) {
            if(lhs->m_expire < rhs->m_expire) return true;
            else if(lhs->m_expire == rhs->m_expire) {
                return lhs->m_id < rhs->m_id;
            }   
            return false;
        }
    };
private:
    TimerCallback m_cb;
    
    time_type m_expire;   //调用回调下一次的时间
    time_type m_interval; //调用回调间隔
    bool m_repeat;      //是否重复调用回调, true when interval > 0
    const id_type m_id;    //每个类的唯一标识符

    static AtomicIntegerT<id_type> count;
};


class TimerManager : Noncopyable {
public:
    TimerManager(EventLoop* loop);
    ~TimerManager();

    Timer::id_type addTimer(Timer::TimerCallback cb, time_type expire, time_type interval = 0);

    void cancel(Timer::id_type id);

private:
    typedef std::vector<Timer::ptr> TimerList;
    void handleRead();
    void fillExpired(time_type now, TimerList &expired_timers);
    void resetTimerfd(time_type expire);
    void closeClock();  //没有定时器需要等待
    void addTimer(Timer::ptr timer);
    //线程安全
    void addTimerInloop(Timer::ptr timer);
    void cancelInloop(Timer::id_type timer);

    EventLoop* m_loop;
    const int m_timerfd;
    Channel m_timerfdChannel;
    
    std::set<Timer::ptr, Timer::Comparator> m_timers;
    std::unordered_map<Timer::id_type, Timer::ptr> m_table;
    TimerList m_expired_timers;
};

}

}


#endif