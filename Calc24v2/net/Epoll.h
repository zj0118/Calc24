#pragma once

#include "IOMultiplex.h"

#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <vector>

class Epoll : public IOMultiplex {
public:
	Epoll();
	virtual ~Epoll();

public:
	void poll(int timeoutUs, std::vector<IEventDispatcher*>& triggeredEventDispatchers) override;

	virtual void registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent) override;
	virtual void registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent) override;
	virtual void unregisterReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent) override;
	virtual void unregisterWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent) override;
	virtual void unregisterReadWriteEvent(int fd, IEventDispatcher* eventDispatcher) override;

private:
	Epoll(const Epoll& ep) = delete;
	Epoll& operator=(const Epoll& ep) = delete;

	Epoll(Epoll&& ep) = delete;
	Epoll& operator=(Epoll&& ep) = delete;

private:
	int														m_epollfd{ -1 };
	//key = fd,value = eventFlags
	std::map<int, int32_t>									m_fdEventFlags;
};