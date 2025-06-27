#ifndef __RPCSERVER_H__
#define __RPCSERVER_H__

#include <atomic>
#include <dtCore/src/dtDAQ/grpc/dtServiceListenerGrpc.hpp>
#include <dtCore/src/dtDAQ/grpc/dtStatePublisherGrpc.hpp>
#include <dtProto/Service.grpc.pb.h>
#include <dtProto/robot_msgs/RobotState.pb.h>
#include <memory>
#include <string>
#include <thread>
#include "DualArm.pb.h"

class RobotData;
class RpcServer
{
public:
    RpcServer(void *robotData);
    virtual ~RpcServer();

public:
    void Run();
    void Stop();

private:
    std::unique_ptr<dt::DAQ::ServiceListenerGrpc> _dtServiceListener{nullptr};
    std::unique_ptr<dt::DAQ::ServiceListenerGrpc> _perceptionServiceListener{nullptr};
    std::unique_ptr<dt::DAQ::StatePublisherGrpc<dtproto::robot_msgs::RobotStateTimeStamped>> _robotStatePublisher{nullptr};
    std::unique_ptr<dt::DAQ::StatePublisherGrpc<dtproto::dualarm::OperationStateTimeStamped>> _opStatePublisher{nullptr};

private:
    std::thread _dataUpdater;
    std::atomic_bool _runUpdater;
    RobotData *_robotData;
    dtproto::robot_msgs::RobotStateTimeStamped _robotStateMsg;
    dtproto::dualarm::OperationStateTimeStamped _opStateMsg;
};

#endif // __RPCSERVER_H__