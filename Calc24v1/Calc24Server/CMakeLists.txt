cmake_minimum_required(VERSION 2.6)

project (CALC24_SERVER)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g -Wall -O0 -Wno-unused-variable -pthread")

#link_directories(
#    ${PROJECT_SOURCE_DIR}/lib
#	/usr/lib64/mysql/
#)

set(srcs 
<<<<<<< HEAD
=======
Player.cpp
>>>>>>> 9b9d464b20f7d45f9bc97b86d1a72cd23088b59e
TCPServer.cpp
Calc24Server.cpp
)

add_executable(calc24server ${srcs})
#光包含库目录是没用的，还必须使用TARGET_LINK_LIBRARIES链接该库
TARGET_LINK_LIBRARIES(calc24server)