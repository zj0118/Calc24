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

	//TODO:setAcceptCallbackʵ����ֻ����һ�����������Ǵ���������������
	m_pAcceptor->setAcceptCallback(std::bind(&TCPServer::onAccept,this,std::placeholders::_1));

	return true;
}
void TCPServer::uinit()
{
	//ͣ���߳�
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
	//��ѯ���ƽ��µ�clientfd���ص���ͬ��eventloop��
	std::shared_ptr<EventLoop> spEventLoop = m_threadPool.getNextEventLoop();
	auto spTCPConnection = std::make_shared<TCPConnection>(clientfd,spEventLoop);

	std::cout << "TCPServer::onAccept() " << clientfd << ", ThreadId : " << spEventLoop->getThreadId() << std::endl;
	//��������clientfd���ص�IO���ú�����
	spTCPConnection->startRead();

	//�û��Ļص���������m_connectedCallback - > Calc24Server.onConnected()
	m_connectedCallback(spTCPConnection);
}

void TCPServer::onDisconnected(const std::shared_ptr<TCPConnection>& spConn)
{
	m_disconnectedCallback(spConn);
}