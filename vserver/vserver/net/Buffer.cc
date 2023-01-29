#include "Buffer.h"
#include "SocketUtils.h"
#include "Log.h"

namespace vs {
namespace net {

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;
const size_t Buffer::kBufferSize;

ssize_t Buffer::readOnece(int fd) {

    // char extrabuf[2];
    char extrabuf[kBufferSize];

    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + m_pWrite;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    // when there is enough space in this buffer, don't read into extrabuf.
    // when extrabuf is used, we read 128k-1 bytes at most.
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = socket::Readv(fd, vec, iovcnt);

    if (n <= writable) {
        // 已有缓冲区足以承受，直接写
        m_pWrite += n;
    }
    else {
        // 已有缓冲区不够，将剩余部分append到缓冲区后
        m_pWrite = m_buffer.size();
        append(extrabuf, n - writable);
    }

    return n;
}

ssize_t Buffer::readAll(int fd) {
    //EL模式下，需要一次性全部读出
    char buf[kBufferSize];
    int cnt = 0;
    while(1) {
        bzero(buf, kBufferSize);
        int n = ::read(fd, buf, kBufferSize);
        GLOG_INFO << "read number:" << n;
        if(n < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                // read complete
                break;
            }
            else {
                GLOG_ERROR << "read error." << strerror(errno);
                abort();
            }
        }
        else if(n == 0) {
            break;
        }
        else {
            append(buf, n);
        }
    }
    return readableBytes();
}

}
}