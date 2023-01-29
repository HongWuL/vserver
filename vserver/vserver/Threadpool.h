#ifndef __VS_THREADPOOL_H
#define __VS_THREADPOOL_H

#include <memory>
#include <thread>
#include <pthread.h>
#include <deque>
#include <vector>
#include <functional>
#include <assert.h>
#include "Lock.h"
#include "Noncopyable.h"

namespace vs {

class ThreadPool : Noncopyable {
public:
    typedef std::function<void()> Task;

    ThreadPool(size_t max_waits = 0);
    ~ThreadPool();

    int getNumThreads() { return m_threads.size(); };
    void setInitCallback(const Task& cb) { m_initCallback = cb; };
    void setCompleteCallback(const Task& cb) { m_completeCallback = cb; }
    void setMaxWaits(size_t max_waits) {m_max_waits = max_waits; };
    void start(int nmu_threads);
    void addTask(Task task);
    void stop();
    
private:
    void run();
    Task getTask();
    bool isFull();

private:
    bool m_running = false;
    Task m_initCallback;
    Task m_completeCallback;

    // 等待队列
    size_t m_max_waits;
    std::deque<Task> m_tasks;
    // 线程池
    std::vector<std::unique_ptr<std::thread>> m_threads;
    // 线程同步
    mutable Mutex m_mutex;
    Condition m_cond_notfull;
    Condition m_cond_notempty;
};

}

#endif


