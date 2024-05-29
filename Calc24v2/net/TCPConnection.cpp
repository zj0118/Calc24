#include "TCPConnection.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <functional>//std::bind
#include <iostream>


TCPConnection::TCPConnection(int clientfd, const std::shared_ptr<EventLoop>& spEventLoop)
	:m_fd(clientfd), m_spEventLoop(spEventLoop)
{
	std::cout << "TCPConnection::ctor " << m_fd << std::endl;
}

TCPConnection::~TCPConnection()
{
	std::cout << "TCPConnection::dtor " << m_fd << std::endl;
	::close(m_fd);
}

bool TCPConnection::startRead()
{
	m_spEventLoop->registerReadEvent(m_fd, this, true);

	return true;
}

bool TCPConnection::send(const char* buf, int bufLen)
{
	return send(std::string(buf, bufLen));
}

bool TCPConnection::send(const std::string& buf)
{
	//����ֱ�ӷ���Ҫ�жϵ�ǰ���õ��̺߳͵�ǰTCPConnection�Ƿ�����ͬһ���̣߳��������ֱ�ӷ��ͣ�������ǣ��򽻸�Connection�������̷߳���
	if (isCallableInOwnerThread())
	{
		std::cout << "TCPConnection::send " 
			<< buf.length() 
			<< " bytes ,fd: " 
			<< m_fd
			<< " ThreadId: "
			<< m_spEventLoop->getThreadId() 
			<< std::endl;

		return sendInternal(buf.c_str(), buf.length());
	}
	else
	{
		//�����޷�ͨ�������ԭ���ǣ�bind�󶨵Ĳ����ǲ�������������0~n������������tcpconnection���ڶ�������Ա������޷��ж�Ҫѡ�ĸ����а�
		//m_spEventLoop->addTask(std::bind(&TCPConnection::send, this, buf));
		m_spEventLoop->addTask(std::bind(static_cast<bool(TCPConnection::*)(const std::string&)>
			(& TCPConnection::send), this, buf));

		return true;
	}
}

bool TCPConnection::sendInternal(const char* buf, int bufLen)
{
	m_sendBuf.append(buf, bufLen);

	while (true)
	{
		int n = ::send(m_fd, m_sendBuf, m_sendBuf.remaining(), 0);
		if (n == 0) {
			//�Զ˹ر�������
			//TODO: ����Ҳ�ر�����
			onClose();
			return false;
		}
		else if (n < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else if (errno == EWOULDBLOCK || errno == EAGAIN)
			{
				//��ǰ����tcp����̫С�����ݷ�����ȥ��
				m_writeCallback();
				//ע��д�¼�
				registerWriteEvent();

				return true;
			}

			//����������ǳ����ˣ��ر�����
			onClose();
			return false;
		}

		//���ͳɹ���
		m_sendBuf.erase(n);
		if (m_sendBuf.isEmpty())
		{
			return true;
		}
	}

	//return false;
}

void TCPConnection::onRead()
{
	if (!m_enableRead)
		return;

	//������
	char buf[1024];
	int n = ::recv(m_fd, buf, sizeof(buf), 0);
	if (n == 0)
	{
		//�Զ˹ر�����
		//����Ҳ�ر����ӣ����Ƴ������¼�
		onClose();
	}
	else if (n < 0)
	{
		if (errno == EINTR /*|| errno == EWOULDBLOCK || errno == EAGAIN*/)
		{
			return;
		}

		//����������ǳ����ˣ��ر�����
		return;
	}
	
	//�����յ����ݰ�
	m_recvBuf.append(buf, n);

	//������ص���������session��onRead����
	m_readCallback(m_recvBuf);
}

void TCPConnection::onWrite()
{
	if (!m_enableWrite)
		return;

	while (true)
	{
		int n = ::send(m_fd, m_sendBuf, m_sendBuf.remaining(), 0);
		if (n == 0)
		{
			//�Զ˹ر�����
			//TODO:����Ҳ�ر�����
			return ;
		}
		else if (n < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else if (errno == EWOULDBLOCK || errno == EAGAIN)
			{
				//��ǰ����tcp����̫С�����ݷ�����ȥ��
				m_writeCallback();

				unregisterWriteEvent();

				return ;
			}

			//����������ǳ����ˣ��ر�����
			return ;
		}

		//���ͳɹ���
		m_sendBuf.erase(n);
		if (m_sendBuf.isEmpty())
		{
			return ;
		}
	}
}

void TCPConnection::onClose()
{
	std::cout << "TCPConnection::onClose() " << m_fd << std::endl;

	//�ر�ע���¼�
	unregisterAllEvent();

	//��ҵ���һ�����������ɵ���ɵ���,�����Լ�������ָ��,������ɵ�session����
	//m_closeCallback - > Calc24Session::onClose() - > Calc24Server::onDisconnected - > ��map���Ƴ�Session����
	//- > ����session���� - > ������TCPConnection���ر�fd
	m_closeCallback(shared_from_this());
	
}

void TCPConnection::enableReadWrite(bool read, bool write)
{
	m_enableRead = read;
	m_enableWrite = write;
}

void TCPConnection::registerWriteEvent()
{
	if (m_registerWriteEvent)
		return;
	
	//��IO���ú���ע��д�¼�
	m_spEventLoop->registerWriteEvent(m_fd, this, true);
}

void TCPConnection::unregisterWriteEvent()
{
	if (!m_registerWriteEvent)
		return;

	//��IO���ú�����ע��д�¼�
	m_spEventLoop->unregisterWriteEvent(m_fd, this, false);
	m_registerWriteEvent = false;
}

void TCPConnection::unregisterAllEvent()
{
	//��IO���ú�����ע�����ж�д�¼�
	m_spEventLoop->unregisterAllEvents(m_fd, this);
	m_registerWriteEvent = false;
}

bool TCPConnection::isCallableInOwnerThread()
{
	if (std::this_thread::get_id() == m_spEventLoop->getThreadId())
		return true;
	else
		return false;
}