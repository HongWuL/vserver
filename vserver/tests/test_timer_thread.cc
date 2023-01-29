#include "Eventloop.h"
#include <stdio.h>
#include <thread>
#include "Utils.h"

vs::net::EventLoop* g_loop;

int id1, id2;

void run() {
    printf("run(): pid = %d\n", vs::GetThreadId());
}

void run1() {
    printf("run1(): pid = %d\n", vs::GetThreadId());
}
void run2() {
    printf("run2(): pid = %d\n", vs::GetThreadId());
}
void run3() {
    printf("run3(): pid = %d\n", vs::GetThreadId());
    g_loop->cancel(id1);
    g_loop->cancel(id2);
}

void threadfunc()
{
    printf("threadfunc(): pid = %d\n", vs::GetThreadId());
    g_loop->runAfter(2000, run);
}

int main()
{
  printf("main(): pid = %d\n", vs::GetThreadId());

  vs::net::EventLoop loop;
  g_loop = &loop;
  id1 = loop.runAfter(10000, run1);
  id2 = loop.runAfter(5000, run2);
  loop.runAfter(1000, run3);
//   std::thread t1(threadfunc);
//   std::thread t2(threadfunc);
  loop.loop();
}