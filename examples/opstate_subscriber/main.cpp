#include "DualArm.pb.h"
#include <dtCore/src/dtDAQ/grpc/dtStateSubscriberGrpc.hpp>
#include <dtCore/src/dtLog/dtLog.h>

int main(int argc, char **argv)
{
    dt::Log::Initialize("grpc_imu_sub");
    dt::Log::SetLogLevel(dt::Log::LogLevel::trace);

    std::function<void(dtproto::dualarm::OperationStateTimeStamped &)> on_op_state = [](dtproto::dualarm::OperationStateTimeStamped &msg) {
        LOG(trace) << "-------------------------------------------------------------------------------------";
        LOG(trace) << "Time = " << msg.header().time_stamp().seconds() << "." << (int)(msg.header().time_stamp().nanos() * 1e-3) << " [s]";
        LOG(trace) << "State = (" << msg.state().op_mode() << ", " << msg.state().op_status() << ")";
        LOG(trace) << "";
    };
    std::unique_ptr<dt::DAQ::StateSubscriberGrpc<dtproto::dualarm::OperationStateTimeStamped>> sub_op_state = std::make_unique<dt::DAQ::StateSubscriberGrpc<dtproto::dualarm::OperationStateTimeStamped>>("Imu", "127.0.0.1:50060");
    sub_op_state->RegMessageHandler(on_op_state);

    //
    // reconnect thread
    //
    std::atomic<bool> bRun;
    bRun.store(true);

    std::thread proc_reconnector = std::thread([&bRun, &sub_op_state]() {
        while (bRun.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));

            if (!sub_op_state->IsRunning())
            {
                std::cout << "Reconnect to OP_State publisher..." << std::endl;
                sub_op_state->Reconnect();
            }
        }
    });

    //
    // main-loop
    //
    while (bRun.load())
    {
        std::cout << "(type \'q\' to quit) >\n";
        std::string cmd;
        std::cin >> cmd;
        if (cmd == "q" || cmd == "quit")
        {
            bRun = false;
        }
    }

    LOG(debug) << "Terminating...";
    proc_reconnector.join();
    dt::Log::Terminate();

    return 0;
}