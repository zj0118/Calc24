#include "Select.h"

//void Select::poll(int timeoutUs, std::vector<IEventDispatcher*> triggeredEventDispatchers)
//{
//	struct timeval tv;
//	tv.tv_sec = timeoutUs / 1000000;
//	tv.tv_usec = timeoutUs - timeoutUs / 1000000 * 1000000;
//
//	int n = ::select(m_maxfd + 1, &m_readfds, &m_writefds, nullptr, &tv);
//	if (n <= 0)
//	{
//		return;
//	}
//
//	//TODO:����m_readfds������FD_ISSET�ж���Щsocket�ɶ�,������
//	triggeredEventDispatchers.push_back(fd1);
//
//	
//	//TODO:����m_writefds������FD_ISSET�ж���Щsocket��д��������
//}
//
//void Select::registerReadEvent(int fd, bool readEvent)
//{
//	if (m_maxfd < fd)
//	{
//		m_maxfd = fd;
//	}
//
//	FD_SET(fd, &m_readfds);
//}
//
//void Select::registerWriteEvent(int fd, bool writeEvent)
//{
//	if (m_maxfd < fd)
//	{
//		m_maxfd = fd;
//	}
//
//	FD_SET(fd, &m_writefds);
//}