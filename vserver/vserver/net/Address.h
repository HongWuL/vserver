#ifndef __VS_ADDRESS_H
#define __VS_ADDRESS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <exception>
#include <iostream>
#include <string>

namespace vs {

namespace net {

enum Type {
    TCP = SOCK_STREAM
};

enum Family {
    IPv4 = AF_INET
};

enum Protocol {
    TCPproto = IPPROTO_TCP
};

class SocketAddress {
public:
    SocketAddress();
    void setFamily(Family family);
    void setInAddrFromString(const char* addr = "");
    void setAddrNetwork(struct sockaddr_in addr);
    void setPortHost(uint16_t port);
    void setPortNetwork(uint16_t port);

    sockaddr* getAddr() const;
    int getAddrLen() const;

    std::string toString() const;
private:
    //存储的是网络端地址
    sockaddr_in m_addr;
};


} // namespace net
} // namespace vs




#endif

