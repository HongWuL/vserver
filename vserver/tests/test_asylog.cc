#include "Asynclogging.h"
#include <iostream>
#include "Log.h"
#include <memory>

using namespace vs;

int main(int argc, char const *argv[])
{
    /* code */
    // std::shared_ptr<AsyncFileLogAppender> app(
    //     new AsyncFileLogAppender("test.log", 8));
    //  std::shared_ptr<StdoutLogAppender> app(
    //     new StdoutLogAppender);       
    // std::shared_ptr<Logger> logger(new Logger("ROOT_test", LogLevel::INFO, "file"));
    // logger->addLogAppender(app);
    GLOG_INFO << "Hello world";
    GLOG_WARN << "测试log";

    while(1);

    return 0;
}
