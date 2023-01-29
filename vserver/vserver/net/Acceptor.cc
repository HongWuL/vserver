#include "Acceptor.h"
#include "SocketUtils.h"
#include "Eventloop.h"

namespace vs {
namespace net {

Acceptor::Acceptor(EventLoop* loop, const SocketAddress& listenAddr, bool reuseport) :
    m_loop(loop),
    m_listening(false),
    m_listenAddr(listenAddr),
    m_acceptSocket(socket::TcpSocket(true)),
    m_accpetChannel(m_loop, m_acceptSocket.fd()) {
    
    m_acceptSocket.enableReUseAddr();
    m_acceptSocket.enableReUsePort();
    m_acceptSocket.bind(listenAddr);

    m_accpetChannel.setReadCallback(
        std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    m_accpetChannel.disableAll();
    m_accpetChannel.remove();
}

void Acceptor::listen() {
    m_loop->assertInLoopThread();
    m_listening = true;
    m_acceptSocket.listen();
    m_accpetChannel.enableReading();
}

void Acceptor::handleRead() {
    m_loop->assertInLoopThread();
    SocketAddress remoteAddress;

    int connfd = m_acceptSocket.accept(&remoteAddress);

    if (m_newConnectionCallback) {
        std::cout << "Acceptor::handleRead()::m_newConnectionCallback" << std::endl;
        m_newConnectionCallback(connfd, remoteAddress);
    }
    else {
        socket::Close(connfd);
    }
}

}
}