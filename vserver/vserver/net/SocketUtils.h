#ifndef __VS_NET_SOCKETUTILS_H
#define __VS_NET_SOCKETUTILS_H
#include <sys/socket.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <iostream>
#include "string.h"
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "Address.h"
#include "Noncopyable.h"


namespace vs {
namespace net {
namespace socket {

int TcpSocket(bool nonblocking = false);

void Connect(int sockfd, const sockaddr *addr, socklen_t addrlen);

void Bind(int sockfd, const sockaddr *addr, socklen_t addrlen);

void Listen(int sockfd, int backlog = 4096);

int Accept(int sockfd, sockaddr *addr, socklen_t *addrlen);

void Close(int fd);

ssize_t Readn(int fd, void *vptr, size_t n);

void Writen(int fd, const void *vprt, size_t n);

ssize_t Readv(int fd, iovec *iov, int iovcnt);

void Writev(int fd, const iovec *iov, int iovcnt);

void shutDownWrite(int fd);

void SetNonBlocking(int fd);

void EnableReUseAddr(int fd);

void EnableReUsePort(int fd);

}
// Socket 函数简单封装

} // namespace socket

}
#endif
