#pragma once

#include "IOMultiplex.h"

#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <vector>

class Poll : public IOMultiplex {
public:
	void poll(int timeoutUs, std::vector<IEventDispatcher*>& triggeredEventDispatchers) override {};

	virtual void registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent) override {};
	virtual void registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent) override {};
	virtual void unregisterReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent) override {};
	virtual void unregisterWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent) override {};
	virtual void unregisterReadWriteEvent(int fd, IEventDispatcher* eventDispatcher) override {};

private:
	struct pollfd*							m_pollfd;
	std::map<int, IEventDispatcher*>		m_eventDispatchers;
};