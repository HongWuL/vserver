#include "Eventloop.h"
#include "EventLoopThreadPool.h"

using namespace vs;
using namespace vs::net;

int main(int argc, char const *argv[]) {

    EventLoop base_loop;

    EventLoopThreadPool pool(&base_loop, "test");
    pool.setThreadNum(8);
    pool.start();
    EventLoop *io_loop  = pool.getNextLoop();
    
    return 0;
}
