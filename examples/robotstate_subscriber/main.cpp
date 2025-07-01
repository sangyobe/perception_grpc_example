#include <dtCore/src/dtDAQ/grpc/dtStateSubscriberGrpc.hpp>
#include <dtCore/src/dtLog/dtLog.h>
#include <dtProto/robot_msgs/RobotState.pb.h>

int main(int argc, char **argv)
{
    dt::Log::Initialize("grpc_robotstate_sub");
    dt::Log::SetLogLevel(dt::Log::LogLevel::trace);

    std::function<void(dtproto::robot_msgs::RobotStateTimeStamped &)> on_robot_state = [](dtproto::robot_msgs::RobotStateTimeStamped &msg) {
        LOG(trace) << "-------------------------------------------------------------------------------------";
        LOG(trace) << "Time = " << msg.header().time_stamp().seconds() << "." << (int)(msg.header().time_stamp().nanos() * 1e-3) << " [s]";
        LOG(trace) << "Position = (" << msg.state().base_pose().position().x() << ", " << msg.state().base_pose().position().y() << ", " << msg.state().base_pose().position().z() << ")";
        LOG(trace) << "Joint Position = (" << msg.state().joint_state(0).position() << ", " << msg.state().joint_state(1).position() << ", " << msg.state().joint_state(2).position() << ", ...)";
        LOG(trace) << "";
    };
    std::unique_ptr<dt::DAQ::StateSubscriberGrpc<dtproto::robot_msgs::RobotStateTimeStamped>> sub_robot_state = std::make_unique<dt::DAQ::StateSubscriberGrpc<dtproto::robot_msgs::RobotStateTimeStamped>>("Imu", "127.0.0.1:50053");
    sub_robot_state->RegMessageHandler(on_robot_state);

    //
    // reconnect thread
    //
    std::atomic<bool> bRun;
    bRun.store(true);

    std::thread proc_reconnector = std::thread([&bRun, &sub_robot_state]() {
        while (bRun.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));

            if (!sub_robot_state->IsRunning())
            {
                std::cout << "Reconnect to Robot_State publisher..." << std::endl;
                sub_robot_state->Reconnect();
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