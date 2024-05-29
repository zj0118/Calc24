#include "Calc24Session.h"

#include <functional>
#include <iostream>
#include <sstream>
#include <string.h>

#include "Calc24Server.h"

#define MAX_MSG_LENGTH 64

Calc24Session::Calc24Session(Calc24Server* pServer, std::shared_ptr<TCPConnection>&& spConn)
	:m_pServer(pServer), m_spConn(std::move(spConn))
{
	m_id = Calc24Session::generateId();

	m_spConn->setReadCallback(std::bind(&Calc24Session::onRead, this,std::placeholders::_1));
	m_spConn->setWriteCallback(std::bind(&Calc24Session::onWrite, this));
	m_spConn->setCloseCallback(std::bind(&Calc24Session::onClose, this));

	std::cout << "Calc24Session::ctor " << m_id << std::endl;
}

Calc24Session::~Calc24Session()
{
	std::cout << "Calc24Session::dtor " << m_id << std::endl;
}

void Calc24Session::onRead(ByteBuffer& recvBuf)
{
	while (true)
	{
		//解包
		DecodePackageResult result = decodePackage(recvBuf);
		if (result == DecodePackageResult::DecodePackageWantMoreData)
		{
			return;
		}
		else if (result == DecodePackageResult::DecodePackageFailed)
		{
			forceClose();
			return;
		}

		//继续下一个包解析
		
	}
}

void Calc24Session::onWrite()
{

}

void Calc24Session::onClose()
{
	//Session对象做自己一些要清理的逻辑,这个地方shared_from_this()有问题报错，因为不能自己析构自己，留着还有用，应该延迟析构
	//m_pServer->onDisconnected(shared_from_this());
	m_spConn->getEventLoop()->removeTimer(m_timerID);
	m_pServer->onDisconnected(m_id);
}

void Calc24Session::sendWelcomeMsg()
{
	const std::string& welcomeMsg = "Welcome To Calc24 Game.\n";
	m_spConn->send(welcomeMsg);
}

void Calc24Session::sendMsg(const std::string& msg)
{
	m_spConn->send(msg);
}

void Calc24Session::notifyUserToBeReady()
{
	m_timerID = m_spConn->getEventLoop()->addTimer(5000, true, 0, [this](int64_t timerID) -> void {
		if(m_status == SESSION_STATUS_IDLE)
			m_spConn->send("please hand up!\n");
		else {
			m_spConn->getEventLoop()->removeTimer(timerID);
		}
	});
}

void Calc24Session::forceClose()
{
	m_spConn->onClose();
}

void Calc24Session::sendCards(const std::string& cards, int64_t deskID)
{
	m_spConn->send(cards);
	m_deskID = deskID;
	m_status = SESSION_STATUS_IN_GAME;
}

DecodePackageResult Calc24Session::decodePackage(ByteBuffer& recvBuf)
{
	size_t position = recvBuf.fiindLF();
	//找不到\n
	if (position == std::string::npos)
	{
		if (recvBuf.remaining() > MAX_MSG_LENGTH)
		{
			return DecodePackageResult::DecodePackageFailed;
		}
		return DecodePackageResult::DecodePackageWantMoreData;
	}

	std::string currentPackage;
	recvBuf.retrieve(currentPackage, position + 1);

	//发送给其他用户
	processPackage(currentPackage);
	
	return DecodePackageResult::DecodePackageSuccess;
}

bool Calc24Session::processPackage(const std::string package)
{
	if (package.empty())
		return true;
	
	//规定第一个为！则是特殊指令
	if (package[0] != '!')
	{
		//聊天消息
		return processChatMsg(package);
	}
	else
	{
		return processCmd(package);
	}
}

int Calc24Session::generateId()
{
	static std::atomic<int32_t> m_baseId;
	int32_t result = ++m_baseId;
	return result;
}

bool Calc24Session::processChatMsg(const std::string package)
{
	std::cout << "fd " << m_spConn->getfd() << ", client[" << m_id << "] says: " << package << std::endl;

	//std::string msgWithPrefix("client[" + std::to_string(m_id) + "] says: " + package);
	//下面效率更高
	std::ostringstream msgWithPrefix;
	msgWithPrefix << "client[";
	msgWithPrefix << m_id;
	msgWithPrefix << "] says: ";
	msgWithPrefix << package;
	m_pServer->sendAll(msgWithPrefix.str(), false, m_id, m_status, m_deskID);

	//TODO:这个返回值没意义，可以改成void
	return true;
}

bool Calc24Session::processCmd(const std::string package)
{
	//!ready表示已经准备好,TODO:不能用硬编码，改成常量！
	//!2 3 4 5 表示对24点游戏的结果进行计算
	if (package.substr(0, 6) == "!ready")
	{
		m_status = SESSION_STATUS_READY;
		return true;
	}

	//假设“！xyz”是正确答案
	if (package == "!xyz\n")
	{
		//告诉当前玩家是正确的，结束游戏
		std::string rightAnswer = "Congratulations! Your answer is right! Game Over!\n";
		m_spConn->send(rightAnswer);
		//告诉其他玩家已经有玩家算出结果，结束游戏
		std::string notifyOthersGameOverMsg = "One palyer has already given the right answer! Game Over!\n";
		m_pServer->sendAll(notifyOthersGameOverMsg, false, m_id, m_status, m_deskID);

		//让所有当前桌子上的玩家回到空闲状态
		m_pServer->resetSessionToIdle(m_deskID);
	}
	else
	{
		std::string wrongAnswer = "Your answer is wrong, please try again!\n";
		m_spConn->send(wrongAnswer);
	}

	return false;
}

void Calc24Session::resetToIdle()
{
	m_deskID = 0;
	m_status = SESSION_STATUS_IDLE;
	notifyUserToBeReady();
}