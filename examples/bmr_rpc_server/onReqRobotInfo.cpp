#include "onReqRobotInfo.h"
#include "bmr.h"
#include <dtCore/src/dtLog/dtLog.h>

OnReqRobotInfo::OnReqRobotInfo(dt::DAQ::ServiceListenerGrpc *server, grpc::Service *service, grpc::ServerCompletionQueue *cq, void *udata)
    : dt::DAQ::ServiceListenerGrpc::Session(server, service, cq, udata), _responder(&_ctx)
{
    _response.set_name("DualArm");
    _response.set_author("HMC-ART");
    _response.set_version(DTCONTROL_VERSION_STR);
    _response.set_description("");
    _response.set_serial("");
    _response.set_type(0);
    _response.set_id(0);
    _response.set_dof(NUM_JOINT);

    _call_state = CallState::WAIT_CONNECT;
    (static_cast<ServiceType *>(_service))->RequestRequestRobotInfo(&_ctx, &_request, &_responder, _cq, _cq, this);
    LOG(debug) << "OnReqRobotInfo[" << _id << "] Waiting for new service call.";
}

bool OnReqRobotInfo::OnCompletionEvent(bool ok)
{
    if (_call_state == CallState::FINISHED)
    {
        return true;
    }
    else if (_call_state == CallState::WAIT_FINISH)
    {
        LOG(debug) << "OnReqRobotInfo[" << _id << "] Finished.";
        // _call_state = CallState::FINISHED;
        // _server->RemoveSession(_id);
        return false;
    }
    else if (ok)
    {
        if (_call_state == CallState::WAIT_CONNECT)
        {
            LOG(debug) << "OnReqRobotInfo[" << _id << "] NEW service call.";

            _server->template AddSession<OnReqRobotInfo>((void *)nullptr);
            {
                std::lock_guard<std::mutex> lock(_proc_mtx);

                _call_state = CallState::WAIT_FINISH;
                _responder.Finish(_response, grpc::Status::OK, this);
            }
        }
        else
        {
            LOG(err) << "OnReqRobotInfo[" << _id << "] Invalid session status (" << static_cast<int>(_call_state) << ")";
            ABSL_ASSERT(false && "Invalid Session Status.");
            return false;
        }
    }
    else
    {
        if (_call_state == CallState::WAIT_CONNECT)
        {
            LOG(err) << "OnReqRobotInfo[" << _id << "] Session has been shut down before receiving a matching request.";
            return false;
        }
        else
        {
            std::lock_guard<std::mutex> lock(_proc_mtx);
            LOG(trace) << "OnReqRobotInfo[" << _id << "] Finish()";
            _call_state == CallState::WAIT_FINISH;
            _responder.Finish(_response, grpc::Status::CANCELLED, this);
        }
    }
    return true;
}