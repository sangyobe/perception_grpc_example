#ifndef SYSTEM_NETWORK_ONSUBPERCEIVEDOBJECT_H
#define SYSTEM_NETWORK_ONSUBPERCEIVEDOBJECT_H

#include "robotData.h"
#include <dtCore/src/dtDAQ/grpc/dtServiceListenerGrpc.hpp>
#include <dtProto/Service.grpc.pb.h>
#include <dtProto/perception_msgs/Object.pb.h>
#include <memory>
#include <string>

class OnSubPerceivedObjectArray : public dt::DAQ::ServiceListenerGrpc::Session
{
    using CallState = typename dt::DAQ::ServiceListenerGrpc::Session::CallState;
    using ServiceType = dtproto::perception::AsyncService;

public:
    OnSubPerceivedObjectArray(dt::DAQ::ServiceListenerGrpc *server, grpc::Service *service, grpc::ServerCompletionQueue *cq, void *udata = nullptr);
    ~OnSubPerceivedObjectArray() = default;
    bool OnCompletionEvent(bool ok) override;

private:
    ::dtproto::perception_msgs::ObjectArrayTimeStamped _request;
    ::dtproto::std_msgs::Response _response;
    ::grpc::ServerAsyncReader<::dtproto::std_msgs::Response, ::dtproto::perception_msgs::ObjectArrayTimeStamped> _responder;
    RobotData *_robotData;
};

#endif // SYSTEM_NETWORK_ONSUBPERCEIVEDOBJECT_H