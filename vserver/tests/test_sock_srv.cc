#include "SocketUtils.h"

#define MAXLINE 1024

int main(int argc, char const *argv[])
{
    vs::net::SocketAddress addr;
    addr.setInAddrFromString("0.0.0.0");
    addr.setPortHost(6000);
    addr.setFamily(vs::net::Family::IPv4);
    char buf[MAXLINE];

    int listenfd = vs::net::socket::TcpSocket();
    vs::net::socket::Bind(listenfd, addr.getAddr(), addr.getAddrLen());
    vs::net::socket::Listen(listenfd);
    
    while(1) {
        int connfd = vs::net::socket::Accept(listenfd, NULL, NULL);
        std::cout << "accept" << std::endl;
        vs::net::socket::Readn(connfd, buf, sizeof(buf));
        std::cout << "Read " << buf << std::endl;
        vs::net::socket::Close(connfd);
    }

    return 0;
}
