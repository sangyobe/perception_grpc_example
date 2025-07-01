#include "emulRobotCommand.h"
#include "pubRobotCommand.h"
#include "rpcClient.h" // main RPC client interface
#include <dtCore/src/dtDAQ/grpc/dtStateSubscriberGrpc.hpp>
#include <dtCore/src/dtLog/dtLog.h>
#include <dtCore/src/dtUtils/dtStringHelper.hpp>

RobotCommandEmulator robotCommandEmul;
std::string svrIp = "127.0.0.1";

int main(int argc, char **argv)
{
    dt::Log::Initialize("nav_command_publisher");
    dt::Log::SetLogLevel(dt::Log::LogLevel::trace);

    if (argc > 1) { svrIp = argv[1]; }
    LOG(info) << "nav_command_publisher, server_ip = " << svrIp;

    std::atomic<bool> bRun;
    bRun.store(true);

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // RobotCommand RPC service client
    //
    std::unique_ptr<RpcClient<dtproto::dtService>> rpcClient = std::make_unique<RpcClient<dtproto::dtService>>(
        dt::Utils::string_format("%s:%d", svrIp.c_str(), 50052));

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // perceived objects publisher (client-side streamer)
    //
    std::thread cmd_publisher = std::thread([&]() {
        double t_ = 0.0;
        double dt_ = 0.1;

        uint64_t cid = rpcClient->template StartCall<PubRobotCommand>(
            (void *)(&robotCommandEmul.cmd));
        std::shared_ptr<PubRobotCommand> call = std::dynamic_pointer_cast<
            PubRobotCommand,
            dt::DAQ::ServiceCallerGrpc<dtproto::dtService>::Call>(
            rpcClient->GetCall(cid));
        if (call == nullptr) return;

        while (bRun.load())
        {
            call->Publish(robotCommandEmul.cmd);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(long(dt_ * 1000)));
            t_ += dt_;
        }
    });

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // main loop : wait user input
    //
    while (bRun.load())
    {
        std::cout << "(type \'q\' to quit) >\n";
        std::string cmd;
        std::cin >> cmd;
        if (cmd == "q" || cmd == "quit") { bRun = false; }
    }

    cmd_publisher.join();
    dt::Log::Terminate();

    return 0;
}