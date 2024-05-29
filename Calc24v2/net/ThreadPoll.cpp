#include "ThreadPoll.h"

#include <functional>
#include <memory>
#include <thread>

#include "EventLoop.h"

static constexpr int32_t MAX_THREAD_NUM = 20;
static constexpr int32_t DEFAULT_THREAD_NUM = 5;

void ThreadPoll::start(int32_t threadNum/* = 1*/)
{
	if (threadNum <= 0 || threadNum > MAX_THREAD_NUM)
	{
		threadNum = DEFAULT_THREAD_NUM;
	}

	for (int32_t i = 0; i < threadNum; ++i)
	{
		auto spEventLoop = std::make_shared<EventLoop>();
		spEventLoop->init();
		m_eventLoops.push_back(std::move(spEventLoop));
	}

	for (int32_t i = 0; i < threadNum; ++i)
	{
		auto spThread = std::make_shared<std::thread>(std::bind(&ThreadPoll::threadFunc, this, i));
		m_eventLoops[i]->setThreadId(spThread->get_id());
		m_threads.push_back(std::move(spThread));
	}
}

void ThreadPoll::stop()
{
	m_stop = true;
	size_t threadNum = m_threads.size();
	for (size_t i = 0; i < threadNum; ++i)
	{
		m_threads[i]->join();
	}
}

std::shared_ptr<EventLoop> ThreadPoll::getNextEventLoop()
{
	auto event = m_eventLoops[m_lastEventLoopNo];

	++m_lastEventLoopNo;
	if (m_lastEventLoopNo >= m_eventLoops.size())
	{
		m_lastEventLoopNo = 0;
	}

	return event;
}

void ThreadPoll::threadFunc(size_t eventLoopIndex)
{
	while (!m_stop)
	{
		//实际的线程工作
		m_eventLoops[eventLoopIndex]->run();
	}
}