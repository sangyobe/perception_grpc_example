#ifndef __RPCSUBSCRIBER_H__
#define __RPCSUBSCRIBER_H__

#include <dtCore/src/dtDAQ/grpc/dtStateSubscriberGrpc.hpp>

template <typename MessageType>
class RpcSubscriber : public dt::DAQ::StateSubscriberGrpc<MessageType>
{
public:
    RpcSubscriber(const std::string &topic_name, const std::string &server_address)
        : dt::DAQ::StateSubscriberGrpc<MessageType>(topic_name, server_address) {}
};

#endif // __RPCSUBSCRIBER_H__