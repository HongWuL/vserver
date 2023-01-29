
#include "Timer.h"
#include <sys/timerfd.h>
#include <assert.h>
#include <Eventloop.h>

namespace vs {
namespace net {

int createTimerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                    TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        std::cout << "Failed in timerfd_create" << std::endl;
    }
    return timerfd;
}

AtomicIntegerT<uint64_t> Timer::count;

Timer::Timer(TimerCallback cb, time_type expire, time_type interval):
    m_cb(cb),
    m_expire(expire),
    m_interval(interval),
    m_repeat(interval > 0),
    m_id(count.incrementAndGet()) {
}

bool Timer::restart(time_type now) {
    if(m_repeat) {
        m_expire = now + m_interval;
        return true;
    }
    std::cout << "Timer is not repeatable!" << std::endl;
    return false;
}

TimerManager::TimerManager(EventLoop* loop) :
    m_loop(loop),
    m_timerfd(createTimerfd()),
    m_timerfdChannel(loop, m_timerfd) {
    
    m_timerfdChannel.setReadCallback(
        std::bind(&TimerManager::handleRead, this));
    m_timerfdChannel.enableReading();
}

TimerManager::~TimerManager() {
    std::cout << "~TimerManager()" << std::endl;
    m_timerfdChannel.disableAll();
    m_timerfdChannel.remove();
    socket::Close(m_timerfd);
}

Timer::id_type TimerManager::addTimer(Timer::TimerCallback cb, time_type expire, time_type interval) {
    Timer::ptr timer(new Timer(std::move(cb), expire, interval));
    std::cout << "add timer " << timer->m_id << ", "<< expire - GetCurrentTimeMS() << ", timers.size() = " << m_timers.size() << std::endl;
    addTimer(timer);
    return timer->m_id;
}

void TimerManager::addTimer(Timer::ptr timer) {
    m_loop->runInLoop(std::bind(
        &TimerManager::addTimerInloop, this, timer
    ));
}

void TimerManager::addTimerInloop(Timer::ptr timer) {
    bool isAddHead = false; 
    if( m_timers.empty() || timer->m_expire < (*m_timers.begin())->m_expire) {
        isAddHead = true;
    }

    if(isAddHead) {
        resetTimerfd(timer->m_expire);
    }
    m_timers.insert(timer);
    m_table[timer->m_id] = timer;
    // std::cout << "add timer " << timer->m_id << ", " << m_timers.size() << std::endl;
}

void TimerManager::cancel(Timer::id_type id) {
    m_loop->runInLoop(std::bind(
        &TimerManager::cancelInloop, this, id
    ));
}

void TimerManager::cancelInloop(Timer::id_type id) {
    std::cout << "cancel " << id << std::endl;
    if(m_table.count(id) < 1) {
        std::cout << "Timer " << id << " doesn't exsit!" << std::endl;
        return;
    }

    Timer::ptr target = m_table[id];
    auto it = m_timers.find(target);
    assert(it != m_timers.end());
    bool ishead = (it == m_timers.begin());
    m_timers.erase(it);
    m_table.erase(id);
    assert(m_timers.size() == m_table.size());
    
    //是否在头部
    if(ishead) {
        if(m_timers.size() == 0) {
            closeClock();
        }
        else {
            time_type newExpire = (*m_timers.begin())->m_expire;
            resetTimerfd(newExpire);
        }
    }
}

void TimerManager::handleRead() {
    std::cout << "handle read" << std::endl;
    // 读出事件，避免poller反复提醒
    uint64_t tmp;   
    socket::Readn(m_timerfd, &tmp, sizeof tmp);
    // 获取超时Timer
    time_type now = GetCurrentTimeMS();
    m_expired_timers.clear();
    fillExpired(now, m_expired_timers);
    // std::cout << "m_expired_timers: "<< m_expired_timers.size() << std::endl;
    for(Timer::ptr timer: m_expired_timers) {
        timer->run();
    }
}

void TimerManager::fillExpired(time_type now, TimerList &expired_timers) {
    // 获取超时Timer
    // std::multiset<Timer::ptr>::iterator it;
    // std::cout << (*m_timers.begin())->m_expire << " : " << now << std::endl;
    // assert((*m_timers.begin())->m_expire <= now);    //错误唤醒

    Timer::ptr nowTimer(new Timer(NULL, now));
    auto it = m_timers.upper_bound(nowTimer);

    // assert(it != m_timers.begin());
    if(it == m_timers.begin()) {
        expired_timers.push_back(*it);
        m_table.erase((*it)->m_id);
        m_timers.erase(it);
    } 
    
    else {
        std::copy(m_timers.begin(), it, back_inserter(expired_timers));
        for(auto i = m_timers.begin(); i != it; ++i) {
            m_table.erase((*i)->m_id);
        }
        m_timers.erase(m_timers.begin(), it);
    }
    
    //重设timerfd定时
    if(m_timers.empty()) {
        closeClock();
    }
    else {
        resetTimerfd((*m_timers.begin())->m_expire);
    }
}

void TimerManager::resetTimerfd(time_type expire) {
    // std::cout << "reset timerfd " << std::endl;
    struct itimerspec spec;
    memset(&spec, 0, sizeof spec);

    //计算多少时间后发送可读事件    
    int64_t interval = TimeSub(expire, GetCurrentTimeMS()); //MS

    if(interval < 100) {
        interval = 100;
    }
    spec.it_value.tv_sec = interval / 1000;
    spec.it_value.tv_nsec = (interval % (1000)) * 1000;

    int ret = ::timerfd_settime(m_timerfd, 0, &spec, NULL);
    if(ret < 0) {
        std::cout << "resetTimerfd(): timerfd_settime() error" <<std::endl;
    }
}

void TimerManager::closeClock() {
    // std::cout << "close timerfd " << std::endl;
    struct itimerspec spec;
    struct itimerspec sold;
    memset(&spec, 0, sizeof spec);
    memset(&sold, 0, sizeof sold);
    int ret = ::timerfd_settime(m_timerfd, 0, &spec, &sold);
    if(ret < 0) {
        std::cout << "closeClock(): timerfd_settime() error. " << strerror(errno)<<std::endl;
    }
}

}

}

