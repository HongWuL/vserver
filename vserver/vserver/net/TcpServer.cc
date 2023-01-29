#include "TcpServer.h"
#include "Acceptor.h"
#include "Address.h"
#include "Eventloop.h"
#include "Connection.h"
#include "EventLoopThreadPool.h"
#include <assert.h>
#include "Log.h"

namespace vs {
namespace net {

TcpServer::TcpServer(
    EventLoop* loop, 
    const std::string& name, 
    const SocketAddress& listenAddr, 
    bool reUsePort,
    time_type ms):
    m_loop(loop),
    m_name(name),
    m_connId(0),
    m_delay(ms), 
    m_acceptor(new Acceptor(loop, listenAddr, reUsePort)),
    m_started(false),
    m_threadPool(new EventLoopThreadPool(m_loop, m_name)) {
    
    m_acceptor->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, 
            this, std::placeholders::_1, std::placeholders::_2)
    );
}

TcpServer::~TcpServer() {
    GLOG_INFO << "TcpServer::~TcpServer()";
}

void TcpServer::start() {
    if (!m_started) {
        m_started = true;
    }
    GLOG_INFO << "TcpServer::start()";
    m_threadPool->start(m_threadInitCallback);
    
    assert(!m_acceptor->listening());
    m_loop->runInLoop(
        std::bind(&Acceptor::listen, m_acceptor.get()));
}

void TcpServer::setThreadNum(int numThreads) {
    assert(0 <= numThreads);
    m_threadPool->setThreadNum(numThreads);
}

void TcpServer::newConnection(int sockfd, const SocketAddress& remoteAddr) {
    m_loop->assertInLoopThread();
    EventLoop* ioLoop = m_threadPool->getNextLoopMinConnection();

    //set
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", m_connId);
    std::string connName = m_name + buf;
    m_connId ++;

    GLOG_INFO << "new connection: ["<< connName << "]" << remoteAddr.toString();

    SocketAddress localAdress(m_acceptor->getListenAddr());
    TcpConnection::Ptr conn(new TcpConnection(
                                            ioLoop,
                                            connName,
                                            sockfd,
                                            localAdress,
                                            remoteAddr,
                                            m_delay
                            ));
    m_connections[connName] = conn;
    conn->setConnectionCallback(m_connectionCallback);
    conn->setMessageCallback(m_messageCallback);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    // conn->connectEstablished();
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnection::Ptr conn) {
    m_loop->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn)
    );
}

void TcpServer::removeConnectionInLoop(const TcpConnection::Ptr conn) {
    m_loop->assertInLoopThread();
    GLOG_INFO << "TcpServer::removeConnectionInLoop [" << m_name
           << "] - connection " << conn->name();
    size_t n = m_connections.erase(conn->name());
    assert(n == 1);

    EventLoop* ioLoop = conn->getLoop();
    m_threadPool->removeLoopConnection(ioLoop);

    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn)   //将conn传入是关键,传递conn.get()不行,否则会产生生命周期问题
    );
}

}
}