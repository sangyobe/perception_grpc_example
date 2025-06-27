#ifndef ONREQVERSION_H
#define ONREQVERSION_H

#include <dtCore/src/dtDAQ/grpc/dtServiceListenerGrpc.hpp>
#include <dtProto/Service.grpc.pb.h>

class OnReqVersion : public dt::DAQ::ServiceListenerGrpc::Session
{
    using CallState = typename dt::DAQ::ServiceListenerGrpc::Session::CallState;
    using ServiceType = dtproto::dtService::AsyncService;

public:
    OnReqVersion(dt::DAQ::ServiceListenerGrpc *server, grpc::Service *service, grpc::ServerCompletionQueue *cq, void *udata = nullptr);
    ~OnReqVersion() = default;
    bool OnCompletionEvent(bool ok) override;

private:
    google::protobuf::Empty _request;
    google::protobuf::StringValue _response;
    ::grpc::ServerAsyncResponseWriter<google::protobuf::StringValue> _responder;
};

#endif // ONREQVERSION_H