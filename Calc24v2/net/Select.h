#pragma once

#include "IOMultiplex.h"

#include <sys/types.h>
#include <unistd.h>

#include <vector>

class Select : public IOMultiplex {
public:
	void poll(int timeoutUs, std::vector<IEventDispatcher*>& triggeredEventDispatchers) override {};

	virtual void registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent) override {};
	virtual void registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent) override {};
	virtual void unregisterReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent) override {};
	virtual void unregisterWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent) override {};
	virtual void unregisterReadWriteEvent(int fd, IEventDispatcher* eventDispatcher) override {};

private:
	int								m_maxfd;
	//fd_set							m_readfds;
	//fd_set							m_writefds;

};