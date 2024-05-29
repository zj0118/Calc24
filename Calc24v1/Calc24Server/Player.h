#pragma once

#include <string>
#include <memory>

#define NO_PLAYER_ON_SEAT -1

class Player;
 
//游戏桌
struct Desk {
	int id;
	std::weak_ptr<Player> spPlayer1;
	std::weak_ptr<Player> spPlayer2;
	std::weak_ptr<Player> spPlayer3;
	//牌信息
	std::string sendCards;
};

class Player final {
public:
	Player(int clientfd);
	~Player();
	//拷贝构造
	Player(const Player& pl) = delete;
	//移动构造

	int getClientfd() const;
	void setDesk(const std::shared_ptr<Desk> spDesk);
	
	void setReady(bool ready);
	bool getReady() const;
	
	//是否已经发过牌
	bool isCardsSent() const;
	//发送欢迎信息
	bool senWelcomeMsg();
	//发牌
	bool sendCards();
	bool recvData();
	//解包
	void handleClientMsg(std::string& currentMsg);
	//
	bool sendMsgToClient(const std::string& msg);
	void removeSelfFromDesk();

private:
	const int								m_clientfd;
	//玩家所在桌是否人满可以进行发牌了
	bool									m_ready{ false };
	bool									m_cardsSent{ false };
	
	std::string								m_recvBuf;
	//TODO:为什么要用weak_ptr？因为玩家不能控制Desk的生命周期
	std::weak_ptr<Desk>						m_spDesk;
};