#include "Socket.h"
#include "SocketUtils.h"
#include "Address.h"
using namespace vs;
using namespace vs::net;

int main(int argc, char const *argv[])
{
    SocketAddress addr;
    addr.setInAddrFromString("0.0.0.0");
    addr.setPortHost(6000);
    addr.setFamily(Family::IPv4);

    int sockfd = socket::TcpSocket();
    socket::EnableReUseAddr(sockfd);
    socket::EnableReUsePort(sockfd);

    Socket sock(sockfd);
    sock.bind(addr);
    sock.listen();
    // socket::Bind(sockfd, addr.getAddr(), addr.getAddrLen());
    // socket::Listen(sockfd);

    while(1) {
        SocketAddress remoteAddr;
        int fd = sock.accept(&remoteAddr);
        // int fd = socket::Accept(sockfd, NULL, 0);
        // char buf[1024];
        // int n = socket::Readn(fd, buf, sizeof buf);
        // int n = ::read(fd, buf, sizeof buf);

        std::cout << remoteAddr.toString() << std::endl;

        char extrabuf[100], extrabuf2[100];
        memset(extrabuf, 0, sizeof extrabuf);
        memset(extrabuf2, 0, sizeof extrabuf2);

        struct iovec vec[1];
        vec[0].iov_base = extrabuf;
        vec[0].iov_len = sizeof extrabuf;
        vec[1].iov_base = extrabuf2;
        vec[1].iov_len = sizeof extrabuf2;
        int n;
        n = socket::Readv(fd, vec, 2);
        std::cout << n << ": " << extrabuf << std::endl;

        const char* message = "reveived";

        socket::Writen(fd, message, strlen(message));

    }
    
    return 0;
}
