#ifndef __VS_NET_POLLER_H
#define __VS_NET_POLLER_H

#include "Noncopyable.h"
#include <vector>
#include <map>

struct epoll_event;

namespace vs {
namespace net {

class Channel;
class EventLoop;

class Poller : Noncopyable {
public:
    enum class Mode {
        kLT,    //水平触发
        kET     //边缘触发
    };

    const static int kNew = -1;
    const static int kAdded = 1;
    const static int kDeleted = 2;

    typedef std::vector<Channel*> ChannelList;

    Poller(EventLoop* loop);
    ~Poller();
  
    int wait(int timeoutMS, ChannelList* activeChannels);

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    
private:
    typedef std::map<int, Channel*> ChannelMap;
    typedef std::vector<struct epoll_event> EventList;
  
    static const int kInitEventListSize = 16;

    void update(int operation, Channel* channel);
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
                          
    ChannelMap m_channels;  //fd : Channel
    EventList m_events;
    Mode m_mode;
    int m_epollfd;
    EventLoop* m_ownerLoop;
};

}
}


#endif

