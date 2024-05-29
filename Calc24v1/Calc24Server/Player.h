#pragma once

#include <string>
#include <memory>

#define NO_PLAYER_ON_SEAT -1

class Player;
 
//��Ϸ��
struct Desk {
	int id;
	std::weak_ptr<Player> spPlayer1;
	std::weak_ptr<Player> spPlayer2;
	std::weak_ptr<Player> spPlayer3;
	//����Ϣ
	std::string sendCards;
};

class Player final {
public:
	Player(int clientfd);
	~Player();
	//��������
	Player(const Player& pl) = delete;
	//�ƶ�����

	int getClientfd() const;
	void setDesk(const std::shared_ptr<Desk> spDesk);
	
	void setReady(bool ready);
	bool getReady() const;
	
	//�Ƿ��Ѿ�������
	bool isCardsSent() const;
	//���ͻ�ӭ��Ϣ
	bool senWelcomeMsg();
	//����
	bool sendCards();
	bool recvData();
	//���
	void handleClientMsg(std::string& currentMsg);
	//
	bool sendMsgToClient(const std::string& msg);
	void removeSelfFromDesk();

private:
	const int								m_clientfd;
	//����������Ƿ��������Խ��з�����
	bool									m_ready{ false };
	bool									m_cardsSent{ false };
	
	std::string								m_recvBuf;
	//TODO:ΪʲôҪ��weak_ptr����Ϊ��Ҳ��ܿ���Desk����������
	std::weak_ptr<Desk>						m_spDesk;
};