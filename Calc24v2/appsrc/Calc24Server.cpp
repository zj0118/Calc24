#include "Calc24Server.h"

#include <iostream>

#include "AsyncLog.h"
#include "Calc24Session.h"

bool Calc24Server::init(int32_t threadNum, const std::string& ip, uint16_t port)
{
	//lambda不好调试
	/*m_tcpServer.setConnectedCallback([](std::shared_ptr<TCPConnection>& spConn) -> void {

	});*/
	m_tcpServer.setConnectedCallback(std::bind(&Calc24Server::onConnected, this, std::placeholders::_1));
	if (!m_tcpServer.init(threadNum, ip, port))
		return false;

	////每隔一秒检测是否有足够数量的玩家已经准备好
	m_checkHandupTimerID = m_tcpServer.getBaseEventLoop().addTimer(1000, true, 0, [this](int64_t timerID) -> void {

		/*TODO:为什么不行？
		for (int i = 0; i < m_sessions.size(); ++i)
		{
			if (m_sessions[i]->isHandup())
			{
				m_sessions[i]->initCards();
			}
		}*/
		static constexpr int REQUIRED_MIN_READY_COUNT = 2;
		int readySessionCount = 0;
		
		for (auto& iter : m_sessions)
		{
			if (iter.second->isReady())
			{
				readySessionCount++;
			}
		}

		if (readySessionCount < REQUIRED_MIN_READY_COUNT)
			return;

		//生成牌
		std::string cards;
		int64_t deskID;

		generateCards(cards,deskID);

		for (auto& iter : m_sessions)
		{
			if (iter.second->isReady())
			{
				iter.second->sendCards(cards, deskID);
			}
		}
	});

	LOGI("init completed！");
	m_tcpServer.start();
	
	return true;
}
void Calc24Server::uinit()
{
	if (m_checkHandupTimerID > 0)
	{
		m_tcpServer.getBaseEventLoop().removeTimer(m_checkHandupTimerID);
	}

	m_tcpServer.uinit();
}

void Calc24Server::resetSessionToIdle(int64_t deskID)
{
	for (auto& iter : m_sessions)
	{
		if (iter.second->getDeskID() != deskID)
			continue;
		
		iter.second->resetToIdle();
	}
}

//延迟析构
void Calc24Server::onConnected(std::shared_ptr<TCPConnection>& spConn)
{
	//先删除无效的Calc24Session:
	m_pendingToDeleteSessions.clear();

	auto spSession = std::make_shared<Calc24Session>(this, std::move(spConn));
	//发送欢迎信息
	spSession->sendWelcomeMsg();
	LOGI("spSession-》sendWelcomeMsg");

	spSession->notifyUserToBeReady();
	LOGI("spSession-》notifyUserToBeReady");
	m_sessions.emplace(spSession->getId(), std::move(spSession));
}

void Calc24Server::sendAll(const std::string& msg, bool includeSelf, int32_t id, int status, int64_t deskID)
{
	for (const auto& iter : m_sessions)
	{
		if (!includeSelf && iter.second->getId() == id)
			continue;

		if (iter.second->getStatus() != status || iter.second->getDeskID() != deskID)
			continue;

		iter.second->sendMsg(msg);
	}
}

void Calc24Server::onDisconnected(int32_t id)
{
	LOGI("Calc24Server::onDisconnected");
	auto iter = m_sessions.find(id);
	if (iter != m_sessions.end())
	{
		auto pendingSession = iter->second;
		m_pendingToDeleteSessions.push_back(std::move(pendingSession));
		m_sessions.erase(iter);
		//std::cout << "Calc24Server::onDisconnected" << std::endl;
	}
}

void Calc24Server::generateCards(std::string& newCards, int64_t& deskID)
{
	//constexpr c++11
	static constexpr char allCars[] = { 'A','2','3','4','5','6','7','8','9','X','J','Q','K','w','W' };
	static constexpr int allCarsCount = sizeof(allCars) / sizeof(allCars[0]);
	int index1 = rand() % allCarsCount;
	int index2 = rand() % allCarsCount;
	int index3 = rand() % allCarsCount;
	int index4 = rand() % allCarsCount;

	char sznewCards[24];
	sprintf(sznewCards, "Your cards is: %c %c %c %c\n", allCars[index1], allCars[index2], allCars[index3], allCars[index4]);
	newCards = sznewCards;

	deskID = Calc24Server::generateDeskID();
}

int64_t Calc24Server::generateDeskID()
{
	static std::atomic<int64_t> m_baseId;
	int64_t deskID = ++m_baseId;
	return deskID;
}