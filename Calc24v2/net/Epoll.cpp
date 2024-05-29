#include "Epoll.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>

#include <iostream>

#include "util.h"

Epoll::Epoll()
{
	m_epollfd = epoll_create(EPOLL_CLOEXEC);
	if (m_epollfd < 0)
	{
		crash();
	}
}

Epoll::~Epoll()
{
	if (m_epollfd > 0)
	{
		::close(m_epollfd);
	}
}

void Epoll::poll(int timeoutUs, std::vector<IEventDispatcher*>& triggeredEventDispatchers)
{
	//����epoll������д�¼�
	struct epoll_event events[1024];
	int timeoutMs = timeoutUs / 1000;

	bool enableRead = false;
	bool enableWrite = false;

	int n = ::epoll_wait(m_epollfd, events, 1024, timeoutMs);
	IEventDispatcher* pEventDispatcher;

	for (int i = 0; i < n; i++)
	{
		if (events[i].events & EPOLLIN)
			enableRead = true;
		else 
			enableRead = false;

		if (events[i].events & EPOLLOUT)
			enableWrite = true;
		else
			enableWrite = false;
		
		pEventDispatcher = static_cast<IEventDispatcher*>(events[i].data.ptr);
		pEventDispatcher->enableReadWrite(enableRead, enableWrite);
		triggeredEventDispatchers.push_back(pEventDispatcher);
	}
	
}

void Epoll::registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent)
{
	int32_t eventFlag = 0;
	//���ж�fd��ʲô�¼�
	auto iter = m_fdEventFlags.find(fd);
	if (iter == m_fdEventFlags.end())
	{
		eventFlag |= EPOLLIN;
		m_fdEventFlags[fd] = eventFlag;
	}
	else
	{
		eventFlag = iter->second;
		if (eventFlag & EPOLLIN)
		{
			return;
		}

		eventFlag |= EPOLLIN;
		m_fdEventFlags[fd] = eventFlag;
	}

	//��m_epollEvent����
	struct epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = eventFlag;
	event.data.ptr = eventDispatcher;
	if (::epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		//TODO:��ӡ������־
		crash();
	}
}

void Epoll::registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent)
{
	int32_t eventFlag = 0;
	//���ж�fd��ʲô�¼�
	auto iter = m_fdEventFlags.find(fd);
	if (iter == m_fdEventFlags.end())
	{
		eventFlag |= EPOLLOUT;
		m_fdEventFlags[fd] = eventFlag;
	}
	else
	{
		eventFlag = iter->second;
		if (eventFlag & EPOLLOUT)
		{
			return;
		}

		eventFlag |= EPOLLOUT;
		m_fdEventFlags[fd] = eventFlag;
	}

	//��m_epollEvent����
	struct epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = eventFlag;
	event.data.ptr = eventDispatcher;
	if (::epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		//TODO:��ӡ������־
		crash();
	}
}

void Epoll::unregisterReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent)
{
	int32_t eventFlag = 0;
	int operation;
	//���ж�fd��ʲô�¼�
	auto iter = m_fdEventFlags.find(fd);
	if (iter == m_fdEventFlags.end())
	{
		return;
	} else
	{
		eventFlag = iter->second;
		if (eventFlag & EPOLLIN)
		{
			eventFlag &= ~EPOLLIN;
		}

		//û���κ��¼�ֱ��ɾ��
		if (eventFlag == 0)
		{
			m_fdEventFlags.erase(iter);
			operation = EPOLL_CTL_DEL;
		}
		else
		{
			m_fdEventFlags[fd] = eventFlag;
			operation = EPOLL_CTL_MOD;
		}
	}

	//��m_epollEvent����
	struct epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = eventFlag;
	event.data.ptr = eventDispatcher;
	if (::epoll_ctl(m_epollfd, operation, fd, &event) < 0)
	{
		//TODO:��ӡ������־
		crash();
	}
}

void Epoll::unregisterWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent)
{
	int32_t eventFlag = 0;
	int operation;

	//���ж�fd��ʲô�¼�
	auto iter = m_fdEventFlags.find(fd);
	if (iter == m_fdEventFlags.end())
	{
		return;
	}
	else
	{
		eventFlag = iter->second;
		if (eventFlag & EPOLLOUT)
		{
			eventFlag &= ~EPOLLOUT;
		}

		//û���κ��¼�ֱ��ɾ��
		if (eventFlag == 0)
		{
			m_fdEventFlags.erase(iter);
			operation = EPOLL_CTL_DEL;
		}
		else
		{
			m_fdEventFlags[fd] = eventFlag;
			operation = EPOLL_CTL_MOD;
		}
	}

	//��m_epollEvent����
	struct epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = eventFlag;
	event.data.ptr = eventDispatcher;
	if (::epoll_ctl(m_epollfd, operation, fd, &event) < 0)
	{
		//TODO:��ӡ������־
		crash();
	}
}

void Epoll::unregisterReadWriteEvent(int fd, IEventDispatcher* eventDispatcher)
{
	int32_t eventFlag = 0;

	//���ж�fd���¼���
	auto iter = m_fdEventFlags.find(fd);
	if (iter == m_fdEventFlags.end())
		return;

	m_fdEventFlags.erase(iter);
		
	//��m_epollEvent����
	struct epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = eventFlag;
	event.data.ptr = eventDispatcher;
	if (::epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, &event) < 0)
	{
		//TODO:��ӡ������־
		crash();
	}

	std::cout << "removed fd[" << fd << "] from epoll" << std::endl;
}