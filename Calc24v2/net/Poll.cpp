#include "Poll.h"

//void Poll::poll(int timeoutUs, std::vector<IEventDispatcher*> triggeredEventDispatchers)
//{
//	::poll();
//	//�õ����¼���fd
//	//�������¼���fd��ͨ��m_eventDispatchersȡ��IEventDispatcher*����
//	triggeredEventDispatchers.push_back(eventDispatcher);
//}
//
//void Poll::registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent)
//{
//	//��m_pollfd�ṹ������pollfd	
//	m_eventDispatchers[fd] = eventDispatcher;
//}
//
//void Poll::registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent)
//{
//	m_eventDispatchers[fd] = eventDispatcher;
//}