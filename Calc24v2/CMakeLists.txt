cmake_minimum_required(VERSION 2.8)

project (CALC24_SERVER)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -Wall -O0 -Wno-unused-variable -pthread")

link_directories(
	/lib64
)
include_directories(
    utils/
	net/
)

set(net_srcs 
net/Accept.cpp
net/ByteBuffer.cpp
net/Epoll.cpp
net/EventLoop.cpp
net/Poll.cpp
net/Select.cpp
net/TCPConnection.cpp
net/TCPServer.cpp
net/ThreadPoll.cpp
net/WakeupEventDispatcher.cpp
net/Timer.cpp
)

set(util_srcs
utils/util.cpp
)

set(app_srcs
appsrc/AsyncLog.cpp
appsrc/Calc24Server.cpp
appsrc/Calc24Session.cpp
appsrc/main.cpp
)


add_executable(calc24server ${net_srcs} ${app_srcs} ${util_srcs})
#光包含库目录是没用的，还必须使用TARGET_LINK_LIBRARIES链接该库
TARGET_LINK_LIBRARIES(calc24server)