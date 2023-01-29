#ifndef __VS_NET_CHANNEL_H
#define __VS_NET_CHANNEL_H


#include <functional>
#include "Noncopyable.h"
#include <sys/epoll.h>
#include <memory>

namespace vs {
namespace net {

class EventLoop;

class Channel {
public:
    
    const static int kNoneEvent = 0;
    const static int kReadEvent = EPOLLIN | EPOLLPRI;
    const static int kWriteEvent = EPOLLOUT;   

    typedef std::function<void()> EventCallback;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent();

    void setReadCallback(EventCallback cb) { m_readCallback = std::move(cb); }
    
    void setWriteCallback(EventCallback cb) { m_writeCallback = std::move(cb); }
    
    void setCloseCallback(EventCallback cb) { m_closeCallback = std::move(cb); }
    
    void setErrorCallback(EventCallback cb) { m_errorCallback = std::move(cb); }

    void enableReading() { m_events |= kReadEvent; update(); }
    void disableReading() { m_events &= ~kReadEvent; update(); }
    void enableWriting() { m_events |= kWriteEvent; update(); }
    void disableWriting() { m_events &= ~kWriteEvent; update(); }
    void disableAll() { m_events = kNoneEvent; update(); }
 
    bool isWritable() const { return m_events & kWriteEvent; }
    bool isReadable() const { return m_events & kReadEvent; }

    // for Poller
    int fd() const { return m_fd; }
    int index() const { return m_index; }
    int events() const { return m_events; }
    bool isNoneEvent() const { return m_events == kNoneEvent; };
    void set_index(int idx) { m_index = idx; }
    void set_revents(int revt) { m_revents = revt; }
    EventLoop* ownerLoop() { return m_loop;};

    void remove();
    // for Connection
    void tie(const std::shared_ptr<void>& obj);

private:
    void update();

    EventLoop* m_loop;
    const int  m_fd;
    int        m_events;
    int        m_revents; 
    int        m_index;

    EventCallback m_readCallback;
    EventCallback m_writeCallback;
    EventCallback m_closeCallback;
    EventCallback m_errorCallback;

    bool m_eventHandling;
    std::weak_ptr<void> m_tie;
    bool m_tied;
};


}
}


#endif

