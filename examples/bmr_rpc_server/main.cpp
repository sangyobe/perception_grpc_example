#include <iostream>
#include "rpcServer.h"
#include "bmr.h"
#include <dtCore/src/dtLog/dtLog.h>

BMR bmr;

int main()
{
    dt::Log::Initialize("BMR_test_server"); //, "logs/bmr_rpc_server.txt");
    dt::Log::SetLogLevel(dt::Log::LogLevel::trace);

    RpcServer rpcServer((void *)&(bmr.robotData));
    rpcServer.Run();

    std::atomic<bool> bRun;
    bRun.store(true);
    while (bRun.load()) {
        std::cout << "(type \'q\' to quit) >\n";
        std::string cmd;
        std::cin >> cmd;
        if (cmd == "q" || cmd == "quit") {
            bRun = false;
        }
    }
    rpcServer.Stop();

    dt::Log::Terminate(); // flush all log messages
    return 0;
}