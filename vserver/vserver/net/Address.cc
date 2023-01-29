#include "Address.h"

namespace vs {
namespace net {

SocketAddress::SocketAddress() {
    memset(&m_addr, 0, sizeof m_addr);
    m_addr.sin_family = Family::IPv4;
    m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

void SocketAddress::setFamily(Family family) {
    m_addr.sin_family = family;
}

void SocketAddress::setInAddrFromString(const char* addr) {
    if(addr == "0.0.0.0") {
        m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else {
        int r = inet_pton(m_addr.sin_family, addr, &m_addr.sin_addr);
        if(r != 1) {
            std::cout << "Address not valid" << std::endl;
            throw std::exception();
        } 
    }
}

void SocketAddress::setAddrNetwork(sockaddr_in addr) {
    m_addr = addr;
}

void SocketAddress::setPortHost(uint16_t port) {
    m_addr.sin_port = htons(port);  // host to network
}

void SocketAddress::setPortNetwork(uint16_t port) {
    m_addr.sin_port = port;
}

sockaddr* SocketAddress::getAddr() const {
    return (sockaddr *) &m_addr;
}

int SocketAddress::getAddrLen() const{
    return sizeof(m_addr);
}

// 获取本地端地址String
std::string SocketAddress::toString() const{
    char addr_str[16];
    inet_ntop(m_addr.sin_family, &m_addr.sin_addr, addr_str, 16);

    in_port_t port = ntohs(m_addr.sin_port);

    char port_str[6];
    sprintf(port_str, "%u", port);

    std::string info = "";
    info += addr_str;
    info += ":";
    info += port_str;
    return info;
}

} // namespace net
} // namespace vs


