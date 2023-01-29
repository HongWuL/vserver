#include <iostream>
#include <functional>
#include "Thread.h"
#include <unistd.h>
using namespace vs;

void f() {
    std::cout << "hello" << std::endl;    
}

void g(int x) {
    std::cout << "hi" << x << std::endl;    
}

int main(int argc, char const *argv[])
{
    Thread t(std::bind(f), "test");
    t.start();
    std::cout << t.tid() << std::endl;

    t.join();
    // ::sleep(2);
    return 0;
}
