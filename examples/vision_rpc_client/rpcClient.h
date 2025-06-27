#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include "QuadrupedNav.grpc.pb.h"
#include <dtCore/src/dtDAQ/grpc/dtServiceCallerGrpc.hpp>
#include <string>

template <typename ServiceType>
class RpcClient : public dt::DAQ::ServiceCallerGrpc<ServiceType>
{
public:
    RpcClient(const std::string &server_address)
        : dt::DAQ::ServiceCallerGrpc<ServiceType>(server_address) {}
};

#endif // RPCCLIENT_H