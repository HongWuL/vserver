#ifndef __VS_NET_ACCEPTOR_H
#define __VS_NET_ACCEPTOR_H

#include "Address.h"
#include "Socket.h"
#include "Channel.h"

namespace vs {
namespace net {

class EventLoop;

class Acceptor {
public:
    typedef std::function<void (int sockfd, const SocketAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* loop, const SocketAddress& listenAddr, bool reuseport = true);
    ~Acceptor();
  
    void listen();

    bool listening() const { return m_listening; }

    void setNewConnectionCallback(const NewConnectionCallback& cb) { 
        m_newConnectionCallback = cb; }

    SocketAddress getListenAddr() const { 
        return m_listenAddr; 
    }

private:
    void handleRead();

    int m_listenfd;
    bool m_listening;
    
    SocketAddress m_listenAddr;
    EventLoop* m_loop;
    Socket m_acceptSocket;
    Channel m_accpetChannel;
    NewConnectionCallback m_newConnectionCallback;

};

}

}


#endif
