#include "Acceptor.h"

#include <iostream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

Acceptor::Acceptor(EventLoop* pEventLoop):m_pEventLoop(pEventLoop)
{
}

Acceptor::~Acceptor()
{
	if (m_listenfd != -1)
	{
		::close(m_listenfd);
		m_listenfd = -1;
	}

}

void Acceptor::onRead()
{
	while (true)
	{
		struct sockaddr clientaddr;
		socklen_t clientaddrLen = sizeof(clientaddr);

		//4. ���ܿͻ������ӣ�������
		int clientfd = ::accept4(m_listenfd, (struct sockaddr*)&clientaddr, &clientaddrLen, SOCK_NONBLOCK);

		if (clientfd > 0)
		{
			//�ɹ���������
			//m_acceptCallback->TCPServer::onAccept()
			m_acceptCallback(clientfd);

		}
		else if(clientfd == -1)
		{
			if (errno == EWOULDBLOCK)
			{
				//û��������
				return;
			}
			else
			{
				//������
				return;
			}
		}
	}
	
}

bool Acceptor::startListen(const std::string& ip, uint16_t port)
{
	//1.����һ������socket
	m_listenfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (m_listenfd == -1)
	{
		return false;
	}

	//�������ٸ���ip��ַ
	int optval = 1;
	//TODO: �ж�һ�������������Ƿ���óɹ�
	::setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
	::setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));

	//2.��ʼ����������ַ
	struct sockaddr_in bindaddr;
	bindaddr.sin_family = AF_INET;
	if (ip == "")
	{
		bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		bindaddr.sin_addr.s_addr = inet_addr(ip.c_str());
	}
	
	bindaddr.sin_port = htons(port);
	if (::bind(m_listenfd, (struct sockaddr*)&bindaddr, sizeof(bindaddr)) == -1)
	{
		std::cout << "bind listen socket error." << std::endl;
		return false;
	}

	//3.��������
	if (::listen(m_listenfd, SOMAXCONN) == -1)
	{
		std::cout << "listen error." << std::endl;
		return false;
	}

	//������fd���ص�eventloop�ϣ���loop
	m_pEventLoop->registerReadEvent(m_listenfd, this, true);

	return true;
}