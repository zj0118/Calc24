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
	//不能直接发，要判断当前调用的线程和当前TCPConnection是否属于同一个线程，如果是则直接发送，如果不是，则交给Connection所属的线程发送
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
		//这里无法通过编译的原因是：bind绑定的参数是不定参数，接受0~n个参数，由于tcpconnection存在多个，所以编译器无法判断要选哪个进行绑定
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
			//对端关闭了连接
			//TODO: 我们也关闭连接
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
				//当前由于tcp窗口太小，数据发不出去了
				m_writeCallback();
				//注册写事件
				registerWriteEvent();

				return true;
			}

			//其他情况就是出错了，关闭连接
			onClose();
			return false;
		}

		//发送成功了
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

	//收数据
	char buf[1024];
	int n = ::recv(m_fd, buf, sizeof(buf), 0);
	if (n == 0)
	{
		//对端关闭连接
		//我们也关闭连接，先移除所有事件
		onClose();
	}
	else if (n < 0)
	{
		if (errno == EINTR /*|| errno == EWOULDBLOCK || errno == EAGAIN*/)
		{
			return;
		}

		//其他情况就是出错了，关闭连接
		return;
	}
	
	//正常收到数据包
	m_recvBuf.append(buf, n);

	//解包，回调函数调用session的onRead（）
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
			//对端关闭连接
			//TODO:我们也关闭连接
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
				//当前由于tcp窗口太小，数据发不出去了
				m_writeCallback();

				unregisterWriteEvent();

				return ;
			}

			//其他情况就是出错了，关闭连接
			return ;
		}

		//发送成功了
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

	//关闭注册事件
	unregisterAllEvent();

	//给业务层一个机会让他干点想干的事,返回自己的智能指针,结束后干掉session对象
	//m_closeCallback - > Calc24Session::onClose() - > Calc24Server::onDisconnected - > 从map中移除Session对象
	//- > 析构session对象 - > 先析构TCPConnection，关闭fd
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
	
	//向IO复用函数注册写事件
	m_spEventLoop->registerWriteEvent(m_fd, this, true);
}

void TCPConnection::unregisterWriteEvent()
{
	if (!m_registerWriteEvent)
		return;

	//向IO复用函数反注册写事件
	m_spEventLoop->unregisterWriteEvent(m_fd, this, false);
	m_registerWriteEvent = false;
}

void TCPConnection::unregisterAllEvent()
{
	//向IO复用函数反注册所有读写事件
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