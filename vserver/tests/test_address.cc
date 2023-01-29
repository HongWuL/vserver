#include "Address.h"

int main(int argc, char const *argv[])
{
    /* code */
    vs::net::SocketAddress sock;
    sock.setFamily(vs::net::Family::IPv4);
    sock.setPortHost(3306);
    sock.setInAddrFromString("127.0.0.1");
    sockaddr* addr =  sock.getAddr();
    std::cout << sizeof(addr) << std::endl;
    std::cout << sock.getAddrLen() << std::endl;
    return 0;
}
