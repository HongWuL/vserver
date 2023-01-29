#ifndef __VS_NET_CONNETCTION_H
#define __VS_NET_CONNETCTION_H

#include <string>
#include <memory>
#include <functional>
#include "Address.h"
#include "Noncopyable.h"
#include "Buffer.h"
#include "Utils.h"
#include "Timer.h"

namespace vs{
namespace net {

class EventLoop;
class Socket;
class Channel;

class TcpConnection : Noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

    typedef std::shared_ptr<TcpConnection> Ptr;

    typedef std::function<void (const Ptr&)> ConnectionCallback;
    typedef std::function<void (const Ptr&,
                            Buffer* data)> MessageCallback;
    typedef std::function<void (const Ptr&)> CloseCallback;

    TcpConnection(EventLoop* loop,
                const std::string name,
                int sockfd,
                const SocketAddress& localAddr,
                const SocketAddress& remoteAddr,
                time_type delay=0);
    ~TcpConnection();

    bool connected() { return m_state == kConnected; }
    const std::string& name() const { return m_name; }

    void connectEstablished();
    void connectDestroyed();

    void send(const char* message, int len);
    void send(const std::string& message);
    void shutdown();
    void forceClose();
    void forceCloseAfter(time_type ms);

    void setConnectionCallback(const ConnectionCallback& cb) {
        m_connectionCallback = cb;
    }
    void setMessageCallback(const MessageCallback& cb) {
        m_messageCallback = cb;
    }
    void setCloseCallback(const CloseCallback& cb) {    //给TcpServer用的
        m_closeCallback = cb;
    }

    std::string name() { return m_name; }
    EventLoop* getLoop() const { return m_loop; }

private:
    // //回调相关
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const std::string& message);
    // void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();
    void forceCloseInLoop();

    void setState(StateE s) { m_state = s; }
    void updateTimer();

    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    CloseCallback m_closeCallback;  //for TcpServer

    EventLoop* m_loop;
    std::string m_name;
    SocketAddress m_localAddr;
    SocketAddress m_remoteAddr;
    std::unique_ptr<Socket> m_socket;
    std::unique_ptr<Channel> m_channel;

    StateE m_state;

    //Buffer
    Buffer m_inputBuffer;
    Buffer m_outputBuffer;

    //定时断开
    time_type m_delay;
    Timer::id_type m_timerId;
};
    
} 

}

#endif