#include <iostream>

#include <string.h>

#include "AsyncLog.h"
#include "TCPServer.h"
#include "Calc24Server.h"

int main()
{
    std::string logFileFullPath = "logs/calc24";
    if (!CAsyncLog::init(logFileFullPath.c_str()))
    {
        LOGF("日志初始化失败");
    }

    Calc24Server calc24Server;
    if (!calc24Server.init(5, "0.0.0.0", 8888))
    {
        return 0;
    }

    return 1;
}