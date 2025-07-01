#ifndef __PUBROBOTCOMMAND_H__
#define __PUBROBOTCOMMAND_H__

#include "emulRobotCommand.h"
#include <dtCore/src/dtDAQ/grpc/dtServiceCallerGrpc.hpp>
#include <dtProto/Service.grpc.pb.h>
#include <dtProto/robot_msgs/RobotCommand.pb.h>
#include <memory>
#include <string>

class PubRobotCommand : public dt::DAQ::ServiceCallerGrpc<dtproto::dtService>::Call
{
    using CallState = typename dt::DAQ::ServiceCallerGrpc<dtproto::dtService>::Call::CallState;

public:
    PubRobotCommand(dtproto::dtService::Stub *stub, grpc::CompletionQueue *cq, void *udata = nullptr);
    ~PubRobotCommand();

    bool OnCompletionEvent(bool ok) override;

    bool Publish(const RobotCommand &cmd);

private:
    ::dtproto::robot_msgs::RobotCommandTimeStamped _request;
    ::dtproto::std_msgs::Response _response;
    std::unique_ptr<::grpc::ClientAsyncWriter<::dtproto::robot_msgs::RobotCommandTimeStamped>> _writer;
    RobotCommand *_robotCommand{nullptr};
    uint32_t _msgSeq{0};
};

#endif // __PUBROBOTCOMMAND_H__