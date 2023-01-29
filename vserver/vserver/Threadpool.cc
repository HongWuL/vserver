#include "Threadpool.h"
#include "Utils.h"

namespace vs {

ThreadPool::ThreadPool(size_t max_waits):
    m_max_waits(m_max_waits) {
}

ThreadPool::~ThreadPool() {
    if(m_running) {
        stop();
    }
}

void ThreadPool::start(int num_threads) {
    m_running = true;
    m_threads.reserve(num_threads);
    for(int i = 0; i < num_threads; i ++) {
        m_threads.emplace_back(new std::thread(std::bind(&ThreadPool::run, this)));
    }
    if(num_threads == 0 && m_initCallback) {
        m_initCallback();
    }
}


void ThreadPool::stop() {
    {
        MutexLockGuard lock(m_mutex);
        m_running = false;
        m_cond_notempty.boradcast();
        m_cond_notfull.boradcast();
    }
    for(auto& thr: m_threads) {
        thr->join();
    }
}

/***
 * @brief 生产者队列，添加任务到队列
*/
void ThreadPool::addTask(ThreadPool::Task task) {
    if(m_threads.empty()) {
        // 线程池数量为0，则顺次执行程序
        task();
    }
    else {
        MutexLockGuard lock(m_mutex);
        while(isFull() && m_running) {
            m_cond_notfull.wait(m_mutex);
        }
        if(m_running) {
            // 若阻塞队列为0，则认为阻塞队列无限大
            m_tasks.push_back(std::move(task));
            m_cond_notempty.sigal();
        }
    }
}

/***
 * @brief 消费者进程，从任务队列中获取task, 任务队列为空则阻塞
*/
ThreadPool::Task ThreadPool::getTask() {
    MutexLockGuard lock(m_mutex);
    while(m_tasks.empty() && m_running) {
        m_cond_notempty.wait(m_mutex);
    }
    Task task;

    if(!m_tasks.empty()) {
        task = m_tasks.front();
        m_tasks.pop_front();
        if(m_max_waits > 0) {
            m_cond_notfull.sigal();
        }
    }
    return task;
}

void ThreadPool::run() {
    if(m_initCallback) {
        m_initCallback();
    }
    while(m_running) {
        Task task(getTask());
        if(task) {
            task();
        }
    }
}

bool ThreadPool::isFull() {
    return m_max_waits > 0 && m_tasks.size() >= m_max_waits;
}
    
}