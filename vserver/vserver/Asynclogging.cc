#include "Asynclogging.h"
#include <assert.h>

namespace vs {

AsyncLogging::AsyncLogging(const std::string& basename,
                           std::ostream& os,
                           long rollSize,
                           int flushInterval) :
    m_flushInterval(flushInterval),
    m_running(false),
    m_basename(basename),
    m_rollSize(rollSize),
    m_thread(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
    m_latch(1),
    m_mutex(),
    m_cond(),
    m_curBuffer(new Buffer),
    m_nextBuffer(new Buffer),
    m_buffers(),
    m_os(os) {

    m_curBuffer->bzero();
    m_nextBuffer->bzero();
    m_buffers.reserve(16);
}

void AsyncLogging::append(const char* logline, int len) {
    MutexLockGuard lock(m_mutex);
    if (m_curBuffer->avail() > len) {
        m_curBuffer->append(logline, len);
    }
    else {
        m_buffers.push_back(std::move(m_curBuffer));
        if (m_nextBuffer) {
            m_curBuffer = std::move(m_nextBuffer);
        }
        else {
            m_curBuffer.reset(new Buffer); // Rarely happens
        }
        m_curBuffer->append(logline, len);
        m_cond.sigal();
    }
}

void AsyncLogging::threadFunc() {
    assert(m_running);
    m_latch.countDown();

    //写线程的两个缓冲区
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);

    while(m_running) {

        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
        MutexLockGuard lock(m_mutex);
        if(m_buffers.empty()) {
            m_cond.waitForSeconds(m_mutex, m_flushInterval);
        }
        m_buffers.push_back(std::move(m_curBuffer));
        m_curBuffer = std::move(newBuffer1);
        buffersToWrite.swap(m_buffers);
        if (!m_nextBuffer) {
            m_nextBuffer = std::move(newBuffer2);
        }
        }

        assert(!buffersToWrite.empty());
        std::string output;
        for (const auto& buffer : buffersToWrite) {
            // FIXME: use unbuffered stdio FILE ? or use ::writev ?
            output.append(buffer->data(), buffer->length());
        }

        if (buffersToWrite.size() > 2) {
            buffersToWrite.resize(2);
        }

        if (!newBuffer1) {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2) {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }
        buffersToWrite.clear();
        m_os << output << std::flush;
    }
    m_os << std::flush;
}

}