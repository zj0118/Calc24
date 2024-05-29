/*
* zh 2024.5.14
* 用来包装业务的对象
*/
#pragma once

#include "TCPServer.h"

#include <string.h>
#include <vector>

#include "Calc24Session.h"

class Calc24Server final {
public:
	Calc24Server() = default;
	~Calc24Server() = default;

	bool init(int32_t threadNum, const std::string& ip, uint16_t port);
	void uinit();

	void sendAll(const std::string& msg, bool includeSelf, int32_t id, int status, int64_t deskID);
	void onDisconnected(int32_t id);

	void resetSessionToIdle(int64_t deskID);

private:
	void onConnected(std::shared_ptr<TCPConnection>& spConn);
	void generateCards(std::string& newCards, int64_t& deskID);
	static int64_t generateDeskID();

private:
	int64_t																m_checkHandupTimerID{ 0 };
	//组合的方式
	TCPServer															m_tcpServer;
	//key->session的id
	std::unordered_map<int32_t,std::shared_ptr<Calc24Session>>			m_sessions;
	std::vector<std::shared_ptr<Calc24Session>>							m_pendingToDeleteSessions;
};