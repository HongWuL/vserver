#include "Eventloop.h"
#include "Channel.h"
#include "sys/timerfd.h"
#include <iostream>

vs::net::EventLoop *gloop;

void timeout() {
    std::cout << "timeout !" <<std::endl;
    gloop->quit();
}

int main(int argc, char const *argv[])
{
    vs::net::EventLoop loop;
    gloop = &loop;
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    vs::net::Channel channel(&loop, timerfd);
    channel.setReadCallback(timeout);
    channel.enableReading();

    itimerspec interval;
    bzero(&interval, sizeof interval);
    interval.it_value.tv_sec = 2;
    ::timerfd_settime(timerfd, 0, 0, NULL);

    loop.loop();
    ::close(timerfd);
    return 0;
}
