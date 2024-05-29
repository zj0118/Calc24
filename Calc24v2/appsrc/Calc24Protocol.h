#pragma once

#include <cstdint>

enum class MsgType {
	MsgTypeWelcome,
	MsgTypeInitCards,
	MsgTypeChatMsg
};

struct MsgHeader {
	uint32_t packageSize;
	uint32_t msgType;
};

struct WelcomeMsg {
	char welconeMsg[32];
};