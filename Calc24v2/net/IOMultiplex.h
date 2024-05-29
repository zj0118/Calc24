/*
* zj 2024.5.13
* IO复用函数包装类的接口类
*/
#pragma once

#include <vector>

#include "IEventDispatcher.h"

class IOMultiplex {
public:
	virtual void poll(int timeoutUs, std::vector<IEventDispatcher*>& triggeredEventDispatchers) = 0;

	virtual void registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent) = 0;
	virtual void registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent) = 0;

	virtual void unregisterReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent) = 0;
	virtual void unregisterWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent) = 0;

	virtual void unregisterReadWriteEvent(int fd, IEventDispatcher* eventDispatcher) = 0;
};