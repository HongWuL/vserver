#include "SocketUtils.h"

namespace vs {
namespace net {
namespace socket {

int TcpSocket(bool nonblocking) {
    int sockfd = ::socket(Family::IPv4, Type::TCP, Protocol::TCPproto);
    if(sockfd < 0) {
        std::cout << "create tcp socket error ." << strerror(errno) << std::endl;
        throw std::exception();
    }
    if(nonblocking) {
        SetNonBlocking(sockfd);
    }
    return sockfd;
}

void Connect(int sockfd, const sockaddr *addr, socklen_t addrlen) {
    int ret = ::connect(sockfd, addr, addrlen);
    if(ret < 0) {
        std::cout << "connect failed. " <<strerror(errno) << std::endl;
        throw std::exception();
    }
}

void Bind(int sockfd, const sockaddr *addr, socklen_t addrlen) {
    int ret = ::bind(sockfd, addr, addrlen);
    if(ret < 0) {
        std::cout << "bind failed. " <<strerror(errno) << std::endl;
        throw std::exception();
    }
}

void Listen(int sockfd, int backlog) {
    int ret = ::listen(sockfd, backlog);
    if(ret < 0) {
        std::cout << "listen failed. " <<strerror(errno) << std::endl;
        throw std::exception();
    }
}

int Accept(int sockfd, sockaddr *addr, socklen_t *addrlen) {
    int connfd = ::accept(sockfd, addr, addrlen);
    if(connfd < 0) {
        std::cout << "accept failed. " <<strerror(errno) << std::endl;
        throw std::exception();
    }   
    return connfd;
}

void Close(int fd) {
    int ret = ::close(fd);
    if(ret < 0) {
        std::cout << "accept failed. " <<strerror(errno) << std::endl;
        throw std::exception();
    }   
}

static ssize_t _read(int fd, void *vptr, size_t n) {
	size_t	nleft;
	ssize_t	nread;
	char	*ptr;

	ptr = (char*)vptr;
	nleft = n;

	while (nleft > 0) {
		if ( (nread = ::read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;		/* and call read() again */
			else
				return -1;
		} else if (nread == 0)
			break;				/* EOF */

		nleft -= nread;
		ptr   += nread;
	}

	return(n - nleft);		/* return >= 0 */
}


ssize_t Readn(int fd, void *ptr, size_t nbytes) {
    ssize_t n;
    if((n = _read(fd, ptr, nbytes)) < 0) {
        std::cout << "read failed. " <<strerror(errno) << std::endl;
    }
    return n;
} 

static ssize_t	_writen(int fd, const void *vptr, size_t n) {

	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = (const char*)vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = ::write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}


void Writen(int fd, const void *ptr, size_t nbytes)
{
	if (_writen(fd, ptr, nbytes) != nbytes) {
        std::cout << "writen error. " << strerror(errno) << std::endl; 
        throw std::exception();
    }
}

ssize_t Readv(int fd, iovec *iov, int iovcnt) {
    int n = ::readv(fd, iov, iovcnt);
    if(n  < 0) {
        std::cout << "readv error. " << strerror(errno) << std::endl; 
        throw std::exception();
    }
    return n;
}

void Writev(int fd, const iovec *iov, int iovcnt) {
    if (::writev(fd, iov, iovcnt) < 0) {
        std::cout << "writev error. " << strerror(errno) << std::endl; 
        throw std::exception();
    }
}

void shutDownWrite(int fd) {
    if(::shutdown(fd, SHUT_WR) < 0) {
        std::cout << "Shutdown write error. " << strerror(errno) << std::endl; 
    }
}
/***
 * @brief 设置描述符为非阻塞
*/
void SetNonBlocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
}

/***
 * @brief 允许服务器bind一个地址，即使这个地址当前已经存在已建立的连接
*/
void EnableReUseAddr(int fd) {
    int iSockOptVal = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &iSockOptVal, sizeof(iSockOptVal));
}

void EnableReUsePort(int fd) {
    int iSockOptVal = 1;
    int ret = ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &iSockOptVal, sizeof(iSockOptVal));
}

}
} 
} 
