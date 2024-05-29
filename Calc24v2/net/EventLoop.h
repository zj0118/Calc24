/*
* zj 2024.05.13
*/

#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include"IEventDispatcher.h"
#include "IOMultiplex.h"
#include "ITimerService.h"
#include "Timer.h"
#include "WakeupEventDispatcher.h"

using CustomTask = std::function<void()>;

enum class IOMultiplexType {
	IOMultiplexTypeSelect,
	IOMultiplexTypePoll,
	IOMultiplexTypeEpoll
};

class EventLoop final : public ITimerService{
public:
	EventLoop() = default;
	~EventLoop();

public:
	bool init(IOMultiplexType type = IOMultiplexType::IOMultiplexTypeEpoll);
	void run();

	void addTask(CustomTask&& task);

	void setThreadId(const std::thread::id& threadId);
	const std::thread::id getThreadId() const;

	bool isCallableInOwnerThread() const;

	void registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent);
	void registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent);
	void unregisterReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent);
	void unregisterWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent);
	void unregisterAllEvents(int fd, IEventDispatcher* eventDispatcher);

	//定时器相关
	virtual int64_t addTimer(int32_t intervalMs, bool repeated, int64_t repeatedCount, TimerTask timerTask, TimerMode mode = TimerMode::TimerModeFixedInterval) override;
	virtual void removeTimer(int64_t timerID) override;

private:
	void checkAndDoTimers();

	bool createWakeupfd();
	void doOtherTask();

	void addTimerInternal(std::shared_ptr<Timer> spTimer);
	void removeTimerInternal(int64_t timerID);

private:
	bool									m_running{ false };
	int										m_epollfd;
	int										m_wakeupfd;
	bool									m_isCheckTimers{ false };

	std::unique_ptr<IOMultiplex>			m_spIOMultiplex;
	std::thread::id							m_threadId;

	std::vector<CustomTask>					m_customTasks;
	std::mutex								m_mutexTasks;

	WakeupEventDispatcher*					m_pWakeupEventDispatcher{ nullptr };
	//定时器
	std::vector<std::shared_ptr<Timer>>		m_timers;
	std::vector<std::shared_ptr<Timer>>		m_pendingAddTimers;
	std::vector<int64_t>					m_pendingRemoveTimers;
	std::mutex								m_mutexTimers;
};