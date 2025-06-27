#include "onReqVersion.h"
#include "bmr.h"
#include <dtCore/src/dtLog/dtLog.h>

OnReqVersion::OnReqVersion(dt::DAQ::ServiceListenerGrpc *server, grpc::Service *service, grpc::ServerCompletionQueue *cq, void *udata)
    : dt::DAQ::ServiceListenerGrpc::Session(server, service, cq, udata), _responder(&_ctx)
{
    _response.set_value(DTCONTROL_VERSION_STR);
    _call_state = CallState::WAIT_CONNECT;
    (static_cast<ServiceType *>(_service))->RequestRequestVersion(&_ctx, &_request, &_responder, _cq, _cq, this);
    LOG(debug) << "OnReqVersion[" << _id << "] Waiting for new service call.";
}

bool OnReqVersion::OnCompletionEvent(bool ok)
{
    if (_call_state == CallState::FINISHED)
    {
        return true;
    }
    else if (_call_state == CallState::WAIT_FINISH)
    {
        LOG(debug) << "OnReqVersion[" << _id << "] Finished.";
        // _call_state = CallState::FINISHED;
        // _server->RemoveSession(_id);
        return false;
    }
    else if (ok)
    {
        if (_call_state == CallState::WAIT_CONNECT)
        {
            LOG(debug) << "OnReqVersion[" << _id << "] NEW service call.";

            _server->template AddSession<OnReqVersion>((void *)nullptr);
            {
                std::lock_guard<std::mutex> lock(_proc_mtx);

                _call_state = CallState::WAIT_FINISH;
                _responder.Finish(_response, grpc::Status::OK, this);
            }
        }
        else
        {
            LOG(err) << "OnReqVersion[" << _id << "] Invalid session status (" << static_cast<int>(_call_state) << ")";
            ABSL_ASSERT(false && "Invalid Session Status.");
            return false;
        }
    }
    else
    {
        if (_call_state == CallState::WAIT_CONNECT)
        {
            LOG(err) << "OnReqVersion[" << _id << "] Session has been shut down before receiving a matching request.";
            return false;
        }
        else
        {
            std::lock_guard<std::mutex> lock(_proc_mtx);
            LOG(trace) << "OnReqVersion[" << _id << "] Finish()";
            _call_state == CallState::WAIT_FINISH;
            _responder.Finish(_response, grpc::Status::CANCELLED, this);
        }
    }
    return true;
}