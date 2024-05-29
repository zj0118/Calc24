#pragma once

#include <atomic>
//TODO:头文件包含方式待改进
#include "Calc24Protocol.h"
#include "TCPConnection.h"

class Calc24Server;

#define SESSION_STATUS_IDLE			0//玩家空闲状态
#define SESSION_STATUS_READY		1//玩家准备状态
#define SESSION_STATUS_IN_GAME		2//发完牌，游戏状态

enum class DecodePackageResult {
	DecodePackageSuccess,
	DecodePackageFailed,
	DecodePackageWantMoreData
};

class Calc24Session: private std::enable_shared_from_this<Calc24Session> {
public:
	Calc24Session(Calc24Server* pServer, std::shared_ptr<TCPConnection>&& spConn);
	~Calc24Session();

	void onRead(ByteBuffer& recvBuf);
	void onWrite();
	void onClose();

	//业务代码：
	void sendWelcomeMsg();
	void sendMsg(const std::string& msg);
	//通知玩家准备
	void notifyUserToBeReady();
	void forceClose();
	void sendCards(const std::string& cards, int64_t deskID);

	int32_t getId() const
	{
		return m_id;
	}

	bool isReady() const
	{
		return m_status == SESSION_STATUS_READY;
	}

	int8_t getStatus() const{
		return m_status;
	}

	int64_t getDeskID() const {
		return m_deskID;
	}
	//回到空闲状态
	void resetToIdle();

private:
	static int generateId();

private:
	DecodePackageResult decodePackage(ByteBuffer& recvBuf);
	bool processPackage(const std::string package);
	bool processChatMsg(const std::string package);
	bool processCmd(const std::string package);
	

private:
	int32_t										m_id;
	int64_t										m_timerID;
	std::atomic<int64_t>						m_deskID{ 0 };
	//原子变量，线程安全
	std::atomic<int8_t>							m_status{ SESSION_STATUS_IDLE };

	Calc24Server*								m_pServer;
	std::shared_ptr<TCPConnection>				m_spConn;
};