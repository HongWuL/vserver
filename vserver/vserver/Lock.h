/***
 * @brief 线程同步方法封装
*/

#ifndef __VS_LOCK_H
#define __VS_LOCK_H

#include <pthread.h>
#include <semaphore.h>
#include "Noncopyable.h"
#include <exception>
#include <iostream>

namespace vs {

/***
 * @brief 互斥量
*/
class Mutex : Noncopyable {
public:
    Mutex() {
        if(pthread_mutex_init(&m_mutex, NULL) != 0) {
            throw std::exception();
        }
    }
    ~Mutex() {
        if(pthread_mutex_destroy(&m_mutex) != 0) {
            throw std::exception();
        }
    }
    void lock() {
        if(pthread_mutex_lock(&m_mutex) != 0) {
            throw std::exception();
        }
        
    }
    void unlock() {
        if(pthread_mutex_unlock(&m_mutex) != 0) {
            throw std::exception();
        }
    }

private:
    friend class Condition;
    pthread_mutex_t m_mutex;
};

/***
 * @brief 条件变量
*/
class Condition : Noncopyable {
public:
    Condition() {
        if(pthread_cond_init(&m_cond, NULL) != 0) {
            throw std::exception();
        }
    }
    ~Condition() {
        if(pthread_cond_destroy(&m_cond) != 0) {
            throw std::exception();
        }
    }
    void wait(Mutex &mutex) {
        if(pthread_cond_wait(&m_cond, &mutex.m_mutex) != 0) {
            throw std::exception();
        }
    }

    bool waitForSeconds(Mutex &mutex, double seconds);

    void sigal() {
        if(pthread_cond_signal(&m_cond) != 0) {
            throw std::exception();
        }
    }
    void boradcast() {
        if(pthread_cond_broadcast(&m_cond) != 0) {
            throw std::exception();
        }
    }
private:
    pthread_cond_t m_cond;
};

/***
 * @brief 信号量
*/
class Semaphore : Noncopyable {
public:
    Semaphore(unsigned int n = 0) {
        if(sem_init(&m_sem, 0, n) != 0) {
            throw std::exception();
        } 
    }
    ~Semaphore() {
        if(sem_destroy(&m_sem) != 0) {
            throw std::exception();
        }
    }
    void wait() {
        if(sem_wait(&m_sem) != 0) {
            throw std::exception();
        }
    }
    void post() {
        if(sem_post(&m_sem) != 0) {
            throw std::exception();
        }
    }
private:
    sem_t m_sem;
};

/***
 * @brief 局部锁
*/
class MutexLockGuard : Noncopyable {
public:
    explicit MutexLockGuard(Mutex& mutex): m_mutex(mutex) {
        m_mutex.lock();
    }

    ~MutexLockGuard() {
        m_mutex.unlock();
    }

private:
    Mutex& m_mutex;
};

}


#endif