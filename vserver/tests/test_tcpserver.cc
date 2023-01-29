#include "TcpServer.h"
#include "Address.h"
#include "Eventloop.h"
#include "Connection.h"
#include "Buffer.h"
#include <iostream>

using namespace vs;
using namespace vs::net;

void onConnection(TcpConnection::Ptr conn) {
    std::cout << "[Clint]onConnection" << std::endl;
}

void onMessage(TcpConnection::Ptr conn, Buffer* buf) {
    std::cout << "[Clint]Received " << buf->retrieveAllAsString() << std::endl;
}

int main(int argc, char const *argv[])
{
    EventLoop loop;
    SocketAddress m_addr;
    m_addr.setInAddrFromString("0.0.0.0");
    m_addr.setPortHost(6000);

    TcpServer server(&loop, "test", m_addr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();

    
    return 0;
}
