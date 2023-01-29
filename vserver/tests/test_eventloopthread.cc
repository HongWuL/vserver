#include "EventLoopThread.h"
#include "Eventloop.h"
#include <iostream>
#include <unistd.h>
using namespace vs;
using namespace vs::net;

void func(EventLoop* loop) {
    std::cout << "hello" << std::endl;
    sleep(1);
}
int main(int argc, char const *argv[]) {

    /* code */
    EventLoopThread t("test", func);
    EventLoop *loop = t.startLoop();
    return 0;
}
