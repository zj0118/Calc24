#pragma once

#include <atomic>
//TODO:ͷ�ļ�������ʽ���Ľ�
#include "Calc24Protocol.h"
#include "TCPConnection.h"

class Calc24Server;

#define SESSION_STATUS_IDLE			0//��ҿ���״̬
#define SESSION_STATUS_READY		1//���׼��״̬
#define SESSION_STATUS_IN_GAME		2//�����ƣ���Ϸ״̬

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

	//ҵ����룺
	void sendWelcomeMsg();
	void sendMsg(const std::string& msg);
	//֪ͨ���׼��
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
	//�ص�����״̬
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
	//ԭ�ӱ������̰߳�ȫ
	std::atomic<int8_t>							m_status{ SESSION_STATUS_IDLE };

	Calc24Server*								m_pServer;
	std::shared_ptr<TCPConnection>				m_spConn;
};