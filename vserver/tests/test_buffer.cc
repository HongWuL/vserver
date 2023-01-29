#include "Buffer.h"
#include <iostream>
using namespace vs;
using namespace vs::net;

int main(int argc, char const *argv[])
{
    Buffer buffer;
    buffer.append("nihao, china!");
    std::cout << buffer.readableBytes() << std::endl;
    std::cout << buffer.retrieveAllAsString() << std::endl;
    std::cout << buffer.readableBytes() << std::endl;
    return 0;
}
