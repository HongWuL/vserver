#ifndef __VS_NET_SOCKET_H
#define __VS_NET_SOCKET_H

#include "Noncopyable.h"
#include "Address.h"

namespace vs {
namespace net {

class Socket : Noncopyable {
public:
    Socket(int fd);
    ~Socket();

    int fd() { return m_fd; }
    
    void bind(const SocketAddress& addr);
    void listen();
    int accept(SocketAddress* remoteAddr);

    void enableReUseAddr();
    void enableReUsePort();

    void shutdownWrite();
    void setNonBlocking();
    
private:
    int m_fd;   
};

}
}


#endif