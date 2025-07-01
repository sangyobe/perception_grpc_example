#include "onRobotCommand.h"
#include "bmr.h"
#include <dtCore/src/dtLog/dtLog.h>

OnRobotCommand::OnRobotCommand(dt::DAQ::ServiceListenerGrpc *server, grpc::Service *service, grpc::ServerCompletionQueue *cq, void *udata)
    : dt::DAQ::ServiceListenerGrpc::Session(server, service, cq, udata), _responder(&_ctx)
{
    _call_state = CallState::WAIT_CONNECT;
    (static_cast<ServiceType *>(_service))->RequestRobotCommand(&_ctx, &_request, &_responder, _cq, _cq, this);
    LOG(debug) << "OnRobotCommand[" << _id << "] Waiting for new service call.";
}

bool OnRobotCommand::OnCompletionEvent(bool ok)
{
    if (_call_state == CallState::FINISHED)
    {
        return true;
    }
    else if (_call_state == CallState::WAIT_FINISH)
    {
        LOG(debug) << "OnRobotCommand[" << _id << "] Finished.";
        // _call_state = CallState::FINISHED;
        // _server->RemoveSession(_id);
        return false;
    }
    else if (ok)
    {
        if (_call_state == CallState::WAIT_CONNECT)
        {
            LOG(debug) << "OnRobotCommand[" << _id << "] NEW service call.";

            _server->template AddSession<OnRobotCommand>((void *)nullptr);
            {
                std::lock_guard<std::mutex> lock(_proc_mtx);

                if (_request.command().cmd().cmd_mode() == 3)
                {
                    _response.set_rtn(-1);
                    _response.set_msg("fail");
                }
                else
                {
                    _response.set_rtn(0);
                    _response.set_msg("ok");
                }

                _call_state = CallState::WAIT_FINISH;
                _responder.Finish(_response, grpc::Status::OK, this);
            }
        }
        else
        {
            LOG(err) << "OnRobotCommand[" << _id << "] Invalid session status (" << static_cast<int>(_call_state) << ")";
            ABSL_ASSERT(false && "Invalid Session Status.");
            return false;
        }
    }
    else
    {
        if (_call_state == CallState::WAIT_CONNECT)
        {
            LOG(err) << "OnRobotCommand[" << _id << "] Session has been shut down before receiving a matching request.";
            return false;
        }
        else
        {
            std::lock_guard<std::mutex> lock(_proc_mtx);
            LOG(trace) << "OnRobotCommand[" << _id << "] Finish()";
            _call_state == CallState::WAIT_FINISH;
            _responder.Finish(_response, grpc::Status::CANCELLED, this);
        }
    }
    return true;
}