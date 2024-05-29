#pragma once


#include <functional>
#include <string>
#include <unordered_map>

#include "Acceptor.h"
#include "EventLoop.h"
#include "TCPConnection.h"
#include "ThreadPoll.h"

//�ص�����
using onConnectedCallback = std::function<void(std::shared_ptr<TCPConnection>& spConn)>;
using onDisconnectedCallback = std::function<void(const std::shared_ptr<TCPConnection>& spConn)>;

class TCPServer {
public:
	TCPServer() = default;
	virtual ~TCPServer() = default;

	bool init(int32_t threadNum,const std::string& ip, uint16_t port);
	void uinit();

	void start();

	//���ûص�����
	//m_connectedCallback��Ҫ����Ϊָ��ҵ������ӻص�������ҵ����������������ʱ��һЩ�����Ļ���
	void setConnectedCallback(onConnectedCallback&& callback)
	{
		m_connectedCallback = std::move(callback);
	}
	
	void setDisconnectedCallback(onDisconnectedCallback&& callback)
	{
		m_disconnectedCallback = std::move(callback);
	}

	EventLoop& getBaseEventLoop()
	{
		return m_baseEventLoop;
	}

private:
	void onAccept(int clientfd);
	void onDisconnected(const std::shared_ptr<TCPConnection>& spConn);

private:
	//int					m_listenfd{ -1 };
	std::string													m_ip;
	uint16_t													m_port;

	ThreadPoll													m_threadPool;
	EventLoop													m_baseEventLoop;
	Acceptor*													m_pAcceptor;

	std::unordered_map<int, std::shared_ptr<TCPConnection>>		m_connections;
	onConnectedCallback											m_connectedCallback;
	onDisconnectedCallback										m_disconnectedCallback;
};