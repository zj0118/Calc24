/*
* 2024.03.14
* �Զ��建������
*/

#pragma once

#include <string>


class ByteBuffer {
public:
	ByteBuffer() = default;
	~ByteBuffer() = default;

	//�����
	ByteBuffer(const ByteBuffer& rhs);
	ByteBuffer& operator=(const ByteBuffer& rhs);

	ByteBuffer(const ByteBuffer&& rhs);
	ByteBuffer& operator=(ByteBuffer&& rhs);

public:
	operator const char* ();

	//�򻺳������Ӷ���
	void append(const char* buf, int bufLen);
	//ȡ��
	void retrieve(std::string& outBuf, size_t bufLen = 0);
	size_t retrieve(char* buf, size_t bufLen);
	//��������(͵������)
	size_t peek(char* buf, size_t bufLen);

	void erase(size_t bufLen = 0);

	//Ѱ�һس�\n
	size_t fiindLF();

	//���ػ�������ʣ����
	size_t remaining();

	//��������buffer
	void clear();

	bool isEmpty();
private:
	std::string				m_internalBuf;
};