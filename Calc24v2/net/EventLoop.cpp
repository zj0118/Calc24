#include "EventLoop.h"

#include <functional>
#include <vector>
#include <iostream>

#include <sys/eventfd.h>

#include "../appsrc/AsyncLog.h"
#include "Select.h"
#include "Poll.h"
#include "Epoll.h"

EventLoop::~EventLoop()
{
	if (m_pWakeupEventDispatcher)
		delete m_pWakeupEventDispatcher;
}

bool EventLoop::init(IOMultiplexType type/* = IOMultiplexType::IOMultiplexTypeEpoll*/) {

	if (type == IOMultiplexType::IOMultiplexTypeSelect)
	{
		m_spIOMultiplex = std::make_unique<Select>();
	}
	else if (type == IOMultiplexType::IOMultiplexTypePoll)
	{
		m_spIOMultiplex = std::make_unique<Poll>();
	}
	else
	{
		m_spIOMultiplex = std::make_unique<Epoll>();
	}

	//创建唤醒fd
	if(!createWakeupfd())
		return false;

	m_pWakeupEventDispatcher = new WakeupEventDispatcher(m_wakeupfd);
	registerReadEvent(m_wakeupfd, m_pWakeupEventDispatcher, true);

	m_running = true;
	return true;
}

void EventLoop::run()
{
	std::vector<IEventDispatcher*> eventDispatchers;
	while (m_running)
	{
		//1、检测和处理定时器时间
		checkAndDoTimers();

		//2、使用IO复用函数检测一组socket读写事件
		eventDispatchers.clear();
		m_spIOMultiplex->poll(500000, eventDispatchers);

		//3、处理读写事件
		for (size_t i = 0; i < eventDispatchers.size(); ++i)
		{
			//客户端连接会调用Acceptor的onRead(),客户端发消息会调用TCPConnection的onRead()
			eventDispatchers[i]->onRead();
			eventDispatchers[i]->onWrite();
		}
		
		//4、唤醒fd机制处理自定义事件
		doOtherTask();
	}
}

void EventLoop::addTask(CustomTask&& task)
{
	{
		std::lock_guard<std::mutex> scopeLock(m_mutexTasks);
		m_customTasks.push_back(std::move(task));
	}

	m_pWakeupEventDispatcher->wakeup();
	
}

void EventLoop::setThreadId(const std::thread::id& threadId)
{
	m_threadId = threadId;
}

const std::thread::id EventLoop::getThreadId() const
{
	return m_threadId;
}

bool EventLoop::isCallableInOwnerThread() const
{
	if (std::this_thread::get_id() == m_threadId)
		return true;
	else
		return false;
}

void EventLoop::registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent)
{
	//利用epoll_ctl注册和修改事件
	m_spIOMultiplex->registerReadEvent(fd, eventDispatcher,readEvent);
}

void EventLoop::registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent)
{
	m_spIOMultiplex->registerReadEvent(fd, eventDispatcher, writeEvent);
}

void EventLoop::unregisterReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent)
{
	m_spIOMultiplex->unregisterReadEvent(fd, eventDispatcher, readEvent);
}

void EventLoop::unregisterWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent)
{
	m_spIOMultiplex->unregisterReadEvent(fd, eventDispatcher, writeEvent);
}

void EventLoop::unregisterAllEvents(int fd, IEventDispatcher* eventDispatcher)
{
	m_spIOMultiplex->unregisterReadWriteEvent(fd, eventDispatcher);
}

int64_t EventLoop::addTimer(int32_t intervalMs, bool repeated, int64_t repeatedCount, TimerTask timerTask, TimerMode mode)
{
	//TODO:对传入的参数进行有效性校验

	auto spTimer = std::make_shared<Timer>(intervalMs, repeated, repeatedCount, timerTask, mode);
	addTimerInternal(spTimer);

	return spTimer->id();
}
void EventLoop::removeTimer(int64_t timerID)
{
	removeTimerInternal(timerID);
}

void EventLoop::checkAndDoTimers()
{
	int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();

	//std::lock_guard<std::mutex> scopeLock(m_mutexTimers);
	//遍历效率低下，应该用有序的数据结构
	m_isCheckTimers = true;
	for (auto iter = m_timers.begin(); iter != m_timers.end(); iter++)
	{
		//如果是TimerModeFixedDelay，if就要换成while,如果是TimerModeFixedInterval，就是if
		if (nowMs >= (*iter)->nextTriggeredTimeMs())
		{
			//std::cout << "checkAndDoTimers, timerID:" << (*iter)->id() << " threadID: " << std::this_thread::get_id() << std::endl;
			//LOGI("start！checkAndDoTimers");
			(*iter)->doTimer((*iter)->id(), nowMs);
		}
	}

	m_isCheckTimers = false;
	if (!m_pendingRemoveTimers.empty())
	{
		for (auto& timerID : m_pendingRemoveTimers)
			removeTimerInternal(timerID);
	}

	if (!m_pendingAddTimers.empty())
	{
		for (auto& spTimer : m_pendingAddTimers)
			addTimerInternal(spTimer);
	}
}


bool EventLoop::createWakeupfd()
{
	//EFD_NONBLOCK:非阻塞
	m_wakeupfd = ::eventfd(0, EFD_NONBLOCK);
	if (m_wakeupfd == -1)
	{
		return false;
	}
	return true;
}

void EventLoop::doOtherTask()
{
	//空间换时间，减少锁的粒度和效率
	std::vector<CustomTask> tasks;
	{
		std::lock_guard<std::mutex> scopeLock(m_mutexTasks);
		tasks.swap(m_customTasks);
	}
	
	for (auto& task : tasks)
	{
		task();
	}
}

void EventLoop::addTimerInternal(std::shared_ptr<Timer> spTimer)
{
	if (isCallableInOwnerThread())
	{
		if (m_isCheckTimers)
		{
			m_pendingAddTimers.push_back(spTimer);
		}
		else
		{
			std::cout << "addTimerInternal, timerID:" << spTimer->id() << " threadID: " << std::this_thread::get_id() << std::endl;
			m_timers.push_back(spTimer);
		}
		
	}
	else
	{
		addTask(std::bind(&EventLoop::addTimerInternal, this, spTimer));
	}
}

void EventLoop::removeTimerInternal(int64_t timerID)
{
	if(isCallableInOwnerThread())
	{
		if (m_isCheckTimers)
		{
			m_pendingRemoveTimers.push_back(timerID);
		}
		else
		{
			for (auto iter = m_timers.begin(); iter != m_timers.end(); iter++)
			{
				if ((*iter)->id() == timerID)
				{
					std::cout << "removeTimerInternal, timerID:" << timerID << " threadID: " << std::this_thread::get_id() << std::endl;
					m_timers.erase(iter);
					return;
				}
			}
		}
	}
	else 
		addTask(std::bind(&EventLoop::removeTimerInternal, this, timerID));
}