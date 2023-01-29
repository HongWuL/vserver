#include "TcpServer.h"
#include "Address.h"
#include "Eventloop.h"
#include "Connection.h"
#include "Buffer.h"
#include <iostream>

using namespace vs;
using namespace vs::net;

std::string message1;
std::string message2;
std::string message;

int sleepSeconds;

void onConnection(TcpConnection::Ptr conn) {
    std::cout << "[Clint]onConnection" << std::endl;
    if (conn->connected()){

        printf("[Clint]onConnection(): new connection [%s]\n",
            conn->name().c_str());
        if(sleepSeconds > 0) {
            ::sleep(sleepSeconds);
        }

        conn->send(message1);
        conn->send(message2);
        // conn->send(message);
        conn->shutdown();
    }
    else {
        printf("[Clint]onConnection(): connection [%s] is down\n",
            conn->name().c_str());
    }
}

void onMessage(TcpConnection::Ptr conn, Buffer* buf) {
    printf("[Clint]onMessage(): received %zd bytes from connection [%s]\n",
        buf->readableBytes(),
        conn->name().c_str());
    buf->retrieveAll();
}

int main(int argc, char const *argv[]) {

    printf("main(): pid = %d\n", getpid());

    int len1 = 1000000;
    int len2 = 2000000;   

    if(argc == 2) {
        sleepSeconds = atoi(argv[1]);
    }

    if (argc == 3) {
        len1 = atoi(argv[1]);
        len2 = atoi(argv[2]);
    }

    message1.resize(len1);
    message2.resize(len2);
    std::fill(message1.begin(), message1.end(), 'A');
    std::fill(message2.begin(), message2.end(), 'B');

    // std::string line;
    // for (int i = 33; i < 127; ++i) {
    //     line.push_back(char(i));
    // }
    // line += line;

    // for (size_t i = 0; i < 127-33; ++i) {
    //     message += line.substr(i, 72) + '\n';
    // }

    EventLoop loop;
    SocketAddress m_addr;
    m_addr.setInAddrFromString("0.0.0.0");
    m_addr.setPortHost(9006);

    TcpServer server(&loop, "test", m_addr);
    server.setThreadNum(0);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();

    return 0;
}
