#include <dtCore/src/dtDAQ/grpc/dtStateSubscriberGrpc.hpp>
#include <dtCore/src/dtLog/dtLog.h>
#include <dtProto/sensor_msgs/Imu.pb.h>

int main(int argc, char **argv)
{
    dt::Log::Initialize("grpc_imu_sub");
    dt::Log::SetLogLevel(dt::Log::LogLevel::trace);

    std::function<void(dtproto::sensor_msgs::ImuTimeStamped &)> on_imu = [](dtproto::sensor_msgs::ImuTimeStamped &msg) {
        LOG(trace) << "-------------------------------------------------------------------------------------";
        LOG(trace) << "Time = " << msg.header().time_stamp().seconds() << "." << (int)(msg.header().time_stamp().nanos() * 1e-3) << " [s]";
        LOG(trace) << "Orientation(w,x,y,z) = (" << msg.imu().orientation().w() << ", " << msg.imu().orientation().x() << ", " << msg.imu().orientation().y() << ", " << msg.imu().orientation().z() << ")";
        LOG(trace) << "Angular Velocity     = (" << msg.imu().angular_velocity().a1() << ", " << msg.imu().angular_velocity().a2() << ", " << msg.imu().angular_velocity().a3() << ") [rad/s]";
        LOG(trace) << "Linear Acceleration  = (" << msg.imu().linear_acceleration().a1() << ", " << msg.imu().linear_acceleration().a2() << ", " << msg.imu().linear_acceleration().a3() << ") [m/s^2]";
        LOG(trace) << "";
    };
    std::unique_ptr<dt::DAQ::StateSubscriberGrpc<dtproto::sensor_msgs::ImuTimeStamped>> sub_imu = std::make_unique<dt::DAQ::StateSubscriberGrpc<dtproto::sensor_msgs::ImuTimeStamped>>("Imu", "127.0.0.1:50054");
    sub_imu->RegMessageHandler(on_imu);

    //
    // reconnect thread
    //
    std::atomic<bool> bRun;
    bRun.store(true);

    std::thread proc_reconnector = std::thread([&bRun, &sub_imu]() {
        while (bRun.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));

            if (!sub_imu->IsRunning())
            {
                std::cout << "Reconnect to IMU publisher..." << std::endl;
                sub_imu->Reconnect();
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

    proc_reconnector.join();
    dt::Log::Terminate();

    return 0;
}