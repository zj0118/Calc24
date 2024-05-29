<<<<<<< HEAD
﻿ // Calc24Server.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
=======
﻿// Calc24Server.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
>>>>>>> 9b9d464b20f7d45f9bc97b86d1a72cd23088b59e
//

#include <iostream>
#include "TCPServer.h"

int main()
{
<<<<<<< HEAD
    TCPServer tcpServer;
    if (!tcpServer.init("127.0.0.1", 8888))
    {
        std::cout << "tcpServer.init failed" << std::endl;
        return 0;
    }

    return 1;
=======
	TCPServer tcpServer;
	if (!tcpServer.init("127.0.0.1", 8888))
	{
		std::cout << "tcpServer.init failed" << std::endl;
		return 0;
	}

	tcpServer.start();

	return 1;
>>>>>>> 9b9d464b20f7d45f9bc97b86d1a72cd23088b59e

}
