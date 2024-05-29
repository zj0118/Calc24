#pragma once

<<<<<<< HEAD
#include<string>

class TCPServer 
=======
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <thread>
#include <vector>

class Player;
class Desk;

class TCPServer
>>>>>>> 9b9d464b20f7d45f9bc97b86d1a72cd23088b59e
{
public:
	TCPServer() = default;
	~TCPServer() = default;

	TCPServer(const TCPServer& rhs) = delete;
	TCPServer& operator=(const TCPServer& rhs) = delete;

	//�ƶ��������캯��
	TCPServer(const TCPServer&& rhs) = delete;
	//�ƶ�operate=
	TCPServer&& operator=(const TCPServer&& rhs) = delete;

public:
<<<<<<< HEAD
	bool init(const std::string& ip, uint16_t port);

=======

	bool init(const std::string& ip, uint16_t port);
	void start();
	void clientThreadFunc(std::shared_ptr<Player> spPlayer, std::shared_ptr<Desk> spOwnerDesk);


	//����Ҽ���
	void newPlayerJoined(int clientfd);
	//����4����
	void generateCards(const std::shared_ptr<Desk>& spDesk);
	//ת����Ϣ�������ͻ���
	void sendMsgToOtherClients(int msgOwnerClientfd, const std::string& msg);

private:
	int														m_listenfd{ -1 };

	//TODO:�Ƿ������unique_ptr?
	std::unordered_map<int, std::shared_ptr<std::thread>>	m_clientfdToThreads;
	//����m_clientfdToThreads����
	//std::mutex												m_mutexForClientfdToThreads;
	////key = clientfd,value = ָ��Player������ָ��
	std::unordered_map<int, std::shared_ptr<Player>>		m_players;
	std::mutex												m_mutexForPlayers;

	std::vector<std::shared_ptr<Desk>>						m_deskInfo;
>>>>>>> 9b9d464b20f7d45f9bc97b86d1a72cd23088b59e
};