#ifndef ONROBOTCOMMAND_H
#define ONROBOTCOMMAND_H

#include <dtCore/src/dtDAQ/grpc/dtServiceListenerGrpc.hpp>
#include <dtProto/Service.grpc.pb.h>
#include <dtProto/robot_msgs/RobotInfo.pb.h>

class OnRobotCommand : public dt::DAQ::ServiceListenerGrpc::Session
{
    using CallState = typename dt::DAQ::ServiceListenerGrpc::Session::CallState;
    using ServiceType = dtproto::dtService::AsyncService;

public:
    OnRobotCommand(dt::DAQ::ServiceListenerGrpc *server, grpc::Service *service, grpc::ServerCompletionQueue *cq, void *udata = nullptr);
    ~OnRobotCommand() = default;
    bool OnCompletionEvent(bool ok) override;

private:
    dtproto::robot_msgs::RobotCommandTimeStamped _request;
    dtproto::std_msgs::Response _response;
    ::grpc::ServerAsyncResponseWriter<dtproto::std_msgs::Response> _responder;
};

#endif // ONROBOTCOMMAND_H