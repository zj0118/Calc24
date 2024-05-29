/*
* 2024.03.14
* 自定义缓冲区类
*/

#pragma once

#include <string>


class ByteBuffer {
public:
	ByteBuffer() = default;
	~ByteBuffer() = default;

	//待完成
	ByteBuffer(const ByteBuffer& rhs);
	ByteBuffer& operator=(const ByteBuffer& rhs);

	ByteBuffer(const ByteBuffer&& rhs);
	ByteBuffer& operator=(ByteBuffer&& rhs);

public:
	operator const char* ();

	//向缓冲区添加东西
	void append(const char* buf, int bufLen);
	//取包
	void retrieve(std::string& outBuf, size_t bufLen = 0);
	size_t retrieve(char* buf, size_t bufLen);
	//看看内容(偷窥数据)
	size_t peek(char* buf, size_t bufLen);

	void erase(size_t bufLen = 0);

	//寻找回车\n
	size_t fiindLF();

	//返回缓冲区的剩余量
	size_t remaining();

	//清零所有buffer
	void clear();

	bool isEmpty();
private:
	std::string				m_internalBuf;
};