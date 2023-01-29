#ifndef __VS_NET_BUFFER_H
#define __VS_NET_BUFFER_H

#include <stdlib.h>
#include <vector>
#include <assert.h>
#include <string>
#include "Noncopyable.h"

namespace vs {
namespace net {

class Buffer : Noncopyable {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;
    static const size_t kBufferSize = 65536;
    // static const size_t kBufferSize = 16;

    Buffer(size_t initialSize = kInitialSize):
        m_buffer(kCheapPrepend + initialSize),
        m_pRead(kCheapPrepend),
        m_pWrite(kCheapPrepend) {
    }
    
    size_t size() const {
        return m_buffer.size();
    }

    size_t capacity() const {
        return m_buffer.capacity();
    }

    char* begin() {
        return &*m_buffer.begin(); 
    }

    const char* begin() const {
        return &*m_buffer.begin(); 
    }

    size_t readableBytes() const {
        return m_pWrite - m_pRead; 
    }

    size_t writableBytes() const {
        return m_buffer.size() - m_pWrite;
    }

    size_t prependableBytes() const {
        return m_pRead; 
    }

    const char* peek() const {
        return begin() + m_pRead; 
    }

    char* beginWrite() {
        return begin() + m_pWrite; 
    }

    const char* beginWrite() const { 
        return begin() + m_pWrite; 
    }

    /***
     * @brief 缓冲区指针重新初始化
    */
    void retrieveAll() {
        m_pRead = kCheapPrepend;
        m_pWrite = kCheapPrepend;
    }

    /***
     * @brief 读出后重新设置read起点
     * @param len 读出的字节数
    */
    void retrieve(size_t len) {
        assert(len <= readableBytes());
        if (len < readableBytes()) {
            m_pRead += len;
        }
        else {
            retrieveAll();
        }
    }

    /***
     * @brief 读出所有可读字符后重新设置read起点
     * @return Read缓冲区内所有字符组成的string
    */
    std::string retrieveAllAsString() {
        return retrieveAsString(readableBytes());
    }

    /***
     * @brief 读出后len个字符并重新设置read起点
     * @param len 读出的字节数
     * @return 前len个字符组成的string
    */
    std::string retrieveAsString(size_t len) {
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    /***
     * @brief 扩充缓冲空间
     * @param len 即将写入的字节数
    */
    void makeSpace(size_t len) {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
            m_buffer.resize(m_pWrite + len);
        }
        else {
            // 已有空闲空间足够
            // move readable data to the front, make space inside buffer
            assert(kCheapPrepend < m_pRead);
            size_t readable = readableBytes();
            std::copy(begin()+m_pRead,
                begin()+m_pWrite,
                begin()+kCheapPrepend);
            m_pRead = kCheapPrepend;
            m_pWrite = m_pRead + readable;
            assert(readable == readableBytes());
        }
    }

    /***
     * @brief 检查和扩充缓冲空间
     * @param len 即将写入的字节数
    */
    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    /***
     * @brief 写入data
     * @param data 即将写入的数据
     * @param len 即将写入的字节数
    */
    void append(const char*  data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data+len, beginWrite());
        m_pWrite += len;
    }

    void append(std::string data) {
        append(data.c_str(), data.size());        
    }

    /***
     * @brief 从fd中直接读数据
     * @param fd 文件描述符
     * @param savedErrno 发生的错误
    */
    ssize_t readOnece(int fd);
    ssize_t readAll(int fd);
    ssize_t asyncReadOnece(int fd);
    ssize_t asyncReadAll(int fd);

private:
    std::vector<char> m_buffer;
    size_t m_pRead;
    size_t m_pWrite;
};
    
} 

}


#endif