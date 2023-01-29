#ifndef __VS_ASYNCLOGGING_H
#define __VS_ASYNCLOGGING_H

#include <vector>
#include <string.h>
#include <memory>
#include <atomic>
#include "Utils.h"
#include "Noncopyable.h"
#include "Thread.h"
#include "Latch.h"
#include "Lock.h"


namespace vs {

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000*1000;

template<int SIZE>
class FixedBuffer: Noncopyable {
public:
    FixedBuffer():
        m_cur(m_data) {
    }

    int avail() const { 
        return static_cast<int>(end() - m_cur); 
    }
    const char* data() const { return m_data; }
    int length() const { return static_cast<int>(m_cur - m_data); }
    char* current() { return m_cur; }
    void add(size_t len) { m_cur += len; }

    void reset() { m_cur = m_data; }
    void bzero() { memset(m_data, 0, sizeof m_data); }

    void append(const char* buf, size_t len) {
        if (implicit_cast<size_t>(avail()) > len) {
            memcpy(m_cur, buf, len);
            m_cur += len;
        }
    }

    std::string toString() const { return std::string(m_data, length()); }

private:
    const char* end() const { return m_data + sizeof m_data; }

    char m_data[SIZE];
    char* m_cur;
};

class AsyncLogging: Noncopyable {
public:
    AsyncLogging(const std::string& basename,
            std::ostream& os,
            long rollSize,
            int flushInterval = 3);

    ~AsyncLogging() {
        std::cout << "~AsyncLogging()" << std::endl;
        if(m_running) {
            stop();
        }
    }

    void append(const char* logline, int len);

    void start() {
        m_running = true;
        m_thread.start();
        m_latch.wait();
    }

    void stop() {
        std::cout << "stop()" << std::endl;
        m_running = false;
        m_cond.sigal();
        m_thread.join();
    }
private:

    void threadFunc();

    typedef FixedBuffer<kLargeBuffer> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;

    const int m_flushInterval;
    std::atomic<bool> m_running;
    const std::string m_basename;
    const long m_rollSize;

    // 多线程,线程同步
    Thread m_thread;
    CountDownLatch m_latch;
    Mutex m_mutex;
    Condition m_cond;

    //缓冲区
    BufferPtr m_curBuffer;
    BufferPtr m_nextBuffer;
    BufferVector m_buffers;

    //
    std::ostream& m_os;
};

}

#endif

