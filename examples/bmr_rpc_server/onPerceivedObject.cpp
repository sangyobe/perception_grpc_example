#include "onPerceivedObject.h"
#include <dtCore/src/dtLog/dtLog.h>

OnPerceivedObjectArray::OnPerceivedObjectArray(dt::DAQ::ServiceListenerGrpc *server, grpc::Service *service, grpc::ServerCompletionQueue *cq, void *udata)
    : dt::DAQ::ServiceListenerGrpc::Session(server, service, cq, udata), _responder(&_ctx), _robotData((RobotData *)udata)
{
    _call_state = CallState::WAIT_CONNECT;
    (static_cast<ServiceType *>(_service))->RequestSubscribePerceivedObjects(&(_ctx), &_responder, _cq, _cq, this);
    LOG(debug) << "OnPerceivedObjectArray[" << _id << "] Waiting for new service call.";
}

bool OnPerceivedObjectArray::OnCompletionEvent(bool ok)
{
    if (_call_state == CallState::WAIT_FINISH)
    {
        return false;
    }
    else if (ok)
    {

        if (_call_state == CallState::WAIT_CONNECT)
        {
            LOG(debug) << "OnPerceivedObjectArray[" << _id << "] NEW service call.";
            // add another service listener
            _server->template AddSession<OnPerceivedObjectArray>((void *)_robotData);
            // process incomming service call
            {
                std::lock_guard<std::mutex> lock(_proc_mtx);
                _responder.Read(&_request, (void *)this);
                _call_state = CallState::WAIT_READ_DONE;
            }
        }
        else if (_call_state == CallState::WAIT_READ_DONE)
        {
            std::lock_guard<std::mutex> lock(_proc_mtx);

            for (const dtproto::perception_msgs::Object &obj : _request.object_array().objects())
            {
                LOG(info) << "Detected: " << obj.id();
            }

            _responder.Read(&_request, (void *)this);
            _call_state = CallState::WAIT_READ_DONE;
        }
    }
    else
    {
        if (_call_state == CallState::WAIT_CONNECT)
        {
            LOG(err) << "OnPerceivedObjectArray[" << _id << "] Session has been shut down before receiving a matching request.";
            return false;
        }
        else
        {
            std::lock_guard<std::mutex> lock(_proc_mtx);
            _responder.Finish(_response, grpc::Status::CANCELLED, this);
            _call_state = CallState::WAIT_FINISH;
        }
    }
    return true;
}