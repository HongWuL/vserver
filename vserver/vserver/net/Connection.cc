#include "Connection.h"
#include "Eventloop.h"
#include "Socket.h"
#include "Address.h"
#include "Channel.h"
#include "SocketUtils.h"
#include <assert.h>
#include "Log.h"

namespace vs {
namespace net {

TcpConnection::TcpConnection(EventLoop* loop,
                const std::string name,
                int sockfd,
                const SocketAddress& localAddr,
                const SocketAddress& remoteAddr,
                time_type delay):
    m_loop(loop),
    m_name(name),
    m_localAddr(localAddr),
    m_remoteAddr(remoteAddr),
    m_socket(new Socket(sockfd)),
    m_channel(new Channel(m_loop, sockfd)),
    m_state(kConnecting),
    m_delay(delay),
    m_timerId(-1) {

    m_channel->setReadCallback(
        std::bind(&TcpConnection::handleRead, this));
    m_channel->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this));
    m_channel->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this));
    m_channel->setErrorCallback(
        std::bind(&TcpConnection::handleError, this));

    //ET
    m_socket->setNonBlocking();
}

TcpConnection::~TcpConnection() {
    GLOG_DEBUG << "~TcpConnection()";
}

// called when TcpServer accepts a new connection
void TcpConnection::connectEstablished() {
    GLOG_DEBUG << "TcpConnection::connectEstablished()";
    m_loop->assertInLoopThread();
    assert(m_state == kConnecting);
    setState(kConnected);
    m_channel->enableReading();
    m_connectionCallback(shared_from_this());

    if(m_delay > 0) {
        forceCloseAfter(m_delay);
    }
}

// called when TcpServer has removed me from its map
void TcpConnection::connectDestroyed() {
    GLOG_DEBUG << "TcpConnection::connectDestroyed()";
    m_loop->assertInLoopThread();
    if(m_state == kConnected) {
        setState(kDisconnected);
        m_channel->disableAll();
        m_connectionCallback(shared_from_this());
    }
    m_channel->remove();
}

void TcpConnection::send(const std::string& message) {
    if(m_state == kConnected) {
        if(m_loop->isInLoopThread()) {
            sendInLoop(message);
        }
        else {
            m_loop->runInLoop(
                std::bind(&TcpConnection::sendInLoop, shared_from_this(), message)
            );
        }
    }
}

void TcpConnection::send(const char* message, int len) {
    std::string msg(message, message + len);
    send(msg);
}

void TcpConnection::sendInLoop(const std::string& message) {
    m_loop->assertInLoopThread();
    ssize_t nwrote = 0;
    if(!m_channel->isWritable() && m_outputBuffer.readableBytes() == 0) {
        //直接写
        GLOG_INFO << "TcpConnection::sendInLoop() to wirte";
        nwrote = ::write(m_channel->fd(), message.data(), message.size());
        GLOG_INFO << "Sucessfully writen " << nwrote << " bytes";
        if(nwrote >= 0) {
            if(nwrote < message.size()) {
                GLOG_INFO << "I am going to write more data" << std::endl;
            }
        }
        else {
            nwrote = 0;
            GLOG_ERROR << "write error. " << strerror(errno) << std::endl;
        }
    }
    assert(nwrote >= 0);
    if(nwrote < message.size()) {
        m_outputBuffer.append(message.data() + nwrote, message.size() - nwrote);
        if(!m_channel->isWritable()) {
            m_channel->enableWriting();
        }
    }
    // 全部交给epoll管理
    // m_outputBuffer.append(message.data(), message.size());
    // m_channel->enableWriting();
}

void TcpConnection::shutdown() {
    // FIXME: use compare and swap
    if (m_state == kConnected) {
        setState(kDisconnecting);
        // FIXME: shared_from_this()?
        m_loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
    }
}

void TcpConnection::shutdownInLoop() {
    m_loop->assertInLoopThread();
    if (!m_channel->isWritable()) {
        m_socket->shutdownWrite();
    }
}

void TcpConnection::handleRead() {
    m_loop->assertInLoopThread();
    GLOG_DEBUG << "TcpConnection::handleRead()"<<std::endl;
    
    ssize_t n = m_inputBuffer.readOnece(m_channel->fd());
    // ssize_t n = m_inputBuffer.readAll(m_channel->fd());

    if(n > 0) {
        updateTimer();
        m_messageCallback(shared_from_this(), &m_inputBuffer);
    }
    else if (n == 0) {
        handleClose();
    } 
    else {
        handleError();
    }
}

void TcpConnection::forceClose() {
    if (m_state == kConnected || m_state == kDisconnecting) {
        setState(kDisconnecting);
        m_loop->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseInLoop() {
    m_loop->assertInLoopThread();
    if (m_state == kConnected || m_state == kDisconnecting) {
        handleClose();
    }
}

void TcpConnection::forceCloseAfter(time_type ms) {
    if (m_state == kConnected || m_state == kDisconnecting) {
        setState(kDisconnecting);
        m_timerId = m_loop->runAfter(
            ms, makeWeakCallback(shared_from_this(), &TcpConnection::forceClose)
            // std::bind(&TcpConnection::forceClose, shared_from_this())
        );
        //this和shared_from_this都不合适
    }
}

void TcpConnection::updateTimer() {
    // safe ?
    GLOG_DEBUG << "TcpConnection::updateTimer()" << std::endl;
    if((int)m_timerId != -1) {
        m_loop->cancel(m_timerId);
        forceCloseAfter(m_delay);
    }
}

void TcpConnection::handleWrite() {
    GLOG_INFO << "TcpConnection::handleWrite()";
    m_loop->assertInLoopThread();
    if(m_channel->isWritable()) {
        ssize_t n = ::write(m_channel->fd(),
                            m_outputBuffer.peek(),
                            m_outputBuffer.readableBytes());
        if(n > 0) {
            m_outputBuffer.retrieve(n);
            if (m_outputBuffer.readableBytes() == 0) {
                //全部读完了
                m_channel->disableWriting();
                GLOG_INFO << "Write complete!";
            }
            if(m_state == kDisconnecting) {
                shutdownInLoop();
            }
        }
    }
    else {
        GLOG_INFO << "Connection fd = " << m_channel->fd()
              << " is down, no more writing";
    }
}

void TcpConnection::handleClose() {
    GLOG_INFO << "TcpConnection::handleClose()";
    assert(m_state == kConnected || m_state == kDisconnecting);
    m_channel->disableAll();
    m_closeCallback(shared_from_this());
}

void TcpConnection::handleError() {
    GLOG_INFO << "TcpConnection::handleError()"<<std::endl;
}


}
}

