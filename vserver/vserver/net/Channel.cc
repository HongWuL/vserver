#include "Channel.h"
#include "Eventloop.h"
#include <iostream>
#include <assert.h>
#include "Log.h"

namespace vs {
namespace net {

Channel::Channel(EventLoop* loop, int fd):
    m_loop(loop),
    m_fd(fd),
    m_index(-1),
    m_events(0),
    m_revents(0),
    m_eventHandling(false) {
}

Channel::~Channel() {
    assert(!m_eventHandling);
    GLOG_INFO << "~Channel()" << std::endl;
}
void Channel::update() {
    m_loop->updateChannel(this);
}

void Channel::handleEvent() {
    m_eventHandling = true;
    if((m_revents & EPOLLHUP) && !(m_revents & EPOLLIN)) {
        if (m_closeCallback) m_closeCallback();
    }
    if(m_revents & (EPOLLERR)) {
        if (m_errorCallback) m_errorCallback();
    }
    if(m_revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (m_readCallback) m_readCallback();
    }
    if (m_events & (EPOLLOUT)) {
        if (m_writeCallback) m_writeCallback();
    }
    m_eventHandling = false;
}

void Channel::remove() {
    GLOG_DEBUG << "Channel::remove()" << std::endl;
    assert(isNoneEvent());
    m_loop->removeChannel(this);
}

void Channel::tie(const std::shared_ptr<void>& obj) {
    m_tie = obj;
    m_tied = true;
}

}
}



