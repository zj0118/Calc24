#include "WakeupEventDispatcher.h"

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


#include <iostream>

WakeupEventDispatcher::WakeupEventDispatcher(int fd) : m_wakeupfd(fd)
{

}

WakeupEventDispatcher::~WakeupEventDispatcher()
{
	::close(m_wakeupfd);
}

void WakeupEventDispatcher::onRead()
{
	if (!m_enableRead)
		return;

	//������
	uint64_t dummyData;
	int n = ::read(m_wakeupfd, static_cast<void*>(&dummyData), sizeof(dummyData));
	if (n != sizeof(dummyData))
		std::cout << "WakeupEventDispatcher::onRead() failed, errno : " << errno << std::endl;
	else
		std::cout << "WakeupEventDispatcher::onRead() success, m_wakeupfd : " << m_wakeupfd << std::endl;
}

void WakeupEventDispatcher::enableReadWrite(bool read, bool write)
{
	m_enableRead = read;
	m_enableWrite = write;
}

void WakeupEventDispatcher::wakeup()
{
	uint64_t dummyDate = 0;
	//����eventfd����Ҫд��8���ֽڣ���Ȼ�ᱨ��EINVAL
	int n = ::write(m_wakeupfd, static_cast<const void*>(&dummyDate), static_cast<size_t>(sizeof(dummyDate)));
	if (n != sizeof(dummyDate))
		std::cout << "WakeupEventDispatcher::wakeup() failed, errno : " << errno << " " << strerror(errno) << std::endl;
	else
		std::cout << "WakeupEventDispatcher::wakeup() success, m_wakeupfd : " << m_wakeupfd << std::endl;
}