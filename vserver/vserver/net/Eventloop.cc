#include "Eventloop.h"
#include <assert.h>
#include <sys/eventfd.h>
#include "SocketUtils.h"
#include "Utils.h"
#include <signal.h>
#include "Log.h"

namespace vs {
namespace net {

static int createEventfd() {
	int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0) {
		GLOG_ERROR << "Failed in eventfd";
		abort();
	}
	return evtfd;
}

EventLoop::EventLoop():
	m_looping(false),
	m_quit(false),
	m_threadId(GetThreadId()),
	m_poller(new Poller(this)),
	m_wakeupfd(createEventfd()),
	m_wakeupChannel(new Channel(this,  m_wakeupfd)),
	m_timerManager(new TimerManager(this)),
	m_callingPendingFuncs(false) {
	
	::signal(SIGPIPE, SIG_IGN);

	m_wakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead, this));
	m_wakeupChannel->enableReading();

}

EventLoop::~EventLoop() {
	GLOG_DEBUG << "EventLoop::~EventLoop()";
	::close(m_wakeupfd);
}

void EventLoop::loop() {

	assert(!m_looping);
	assertInLoopThread();

	m_looping = true;
	m_quit = false;

	while(!m_quit) {
		GLOG_DEBUG << "Looping......";
		m_activeChannels.clear();
		int numEvents = m_poller->wait(kPollTimeMs, &m_activeChannels);
		for (Channel* channel : m_activeChannels) {
			GLOG_INFO << "Activate channel fd:" << channel->fd();
			channel->handleEvent();
		}
		doPendingFunctors();
	}
	m_looping = false;
}

void EventLoop::quit() {
	m_quit = true;
	GLOG_DEBUG << "Loop quit!";
}

void EventLoop::updateChannel(Channel* channel) {
	assertInLoopThread();
	m_poller->updateChannel(channel);
}

void EventLoop::runInLoop(Func cb) {
	if (isInLoopThread()) {
		// 如果是当前IO线程调用runInLoop，则同步调用cb
    	cb();
  	}
  	else {
		// 如果是其它线程调用runInLoop，则异步地将cb添加到队列
    	// 以便让EventLoop 所在的线程执行这个回调函数
    	queueInLoop(std::move(cb));
  	}
}

void EventLoop::queueInLoop(Func cb) {
	// 通过非阻塞的方法结束当前的事件处理过程,即立即结束Channel::handleEvent()
	{
	MutexLockGuard lock(m_mutex);
	m_pendingFuncs.push_back(std::move(cb));
	}
	GLOG_INFO << "EventLoop::queueInLoop";

	if (!isInLoopThread() || m_callingPendingFuncs) {
		// 调用queueInLoop的线程不是当前IO线程需要唤醒
		// 或者调用queueInLoop的线程是当前IO线程，并且此时正在调用pending functor，需要唤醒
		// 只有当前IO线程的事件回调中调用queueInLoop才不需要唤醒
		wakeup();
	}
}

void EventLoop::wakeup() {
	GLOG_DEBUG << "wakeup";
	uint64_t one = 1;
	socket::Writen(m_wakeupfd, &one, sizeof one);
}

void EventLoop::handleRead() {
	GLOG_DEBUG << "EventLoop::handleRead()";
	uint64_t one = 1;
	ssize_t n = socket::Readn(m_wakeupfd, &one, sizeof one);
	if(n != sizeof one) {
		GLOG_ERROR << "EventLoop::handleRead().";
		abort();
	}
}

void EventLoop::doPendingFunctors() {
	if(m_pendingFuncs.empty()) return;
 	GLOG_DEBUG << "doPendingFunctors() in " << GetThreadId();
	std::vector<Func> funcs;
	m_callingPendingFuncs = true;
	{
	MutexLockGuard lock(m_mutex);
	funcs.swap(m_pendingFuncs);
	}
	for (const Func& func : funcs) {
		func();
	}
	m_callingPendingFuncs = false;
}


Timer::id_type EventLoop::runAt(time_type time, Func cb) {
	return m_timerManager->addTimer(cb, time);
}

Timer::id_type EventLoop::runAfter(time_type delay, Func cb) {
	time_type t = TimeAdd(GetCurrentTimeMS(), delay);
	return runAt(t, cb);
}

void EventLoop::cancel(Timer::id_type timer_id) {
	m_timerManager->cancel(timer_id);
}

void EventLoop::abortNotInLoopThread() {
	GLOG_FATAL << "abortNotInLoopThread()";
	abort();
}

void EventLoop::removeChannel(Channel* channel) {
	assert(channel->ownerLoop() == this);
  	assertInLoopThread();
	m_poller->removeChannel(channel);
}

}
}