#include "emulGridmap.h"
#include "emulObjectPerception.h"
#include "emulOdom.h"
#include "emulRobotCommand.h"
#include "pubGridmap.h"
#include "pubPerceivedObject.h"
#include "pubRobotCommand.h"
#include "pubVisualOdom.h"
#include "rpcClient.h"     // main RPC client interface
#include "rpcSubscriber.h" // simple RPC message subscriber
#include <atomic>
#include <chrono>
#include <dtCore/src/dtLog/dtLog.h>
#include <dtCore/src/dtUtils/dtStringHelper.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

OdomEmulator odomEmul;
GridmapEmulator gridmapEmul;
RobotCommandEmulator robotcmdEmul;
ObjectPerceptionEmulator objectPerceptionEmul;
std::string svrIp = "127.0.0.1";

int main(int argc, char *argv[])
{
    dt::Log::Initialize(
        "leoquad_vision_rpc_client"); //, "logs/leoquad_vision_rpc_client.txt");
    dt::Log::SetLogLevel(dt::Log::LogLevel::info);

    // dt::Log::Create("latency", "logs/delay.csv", true, true);
    // dt::Log::SetLogLevel("latency", dt::Log::LogLevel::info);
    // dt::Log::SetLogPattern("latency", dt::Log::LogPatternFlag::none, "");
    // LOG_U(latency, info) <<
    // "request_time_s,response_time_s,delay_req_ms,delay_res_ms,delay_tot_ms";

    if (argc > 1) { svrIp = argv[1]; }
    LOG(info) << "vision_rpc_client, server_ip = " << svrIp;

    std::atomic<bool> bRun{true};

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // kinematic odometry subscriber
    //
    std::function<void(dtproto::quadruped::OdomWithJointPosTimeStamped &)>
        handler_odom_with_jointpos =
            [](dtproto::quadruped::OdomWithJointPosTimeStamped &msg) {
                LOG(debug) << "handler_odom_with_jointpos Got a new message.";

                if (msg.header().seq() == 0)
                {
                    LOG(info) << "Special ODOM message (seq == 0) is received.";
                }

                LOG(trace) << "  base_pose.position (x|y|z) = "
                           << msg.odom().pose().position().x() << "|"
                           << msg.odom().pose().position().y() << "|"
                           << msg.odom().pose().position().z();
                LOG(trace) << "  base_pose.orientation (w|x|y|z) = "
                           << msg.odom().pose().orientation().w() << "|"
                           << msg.odom().pose().orientation().x() << "|"
                           << msg.odom().pose().orientation().y() << "|"
                           << msg.odom().pose().orientation().z();
                for (int ji = 0; ji < 12; ji++)
                {
                    LOG(trace) << "  joint[" << ji
                               << "].position = " << msg.joint_pos(ji);
                }
                for (int fi = 0; fi < 4; fi++)
                {
                    LOG(trace)
                        << "  foot[" << fi
                        << "].position (x|y|z) = " << msg.foot_pos(fi).x()
                        << "|" << msg.foot_pos(fi).y() << "|"
                        << msg.foot_pos(fi).z();
                }
                LOG(trace) << "  contact = " << msg.contact().a1() << "|"
                           << msg.contact().a2() << "|" << msg.contact().a3()
                           << "|" << msg.contact().a4();

                odomEmul.kodom.position.x = msg.odom().pose().position().x();
                odomEmul.kodom.position.y = msg.odom().pose().position().y();
                odomEmul.kodom.position.z = msg.odom().pose().position().z();
                odomEmul.kodom.orientation.w =
                    msg.odom().pose().orientation().w();
                odomEmul.kodom.orientation.x =
                    msg.odom().pose().orientation().x();
                odomEmul.kodom.orientation.y =
                    msg.odom().pose().orientation().y();
                odomEmul.kodom.orientation.z =
                    msg.odom().pose().orientation().z();

                //
                // gridmapEmul.gridmap.offset.x = odomEmul.kodom.position.x
                // - 3.0; gridmapEmul.gridmap.offset.y =
                // odomEmul.kodom.position.y - 3.0; gridmapEmul.gridmap.offset.z
                // = 0.0;
            };
    std::unique_ptr<
        RpcSubscriber<dtproto::quadruped::OdomWithJointPosTimeStamped>>
        sub_odom_with_jointpos = std::make_unique<
            RpcSubscriber<dtproto::quadruped::OdomWithJointPosTimeStamped>>(
            "Odom", dt::Utils::string_format("%s:%d", svrIp.c_str(), 50055));
    sub_odom_with_jointpos->RegMessageHandler(handler_odom_with_jointpos);

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // IMU data subscriber
    //
    std::function<void(dtproto::sensor_msgs::ImuTimeStamped &)> handler_imu =
        [](dtproto::sensor_msgs::ImuTimeStamped &msg) {
            LOG(debug) << "handler_imu Got a new message.";

            LOG(trace) << "  Imu.angular_velocity (x|y|z) = "
                       << msg.imu().angular_velocity().a1() << "|"
                       << msg.imu().angular_velocity().a2() << "|"
                       << msg.imu().angular_velocity().a3();
            LOG(trace) << "  Imu.linear_acceleration (x|y|z) = "
                       << msg.imu().linear_acceleration().a1() << "|"
                       << msg.imu().linear_acceleration().a2() << "|"
                       << msg.imu().linear_acceleration().a3();
            LOG(trace) << "  Imu.base_pose.orientation (w|x|y|z) = "
                       << msg.imu().orientation().w() << "|"
                       << msg.imu().orientation().x() << "|"
                       << msg.imu().orientation().y() << "|"
                       << msg.imu().orientation().z();
        };
    std::unique_ptr<RpcSubscriber<dtproto::sensor_msgs::ImuTimeStamped>>
        sub_imu = std::make_unique<
            RpcSubscriber<dtproto::sensor_msgs::ImuTimeStamped>>(
            "Imu", dt::Utils::string_format("%s:%d", svrIp.c_str(), 50054));
    sub_imu->RegMessageHandler(handler_imu);

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // RPC service client
    //
    std::unique_ptr<RpcClient<dtproto::quadruped::Nav>> rpcClient = std::make_unique<RpcClient<dtproto::quadruped::Nav>>(
        dt::Utils::string_format("%s:%d", svrIp.c_str(), 50056));

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // visual odometry publisher (client-side streamer)
    //
    std::thread odom_publisher = std::thread([&]() {
        double t_ = 0.0;
        double dt_ = 1.0 / ODOM_PUB_RATE;

        uint64_t cid = rpcClient->template StartCall<PubVisualOdometry>(
            (void *)(&odomEmul.odom));
        std::shared_ptr<PubVisualOdometry> call = std::dynamic_pointer_cast<
            PubVisualOdometry,
            dt::DAQ::ServiceCallerGrpc<dtproto::quadruped::Nav>::Call>(
            rpcClient->GetCall(cid));
        if (call == nullptr) return;

        while (bRun.load())
        {
            call->Publish(odomEmul.odom);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(long(dt_ * 1000)));
            t_ += dt_;
        }
    });

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // grid map publisher (client-side streamer)
    //
    std::thread gridmap_publisher = std::thread([&]() {
        double t_ = 0.0;
        double dt_ = 1.0 / GRIDMAP_PUB_RATE;

        uint64_t cid = rpcClient->template StartCall<PubGridmap>(
            (void *)(&gridmapEmul.gridmap));
        std::shared_ptr<PubGridmap> call = std::dynamic_pointer_cast<
            PubGridmap,
            dt::DAQ::ServiceCallerGrpc<dtproto::quadruped::Nav>::Call>(
            rpcClient->GetCall(cid));
        if (call == nullptr) return;

        while (bRun.load())
        {
            call->Publish(gridmapEmul.gridmap);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(long(dt_ * 1000)));
            t_ += dt_;
        }
    });

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // robot command publisher (client-side streamer)
    //
    std::thread robotcmd_publisher = std::thread([&]() {
        double t_ = 0.0;
        double dt_ = 1.0 / ROBOTCMD_PUB_RATE;

        uint64_t cid = rpcClient->template StartCall<PubRobotCommand>(
            (void *)(&robotcmdEmul.cmd));
        std::shared_ptr<PubRobotCommand> call = std::dynamic_pointer_cast<
            PubRobotCommand,
            dt::DAQ::ServiceCallerGrpc<dtproto::quadruped::Nav>::Call>(
            rpcClient->GetCall(cid));
        if (call == nullptr) return;

        while (bRun.load())
        {
            call->Publish(robotcmdEmul.cmd);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(long(dt_ * 1000)));
            t_ += dt_;
        }
    });

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

    sub_odom_with_jointpos.reset();
    sub_imu.reset();
    odom_publisher.join();
    gridmap_publisher.join();
    robotcmd_publisher.join();
    object_publisher.join();
    dt::Log::Terminate(); // flush all log messages
    return 0;
}