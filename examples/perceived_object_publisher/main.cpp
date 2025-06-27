#include "emulObjectPerception.h"
#include "pubPerceivedObject.h"
#include "rpcClient.h"     // main RPC client interface
#include <dtCore/src/dtDAQ/grpc/dtStateSubscriberGrpc.hpp>
#include <dtCore/src/dtLog/dtLog.h>
#include <dtCore/src/dtUtils/dtStringHelper.hpp>

ObjectPerceptionEmulator objectPerceptionEmul;
std::string svrIp = "127.0.0.1";

int main(int argc, char **argv)
{
    dt::Log::Initialize("perceived_object_publisher");
    dt::Log::SetLogLevel(dt::Log::LogLevel::trace);

    if (argc > 1) { svrIp = argv[1]; }
    LOG(info) << "perceived_object_publisher, server_ip = " << svrIp;

    std::atomic<bool> bRun;
    bRun.store(true);

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // perception RPC service client
    //
    std::unique_ptr<RpcClient<dtproto::perception>> rpcClientPerception = std::make_unique<RpcClient<dtproto::perception>>(
        dt::Utils::string_format("%s:%d", svrIp.c_str(), 50059));

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // perceived objects publisher (client-side streamer)
    //
    std::thread object_publisher = std::thread([&]() {
        double t_ = 0.0;
        double dt_ = 1.0 / OBJECT_PUB_RATE;

        uint64_t cid = rpcClientPerception->template StartCall<PubPerceivedObjectArray>(
            (void *)(&objectPerceptionEmul.objmap));
        std::shared_ptr<PubPerceivedObjectArray> call = std::dynamic_pointer_cast<
            PubPerceivedObjectArray,
            dt::DAQ::ServiceCallerGrpc<dtproto::perception>::Call>(
            rpcClientPerception->GetCall(cid));
        if (call == nullptr) return;

        while (bRun.load())
        {
            call->Publish(objectPerceptionEmul.objmap);
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

    object_publisher.join();
    dt::Log::Terminate();

    return 0;
}