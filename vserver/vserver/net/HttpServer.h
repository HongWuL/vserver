#ifndef __VS_NET_HTTPSERVER_H
#define __VS_NET_HTTPSERVER_H

#include "Noncopyable.h"
#include "TcpServer.h"
#include "Connection.h"
#include <memory.h>
#include <iostream>
#include "HttpRequeset.h"
#include "HttpResponse.h"

namespace vs {
namespace net {

class HttpServer : Noncopyable {
public:
  typedef std::function<void (const HttpRequest&,
                              HttpResponse*)> HttpCallback;

    HttpServer(EventLoop* loop, 
            const std::string& name,
            const SocketAddress& listenAddr, 
            const int thread_num,
            bool reUsePort = true,
            time_type timeout_ms = 0);

    ~HttpServer();
    
    void start();
    
    void onConnection(TcpConnection::Ptr conn);

    void onMessage(TcpConnection::Ptr conn, Buffer* buf);

    void setHttpCallback(HttpCallback cb) {
        m_httpCallback = cb;
    }


private:
    std::shared_ptr<TcpServer> m_tcpServer;
    HttpCallback m_httpCallback;
};

}  
} 

#endif