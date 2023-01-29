#include "Log.h"

using namespace vs;

int main(int argc, char const *argv[])
{
    LOG_INFO(GLOGER) << "hello world";
    LOG_ERROR(GLOGER) << "hello world error!";
    return 0;
}
