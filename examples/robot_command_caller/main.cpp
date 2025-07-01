#include <dtCore/src/dtDAQ/grpc/dtServiceCallerGrpc.hpp>
#include <dtCore/src/dtLog/dtLog.h>

using ServiceType = dtproto::dtService;

/////////////////////////////////////////////////////////////////////////
// Rpc service call handler
//
class RobotCommandCall : public dt::DAQ::ServiceCallerGrpc<ServiceType>::Call
{
    using CallState =
        typename dt::DAQ::ServiceCallerGrpc<ServiceType>::Call::CallState;

public:
    RobotCommandCall(ServiceType::Stub *stub, grpc::CompletionQueue *cq,
                     void *udata = nullptr)
        : dt::DAQ::ServiceCallerGrpc<ServiceType>::Call(stub, cq, udata)
    {
        LOG(info) << "RobotCommand[" << this->_id << "] NEW call.";
        _call_id = *(int *)(udata);

        // fill robot command...
        _request.mutable_command()->mutable_cmd()->set_cmd_mode(_id);
        _request.mutable_command()->mutable_cmd()->set_arg("hello|world");
        _request.mutable_command()->mutable_cmd()->add_arg_n(1);
        _request.mutable_command()->mutable_cmd()->add_arg_n(_call_id);
        _request.mutable_command()->mutable_cmd()->add_arg_f(3.14);

        // make RPC call...
        _responder =
            stub->PrepareAsyncRobotCommand(&(this->_ctx), _request, this->_cq);
        _responder->StartCall();
        _responder->Finish(&_response, &(this->_status), (void *)this);
        this->_call_state = CallState::WAIT_FINISH;
        LOG(info) << "RobotCommand[" << this->_id << "] Wait for response...";
    }

    ~RobotCommandCall()
    {
        // LOG(info) << "RobotCommandCall session deleted."; // Do not output log
        // here. It might be after LOG system has been destroyed.
    }
    bool OnCompletionEvent(bool ok) override
    {
        if (ok)
        {
            if (this->_call_state == CallState::WAIT_FINISH)
            {
                LOG(info) << "RobotCommand[" << this->_id << "] Get response.";
                {
                    std::lock_guard<std::mutex> lock(this->_proc_mtx);

                    LOG(trace) << "RobotCommand[" << this->_id << "].response.rtn : " << _response.rtn();
                    LOG(trace) << "RobotCommand[" << this->_id << "].response.msg : " << _response.msg();

                    _call_state = CallState::FINISHED;
                }
                return false; // remove this call
            }
            else
            {
                LOG(err) << "RobotCommand[" << this->_id << "] Invalid call state (" << static_cast<int>(_call_state) << ")";
                ABSL_ASSERT(false && "Invalid Call State.");
                return false;
            }
        }
        return false; // remove this call
    }

private:
    ::dtproto::robot_msgs::RobotCommandTimeStamped _request;
    ::dtproto::std_msgs::Response _response;

    std::unique_ptr<::grpc::ClientAsyncResponseReader<::dtproto::std_msgs::Response>> _responder;
    int _call_id{-1};
};

class RpcClient : public dt::DAQ::ServiceCallerGrpc<ServiceType>
{
public:
    RpcClient(const std::string &server_address)
        : dt::DAQ::ServiceCallerGrpc<ServiceType>(server_address) {}
};

/////////////////////////////////////////////////////////////////////////
// main
//
int main(int argc, char **argv)
{
    dt::Log::Initialize("robot_command_caller"); //, "logs/robot_command_caller.txt");
    dt::Log::SetLogLevel(dt::Log::LogLevel::trace);

    // initialize RPC client
    std::unique_ptr<RpcClient> rpcClient =
        std::make_unique<RpcClient>("localhost:50052");

    for (int cid = 0; cid < 5; cid++)
    {
        rpcClient->template StartCall<RobotCommandCall>(&cid);
    }

    std::atomic<bool> bRun{true};
    while (bRun.load())
    {
        std::cout << "(type \'q\' to quit) >\n";
        std::string cmd;
        std::cin >> cmd;
        if (cmd == "q" || cmd == "quit")
        {
            bRun = false;
        }
    }
    rpcClient->Stop();

    dt::Log::Terminate(); // flush all log messages
    return 0;
}
