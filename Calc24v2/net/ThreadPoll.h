/*
* zj 2024.5.14
* 线程池类
*/

#pragma once

#include <cstdint>
#include <vector>
#include <thread>

#include "EventLoop.h"

class ThreadPoll final{
public:
	ThreadPoll() = default;
	~ThreadPoll() = default;

	//创建线程
	void start(int32_t threadNum = 1);
	void stop();

	std::shared_ptr<EventLoop> getNextEventLoop();

private:
	void threadFunc(size_t eventLoopIndex);

private:
	ThreadPoll(const ThreadPoll& tp) = delete;
	ThreadPoll& operator=(ThreadPoll& tp) = delete;

	ThreadPoll(ThreadPoll&& tp) = delete;
	ThreadPoll& operator=(ThreadPoll&& tp) = delete;

private:
	bool												m_stop{ false };
	size_t												m_lastEventLoopNo{ 0 };
	std::vector<std::shared_ptr<std::thread>>			m_threads;
	std::vector<std::shared_ptr<EventLoop>>				m_eventLoops;

};