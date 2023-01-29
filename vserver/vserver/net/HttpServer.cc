#include "HttpServer.h"
#include "Log.h"

namespace vs {
namespace net {

HttpServer::HttpServer(EventLoop* loop, 
        const std::string& name,
        const SocketAddress& listenAddr, 
        const int thread_num,
        bool reUsePort,
        time_type timeout_ms):
    m_tcpServer(new TcpServer(loop, name, listenAddr, reUsePort, timeout_ms)) {
        
    m_tcpServer->setThreadNum(thread_num);
    m_tcpServer->setConnectionCallback(std::bind(
        &HttpServer::onConnection, this, std::placeholders::_1
    ));
    m_tcpServer->setMessageCallback(std::bind(
        &HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2
    ));
    GLOG_INFO << "HttpServer::HttpServer()";

}

HttpServer::~HttpServer() {
    GLOG_INFO << "HttpServer::~HttpServer()";
}

void HttpServer::start() {
    m_tcpServer->start();
}

void HttpServer::onConnection(TcpConnection::Ptr conn) {
    GLOG_INFO << "[HttpServer] onConnection";
}

void HttpServer::onMessage(TcpConnection::Ptr conn, Buffer* buf) {
    std::string msg = buf->retrieveAllAsString();
    HttpRequest req;
    req.parse(msg.c_str(), msg.size());
    GLOG_INFO << "[HttpServer] Received\n" << msg;
    const std::string& connection = req.getHeader("Connection");

    bool close = connection == "close" ||
        (req.getVersion() == Http::Version::Http10 && connection != "Keep-Alive");    
    HttpResponse res(close);   

    if(m_httpCallback) {
        m_httpCallback(req, &res);
    }

    std::string r = res.dump();

    conn->send(r);
    
    if(close) {
        conn->shutdown();
    }

}



}
}