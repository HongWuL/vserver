#include "Address.h"
#include "Eventloop.h"
#include "Connection.h"
#include "Buffer.h"
#include "HttpServer.h"
#include <iostream>
#include "HttpRequeset.h"
#include "HttpResponse.h"

using namespace vs;
using namespace vs::net;


void process(const HttpRequest& req, HttpResponse* res) {
    std::cout << "[Clint]" << std::endl;
    std::cout << req.info() << std::endl;

    res->setStatus(Http::StatusCode::OK);
    
    res->setBody("<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset=\"utf-8\">"
        "<title>Hello world</title>"
        "</head>"
        "<body>"
            "<h1>Hello world</h1>"
        "</body>"
        "</html>");
    
}

int main(int argc, char const *argv[])
{
    EventLoop loop;
    SocketAddress m_addr;
    m_addr.setInAddrFromString("0.0.0.0");
    m_addr.setPortHost(9006);

    HttpServer server(&loop, "test", m_addr, 4);
    server.setHttpCallback(process);
    
    server.start();

    loop.loop();

    return 0;
}
