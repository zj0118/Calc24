#pragma once

#include <string>

class TCPClient 
{
public:
	TCPClient() = default;
	~TCPClient() = default;

	TCPClient(const TCPClient& rhs) = delete;
	TCPClient& operator=(const TCPClient& rhs) = delete;

	//移动拷贝构造函数
	TCPClient(const TCPClient&& rhs) = delete;
	//移动operate=
	TCPClient&& operator=(const TCPClient&& rhs) = delete;

public:
	bool init(const std::string& serverIp, uint16_t serverPort);

};