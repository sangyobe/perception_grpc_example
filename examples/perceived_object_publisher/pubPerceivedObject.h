#ifndef __PUBPERCEIVEDOBJECT_H__
#define __PUBPERCEIVEDOBJECT_H__

#include "emulObjectPerception.h"
#include <dtCore/src/dtDAQ/grpc/dtServiceCallerGrpc.hpp>
#include <memory>
#include <string>

class PubPerceivedObjectArray : public dt::DAQ::ServiceCallerGrpc<dtproto::perception>::Call
{
    using CallState = typename dt::DAQ::ServiceCallerGrpc<dtproto::perception>::Call::CallState;

public:
    PubPerceivedObjectArray(dtproto::perception::Stub *stub, grpc::CompletionQueue *cq, void *udata = nullptr);
    ~PubPerceivedObjectArray();

    bool OnCompletionEvent(bool ok) override;

    bool Publish(const ObjectMap &objmap);

private:
    ::dtproto::perception_msgs::ObjectArrayTimeStamped _request;
    ::dtproto::std_msgs::Response _response;
    std::unique_ptr<::grpc::ClientAsyncWriter<::dtproto::perception_msgs::ObjectArrayTimeStamped>> _writer;
    ObjectMap *_objmapData{nullptr};
    uint32_t _msgSeq{0};
};

#endif // __PUBPERCEIVEDOBJECT_H__