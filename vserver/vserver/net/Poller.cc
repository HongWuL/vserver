
#include "Poller.h"
#include "Eventloop.h"
#include "Channel.h"
#include "Utils.h"
#include <assert.h>
#include <cstring>
#include <sys/epoll.h>
#include "Log.h"

namespace vs {
namespace net {

Poller::Poller(EventLoop* loop):
    m_ownerLoop(loop),
    m_epollfd(::epoll_create1(EPOLL_CLOEXEC)),
    m_events(kInitEventListSize),
    m_mode(Mode::kLT) {

}

Poller::~Poller() {
    ::close(m_epollfd);
}

int Poller::wait(int timeoutMS, ChannelList* activeChannels) {
    int numEvents = ::epoll_wait(m_epollfd, 
                                 &*m_events.begin(), 
                                 static_cast<int>(m_events.size()),
                                 timeoutMS);
    int savedErrno = errno;
    if(numEvents > 0) {
        fillActiveChannels(numEvents, activeChannels);
        if (implicit_cast<size_t>(numEvents) == m_events.size()) {
            //自适应扩充缓冲区域
            m_events.resize(m_events.size() * 2);
        }
    }
    else if (numEvents == 0) {
        GLOG_DEBUG << "Nothing happened.";
    }
    else {
        if (savedErrno != EINTR) {
            errno = savedErrno;
            GLOG_ERROR << "EPollPoller::wait() error." << strerror(savedErrno);
        }
    }
    return numEvents;
}

void Poller::update(int operation, Channel* channel) {
    struct epoll_event event;
    memset(&event, 0, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if(m_mode == Mode::kET) {
        GLOG_INFO << "Epoll mode: ET";
        event.events |= EPOLLET;
    }

    ::epoll_ctl(m_epollfd, operation, fd, &event);
}

void Poller::updateChannel(Channel* channel) {
    m_ownerLoop->assertInLoopThread();
    const int index = channel->index();
    const int fd = channel->fd();
    if(index == kNew || index == kDeleted) {
        // 新添加Channel或者之前被删除过
        if(index == kNew) {
            assert(m_channels.find(fd) == m_channels.end());
            m_channels[fd] = channel;
        }
        else {
            assert(m_channels.find(fd) != m_channels.end());
            assert(m_channels[fd] == channel);
        }
        
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else {
        //修改Channel监听事件
        assert(m_channels.find(fd) != m_channels.end());
        assert(m_channels[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void Poller::removeChannel(Channel* channel) {
    m_ownerLoop->assertInLoopThread();
    int fd = channel->fd();
    assert(m_channels.count(fd) > 0);

    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    int n = m_channels.erase(fd);
    assert(n == 1);

    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void Poller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
    for(int i = 0; i < numEvents; i ++) {
        Channel* channel = static_cast<Channel*>(m_events[i].data.ptr);
        channel->set_revents(m_events[i].events);
        activeChannels->push_back(channel);
    }
}

}
}

