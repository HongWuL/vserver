cmake_minimum_required(VERSION 2.8)
project(vserver)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Wno-unused-function")

include_directories(.)
include_directories(vserver)
include_directories(vserver/net/)

# find_library(YAMLCPP yaml-cpp)

set(LIB_SRC
    vserver/Lock.cc
    vserver/Threadpool.cc
    vserver/Latch.cc
    vserver/Utils.cc
    vserver/Asynclogging.cc
    vserver/Log.cc
    vserver/Thread.cc
    vserver/net/SocketUtils.cc
    vserver/net/Address.cc
    vserver/net/Channel.cc
    vserver/net/Poller.cc
    vserver/net/Eventloop.cc
    vserver/net/Timer.cc
    vserver/net/Socket.cc
    vserver/net/Buffer.cc
    vserver/net/Acceptor.cc
    vserver/net/Connection.cc
    vserver/net/TcpServer.cc
    vserver/net/HttpServer.cc
    vserver/net/HttpRequeset.cc
    vserver/net/HttpResponse.cc
    vserver/net/EventLoopThread.cc
    vserver/net/EventLoopThreadPool.cc
)

add_library(vserver SHARED ${LIB_SRC})

add_executable(test_lock tests/test_lock.cc)
add_dependencies(test_lock vserver)
target_link_libraries(test_lock vserver pthread)

add_executable(test_thread tests/test_thread.cc)
add_dependencies(test_thread vserver)
target_link_libraries(test_thread vserver pthread)

add_executable(test_threadpool tests/test_threadpool.cc)
add_dependencies(test_threadpool vserver)
target_link_libraries(test_threadpool vserver pthread)

add_executable(test_address tests/test_address.cc)
add_dependencies(test_address vserver)
target_link_libraries(test_address vserver pthread)

add_executable(test_eventloop tests/test_eventloop.cc)
add_dependencies(test_eventloop vserver)
target_link_libraries(test_eventloop vserver pthread)

add_executable(test_timer tests/test_timer.cc)
add_dependencies(test_timer vserver)
target_link_libraries(test_timer vserver pthread)

add_executable(test_timer_thread tests/test_timer_thread.cc)
add_dependencies(test_timer_thread vserver)
target_link_libraries(test_timer_thread vserver pthread)

add_executable(test_socket tests/test_socket.cc)
add_dependencies(test_socket vserver)
target_link_libraries(test_socket vserver pthread)

add_executable(test_buffer tests/test_buffer.cc)
add_dependencies(test_buffer vserver)
target_link_libraries(test_buffer vserver pthread)

add_executable(test_acceptor tests/test_acceptor.cc)
add_dependencies(test_acceptor vserver)
target_link_libraries(test_acceptor vserver pthread)

add_executable(test_tcpserver tests/test_tcpserver.cc)
add_dependencies(test_tcpserver vserver)
target_link_libraries(test_tcpserver vserver pthread)

add_executable(test_send tests/test_send.cc)
add_dependencies(test_send vserver)
target_link_libraries(test_send vserver pthread)

add_executable(test_delay_close tests/test_delay_close.cc)
add_dependencies(test_delay_close vserver)
target_link_libraries(test_delay_close vserver pthread)

add_executable(test_eventloopthread tests/test_eventloopthread.cc)
add_dependencies(test_eventloopthread vserver)
target_link_libraries(test_eventloopthread vserver pthread)

add_executable(test_eventloopthreadpool tests/test_eventloopthreadpool.cc)
add_dependencies(test_eventloopthreadpool vserver)
target_link_libraries(test_eventloopthreadpool vserver pthread)

add_executable(test_tcpserver_thread tests/test_tcpserver_thread.cc)
add_dependencies(test_tcpserver_thread vserver)
target_link_libraries(test_tcpserver_thread vserver pthread)

add_executable(test_logger tests/test_logger.cc)
add_dependencies(test_logger vserver)
target_link_libraries(test_logger vserver pthread)

add_executable(test_asylog tests/test_asylog.cc)
add_dependencies(test_asylog vserver)
target_link_libraries(test_asylog vserver pthread)

add_executable(test_httpserver tests/test_httpserver.cc)
add_dependencies(test_httpserver vserver)
target_link_libraries(test_httpserver vserver pthread)

add_executable(test_httpreq tests/test_httpreq.cc)
add_dependencies(test_httpreq vserver)
target_link_libraries(test_httpreq vserver pthread)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)