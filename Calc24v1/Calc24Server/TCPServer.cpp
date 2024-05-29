#include "TCPServer.h"

<<<<<<< HEAD
bool TCPServer::init(const std::string& ip, uint16_t port) 
{
	//��������socket

	//�󶨶˿ںź�ip

	//��������

	return false;
=======
#include <arpa/inet.h>//htonl,htons,ntohl,ntohs
#include <stdlib.h>
#include <string.h>//sizeof
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <time.h>
#include <unistd.h>//close,sleep

#include <functional>
#include <iostream>

#include "Player.h"


bool TCPServer::init(const std::string& ip, uint16_t port)
{
	//�������
	srand(time(nullptr));

	//1.����һ������socket
	m_listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
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
	bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
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

	return true;
}

void TCPServer::start()
{
	while (true)
	{
		struct sockaddr_in clientaddr;
		socklen_t clientaddrlen = sizeof(clientaddr);
		
		//4. ���ܿͻ������ӣ�������
		int clientfd = ::accept4(m_listenfd, (struct sockaddr*)&clientaddr, &clientaddrlen,SOCK_NONBLOCK);

		if (clientfd == -1)
		{
			std::cout << "accept error." << std::endl;
			return;
		}

		//���̲߳���
		newPlayerJoined(clientfd);
	}
}

//ÿһ������Ҽ��룬�������ӣ�����һ���µ��̴߳���
void TCPServer::newPlayerJoined(int clientfd)
{
	auto spCurrentPlayer = std::make_shared<Player>(clientfd);

	std::shared_ptr<Desk> spCurrentFullDesk;

	auto iter = m_deskInfo.rbegin();
	if (iter == m_deskInfo.rend())
	{
		//��һ����Ҽ���
		//���ӵ�id��1��ʼ
		auto spDesk = std::make_shared<Desk>();
		//TODO:1�ĳɳ���
		spDesk->id = 1;
		spDesk->spPlayer1 = spCurrentPlayer;
		spCurrentPlayer->setDesk(spDesk);

		m_deskInfo.push_back(std::move(spDesk));
	}
	else
	{
		//TODO:weak_ptr expired()
		if ((*iter)->spPlayer1.expired())
		{
			(*iter)->spPlayer1 = spCurrentPlayer;
		}
		else if ((*iter)->spPlayer2.expired())
		{
			(*iter)->spPlayer2 = spCurrentPlayer;
		}
		else if ((*iter)->spPlayer3.expired())
		{
			(*iter)->spPlayer3 = spCurrentPlayer;

			//����������Һ��������������
			spCurrentFullDesk = *iter;
		}
		else
		{
			//������
			auto spDesk = std::make_shared<Desk>();
			spDesk->id = m_deskInfo.size() + 1;
			spDesk->spPlayer1 = spCurrentPlayer;
			spCurrentPlayer->setDesk(spDesk);

			m_deskInfo.push_back(std::move(spDesk));
		}
	}

	if (spCurrentFullDesk != nullptr)
	{		
		//�����˾ͷ���
		std::shared_ptr<Player> sp1 = spCurrentFullDesk->spPlayer1.lock();
		if (sp1 != nullptr)
		{
			sp1->setReady(true);
			sp1->setDesk(spCurrentFullDesk);
		}
			
		
		std::shared_ptr<Player> sp2 = spCurrentFullDesk->spPlayer2.lock();
		if (sp2 != nullptr)
		{
			sp2->setReady(true);
			sp2->setDesk(spCurrentFullDesk);
		}
		
		std::shared_ptr<Player> sp3 = spCurrentFullDesk->spPlayer3.lock();
		if (sp3 != nullptr)
		{
			sp3->setReady(true);
			sp3->setDesk(spCurrentFullDesk);
		}

		//��������
		generateCards(spCurrentFullDesk);
	}

	//��ÿ��Player�������µ��̴߳���
	auto spThread = std::make_shared<std::thread>(std::bind(&TCPServer::clientThreadFunc, this, spCurrentPlayer, spCurrentFullDesk));
	spThread->detach();
	m_clientfdToThreads[clientfd] = std::move(spThread);
	m_players[clientfd] = std::move(spCurrentPlayer);
}

void TCPServer::clientThreadFunc(std::shared_ptr<Player> spPlayer, std::shared_ptr<Desk> spOwnerDesk)
{
	int clientfd = spPlayer->getClientfd();
	std::cout << "new client connected,clientfd: " << clientfd << std::endl;


	if (spPlayer->senWelcomeMsg())
	{
		bool exit = false;
		//�����հ�
		while (!exit)
		{
			if (spPlayer->getReady() && !spPlayer->isCardsSent())
			{
				if (spPlayer->sendCards())
				{
					std::cout << "sendCards to clientfd[" << clientfd << "]successfully" << std::endl;
				}
				else
				{
					std::cout << "sendCards to clientfd[" << clientfd << "]failed" << std::endl;
					exit = true;
					continue;
				}
			}//end if

			//�հ�
			if (!spPlayer->recvData())
			{
				std::cout << "recvData failed, clientfd[" << clientfd << "]" << std::endl;
				exit = true;
				continue;
			}

			//���
			while (true)
			{
				std::string aMsg;
				spPlayer->handleClientMsg(aMsg);
				if (!aMsg.empty())
				{
					std::cout << "clientfd[" << clientfd << "] Says: " << aMsg << std::endl;
					sendMsgToOtherClients(clientfd, aMsg);
				}
				else
				{
					break;
				}
			}//end inner-while
		}//end outer-while
	}

	//����clientfd
	std::lock_guard<std::mutex> scopeLock(m_mutexForPlayers);
	m_players.erase(clientfd);
	m_clientfdToThreads.erase(clientfd);
}

//����������
void TCPServer::generateCards(const std::shared_ptr<Desk>& spDesk)
{
	//constexpr c++11
	static constexpr char allCars[] = { 'A','2','3','4','5','6','7','8','9','X','J','Q','K','w','W' };
	static constexpr int allCarsCount = sizeof(allCars) / sizeof(allCars[0]);
	int index1 = rand() % allCarsCount;
	int index2 = rand() % allCarsCount;
	int index3 = rand() % allCarsCount;
	int index4 = rand() % allCarsCount;

	char newCards[24];
	sprintf(newCards, "Your cards is: %c %c %c %c\n", allCars[index1], allCars[index2], allCars[index3], allCars[index4]);
	spDesk->sendCards.append(newCards,strlen(newCards));
}

void TCPServer::sendMsgToOtherClients(int msgOwnerClientfd, const std::string& msg)
{
	int otherClientfd;
	std::string msgWithOwnerInfo;//����Ϣ��clientfdͷ

	//��ֹ����m_mutexForPlayers��д���ܸ���
	std::lock_guard<std::mutex> scopedLock(m_mutexForPlayers);

	for (const auto& client : m_players)
	{
		otherClientfd = client.first;

		if (otherClientfd == msgOwnerClientfd)
			continue;

		msgWithOwnerInfo = "Client[" + std::to_string(msgOwnerClientfd) + "] Says: " + msg;
		if (!client.second->sendMsgToClient(msgWithOwnerInfo))
		{
			std::cout << "sendMsgTpOtherClients failed,clientfd: " << otherClientfd << std::endl;
		}
	}
>>>>>>> 9b9d464b20f7d45f9bc97b86d1a72cd23088b59e
}