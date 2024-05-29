#include "TCPServer.h"

#include <functional>
#include <iostream>

#include "TCPConnection.h"

bool TCPServer::init(int32_t threadNum, const std::string& ip, uint16_t port)
{
	m_threadPool.start(threadNum);
	if (ip.empty())
	{
		std::cout << "ip is empty" << std::endl;
		return false;
	}

	if (port < 0)
	{
		std::cout << "port is empty" << std::endl;
		return false;
	}

	m_ip = ip;
	m_port = port;
	m_baseEventLoop.init();

	m_pAcceptor = new Acceptor(&m_baseEventLoop);
	if (!m_pAcceptor->startListen(m_ip, m_port))
	{
		return false;
	}

	//TODO:setAcceptCallback实际上只接受一个参数，但是传了两个有问题吗？
	m_pAcceptor->setAcceptCallback(std::bind(&TCPServer::onAccept,this,std::placeholders::_1));

	return true;
}
void TCPServer::uinit()
{
	//停掉线程
	m_threadPool.stop();
}

void TCPServer::start()
{
	std::cout << "baseEventLoop threadID :" << std::this_thread::get_id() << std::endl;
	m_baseEventLoop.setThreadId(std::this_thread::get_id());
	m_baseEventLoop.run();
}

//TCPServer::m_disconnectedCallback - >Calc24Server::onDisconnected
//TCPConnection::m_closeCallback - > TCPServer::m_disconnectedCallback
void TCPServer::onAccept(int clientfd)
{
	//轮询机制将新的clientfd挂载到不同的eventloop中
	std::shared_ptr<EventLoop> spEventLoop = m_threadPool.getNextEventLoop();
	auto spTCPConnection = std::make_shared<TCPConnection>(clientfd,spEventLoop);

	std::cout << "TCPServer::onAccept() " << clientfd << ", ThreadId : " << spEventLoop->getThreadId() << std::endl;
	//将创建的clientfd挂载到IO复用函数上
	spTCPConnection->startRead();

	//用户的回调函数，即m_connectedCallback - > Calc24Server.onConnected()
	m_connectedCallback(spTCPConnection);
}

void TCPServer::onDisconnected(const std::shared_ptr<TCPConnection>& spConn)
{
	m_disconnectedCallback(spConn);
}