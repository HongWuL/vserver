#include "Socket.h"
#include "SocketUtils.h"

namespace vs {
namespace net {


Socket::Socket(int fd) :
    m_fd(fd) {
}

Socket::~Socket() {
    socket::Close(m_fd);
}

void Socket::bind(const SocketAddress& addr) {
    socket::Bind(m_fd, addr.getAddr(), addr.getAddrLen());
}   

void Socket::listen() {
    socket::Listen(m_fd);
}

int Socket::accept(SocketAddress* remoteAddr) {
    std::cout << "listen " << m_fd << std::endl;

    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    memset(&cliaddr, 0, sizeof cliaddr);

    int acceptfd = socket::Accept(m_fd, (struct sockaddr *)&cliaddr, &clilen);
    
    if(remoteAddr != NULL) {
        remoteAddr->setAddrNetwork(cliaddr);
    }

    return acceptfd;
    
}

void Socket::enableReUseAddr() {
    socket::EnableReUseAddr(m_fd);
}

void Socket::enableReUsePort() {
    socket::EnableReUsePort(m_fd);
}

void Socket::shutdownWrite() {
    socket::shutDownWrite(m_fd);
}

void Socket::setNonBlocking() {
    socket::SetNonBlocking(m_fd);
}

}
}
