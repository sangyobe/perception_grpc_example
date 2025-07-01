#ifndef SYSTEM_NETWORK_ONSUBROBOTCOMMAND_H
#define SYSTEM_NETWORK_ONSUBROBOTCOMMAND_H

#include "robotData.h"
#include <dtCore/src/dtDAQ/grpc/dtServiceListenerGrpc.hpp>
#include <dtProto/Service.grpc.pb.h>
#include <dtProto/robot_msgs/RobotCommand.pb.h>
#include <memory>
#include <string>

class OnSubRobotCommand : public dt::DAQ::ServiceListenerGrpc::Session
{
    using CallState = typename dt::DAQ::ServiceListenerGrpc::Session::CallState;
    using ServiceType = dtproto::dtService::AsyncService;

public:
    OnSubRobotCommand(dt::DAQ::ServiceListenerGrpc *server, grpc::Service *service, grpc::ServerCompletionQueue *cq, void *udata = nullptr);
    ~OnSubRobotCommand() = default;
    bool OnCompletionEvent(bool ok) override;

private:
    ::dtproto::robot_msgs::RobotCommandTimeStamped _request;
    ::dtproto::std_msgs::Response _response;
    ::grpc::ServerAsyncReader<::dtproto::std_msgs::Response, ::dtproto::robot_msgs::RobotCommandTimeStamped> _responder;
    RobotData *_robotData;
};

#endif // SYSTEM_NETWORK_ONSUBROBOTCOMMAND_H