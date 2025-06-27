#ifndef ONREQROBOTINFO_H
#define ONREQROBOTINFO_H

#include <dtCore/src/dtDAQ/grpc/dtServiceListenerGrpc.hpp>
#include <dtProto/Service.grpc.pb.h>
#include <dtProto/robot_msgs/RobotInfo.pb.h>

class OnReqRobotInfo : public dt::DAQ::ServiceListenerGrpc::Session
{
    using CallState = typename dt::DAQ::ServiceListenerGrpc::Session::CallState;
    using ServiceType = dtproto::dtService::AsyncService;

public:
    OnReqRobotInfo(dt::DAQ::ServiceListenerGrpc *server, grpc::Service *service, grpc::ServerCompletionQueue *cq, void *udata = nullptr);
    ~OnReqRobotInfo() = default;
    bool OnCompletionEvent(bool ok) override;

private:
    google::protobuf::Empty _request;
    dtproto::robot_msgs::RobotInfo _response;
    ::grpc::ServerAsyncResponseWriter<dtproto::robot_msgs::RobotInfo> _responder;
};

#endif // ONREQROBOTINFO_H