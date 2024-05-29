#include "Player.h"

#include<string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include<unistd.h>

#include<iostream>

#define PALYER_WELCOME_MSG "Welcome To Calc24 Game!\n"

Player::Player(int clientfd) :m_clientfd(clientfd)
{

}

Player::~Player()
{
	std::cout << "close clientfd[" << m_clientfd << "]" << std::endl;
	::close(m_clientfd);
}

int Player::getClientfd() const
{
	return m_clientfd;
}

void Player::setDesk(const std::shared_ptr<Desk> spDesk)
{ 
	m_spDesk = spDesk;
}

void Player::setReady(bool ready)
{
	m_ready = ready;
}

bool Player::getReady() const
{
	return m_ready;
}

//�Ƿ��Ѿ�������
bool Player::isCardsSent() const
{
	return m_cardsSent;
}

bool Player::senWelcomeMsg()
{
	int sentLength = ::send(m_clientfd, PALYER_WELCOME_MSG, strlen(PALYER_WELCOME_MSG), 0);
	return sentLength == static_cast<int>(strlen(PALYER_WELCOME_MSG));
}

bool Player::sendCards()
{
	std::shared_ptr<Desk> spDesk = m_spDesk.lock();
	if (spDesk == nullptr)
	{
		//��Ҵ��ڲ����κ�һ�������
		return false;
	}

	int cardsStrLen = static_cast<int>(spDesk->sendCards.size());
	int sentLength = ::send(m_clientfd, spDesk->sendCards.c_str(), cardsStrLen, 0);
	
	if (sentLength == cardsStrLen)
	{
		m_cardsSent = true;
	}
	else
	{
		m_cardsSent = false;
	}

	return m_cardsSent;
}

bool Player::sendMsgToClient(const std::string& msg)
{
	//std::lock_guard<std::mutex> scopedLock(*m_clientfdToMutex[clientfd]);
	//TODO:������Ϣ̫����һ���Է�����ȥ����Ҫ����
	int sentLength = ::send(m_clientfd, msg.c_str(), msg.length(), 0);

	return sentLength == static_cast<int>(msg.length());
}

bool Player::recvData()
{
	char clientMsg[32] = { 0 };
	int clientMsgLength = ::recv(m_clientfd, clientMsg, sizeof(clientMsg) / sizeof(clientMsg[0]), 0);
	if (clientMsgLength == 0)
	{
		//�Զ˹ر�������
		return false;
	}
	if (clientMsgLength < 0)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
		{
			
			return true;
		}
		else
		{
			//���ӳ�����
			return false;
		}
	}

	m_recvBuf.append(clientMsg, clientMsgLength);
	return true;
}

//ABC\nDEF\nGH
void Player::handleClientMsg(std::string& currentMsg)
{
	if (m_recvBuf.empty())
		return;

	size_t index = 0;
	//��¼��һ�ΰ���λ��
	//int lastPackagePos = 0;
	while (true)
	{
		//nc���������\n������,����Ҳ�����õ���һ�������İ�
		if (m_recvBuf[index] == '\n')
		{
			currentMsg = m_recvBuf.substr(0, index + 1);
			m_recvBuf.erase(0, index + 1);
			std::cout << "client[" << m_clientfd << "] Says:" << currentMsg << std::endl;

			//�õ������İ����˳�
			return;
		}

		if (index + 1 >= m_recvBuf.length())
			return;

		++index;
	}
}

void Player::removeSelfFromDesk()
{
	auto spDesk = m_spDesk.lock();
	if (spDesk != nullptr)
	{
		auto spPlayer = spDesk->spPlayer1.lock();
		if (spPlayer->getClientfd() == m_clientfd)
		{
			//spDesk->spPlayer1 = nullptr;
		}
	}
}