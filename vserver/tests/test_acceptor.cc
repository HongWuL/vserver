#include "Acceptor.h"
#include "Address.h"
#include "Eventloop.h"
#include <iostream>
#include <memory>

using namespace vs;
using namespace vs::net;

void handler(int connfd, const SocketAddress &addr) {
    std::cout << "connfd = " << connfd << std::endl;
    std::cout << "addr = " << addr.toString() << std::endl;
    socket::Close(connfd);
}

int main(int argc, char const *argv[])
{
    EventLoop loop;
    SocketAddress m_addr;
    m_addr.setInAddrFromString("0.0.0.0");
    m_addr.setPortHost(6000);

    std::unique_ptr<Acceptor> acceptor(new Acceptor(&loop, m_addr));
    acceptor->setNewConnectionCallback(handler);
    acceptor->listen();
    std::cout << acceptor->getListenAddr().toString() << std::endl;

    loop.loop();

    return 0;
}
