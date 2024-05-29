#include "Poll.h"

//void Poll::poll(int timeoutUs, std::vector<IEventDispatcher*> triggeredEventDispatchers)
//{
//	::poll();
//	//拿到有事件的fd
//	//遍历有事件的fd，通过m_eventDispatchers取到IEventDispatcher*对象
//	triggeredEventDispatchers.push_back(eventDispatcher);
//}
//
//void Poll::registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent)
//{
//	//向m_pollfd结构中添加pollfd	
//	m_eventDispatchers[fd] = eventDispatcher;
//}
//
//void Poll::registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent)
//{
//	m_eventDispatchers[fd] = eventDispatcher;
//}