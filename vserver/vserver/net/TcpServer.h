#ifndef __VS_NET_TCPSERVER_H
#define __VS_NET_TCPSERVER_H

#include "Noncopyable.h"
#include <string>
#include <memory>
#include <functional>
#include <Connection.h>
#include <map>

namespace vs {
namespace net {

class Acceptor;
class EventLoop;
class SocketAddress;
class EventLoopThreadPool;

class TcpServer: Noncopyable {
public:
    typedef TcpConnection::ConnectionCallback ConnectionCallback;
    typedef TcpConnection::MessageCallback MessageCallback;
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    TcpServer(EventLoop* loop, 
            const std::string& name,
            const SocketAddress& listenAddr, 
            bool reUsePort = true,
            time_type ms = 0);
    ~TcpServer();
    void start();
    
    void setConnectionCallback(const ConnectionCallback& cb) {
        m_connectionCallback = cb; 
    }

    void setMessageCallback(const MessageCallback& cb) {
        m_messageCallback = cb; 
    }

    // thread pool
    void setThreadNum(int numThreads);
    
    void setThreadInitCallback(const ThreadInitCallback& cb) { m_threadInitCallback = cb; }
    
    std::shared_ptr<EventLoopThreadPool> threadPool() { return m_threadPool; }
    

private:
    void newConnection(int sockfd, const SocketAddress& remoteAddr);
    void removeConnection(const TcpConnection::Ptr conn);
    void removeConnectionInLoop(const TcpConnection::Ptr conn);

    typedef std::map<std::string, TcpConnection::Ptr> ConnectionMap;
    EventLoop* m_loop;
    const std::string m_name;
    std::unique_ptr<Acceptor> m_acceptor;
    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    bool m_started;
    
    ConnectionMap m_connections;
    int m_connId;
    time_type m_delay;  // 多长时间客户端没有请求后自动断开连接, 0表示不自动断开

    //thread pool
    std::shared_ptr<EventLoopThreadPool> m_threadPool;
    ThreadInitCallback m_threadInitCallback;

};

}

}

#endif