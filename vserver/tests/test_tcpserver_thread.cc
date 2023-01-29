#include "TcpServer.h"
#include "Address.h"
#include "Eventloop.h"
#include "Connection.h"
#include "Buffer.h"
#include <iostream>
#include "Log.h"

using namespace vs;
using namespace vs::net;

void onConnection(TcpConnection::Ptr conn) {
    GLOG_INFO << "[Clint]onConnection" << std::endl;
}

void onMessage(TcpConnection::Ptr conn, Buffer* buf) {
    GLOG_INFO << "[Clint]Received \n" << buf->retrieveAllAsString();
}

int main(int argc, char const *argv[])
{
    EventLoop loop;
    SocketAddress m_addr;
    m_addr.setInAddrFromString("0.0.0.0");
    m_addr.setPortHost(9006);

    TcpServer server(&loop, "test", m_addr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.setThreadNum(0);
    server.start();

    loop.loop();

    
    return 0;
}
