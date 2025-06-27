#include "pubPerceivedObject.h"
#include <dtCore/src/dtLog/dtLog.h>

using ServiceType = dtproto::perception;

PubPerceivedObjectArray::PubPerceivedObjectArray(ServiceType::Stub *stub, grpc::CompletionQueue *cq, void *udata)
    : dt::DAQ::ServiceCallerGrpc<ServiceType>::Call(stub, cq, udata), _objmapData((ObjectMap *)udata)
{
    _request.mutable_object_array()->add_objects();
    _request.mutable_object_array()->add_objects();
    _request.mutable_object_array()->add_objects();

    LOG(debug) << "PubPerceivedObjectArray[" << _id << "] NEW call.";
    _writer = _stub->PrepareAsyncSubscribePerceivedObjects(&(this->_ctx), &_response, this->_cq);
    _writer->StartCall((void *)this);
    this->_call_state = CallState::WAIT_CONNECT;
}

PubPerceivedObjectArray::~PubPerceivedObjectArray()
{
    // LOG(debug) << "PubPerceivedObjectArray[" << _id << "] Delete call."; // Do not output log
    // here. It might be after LOG system has been destroyed.
}

bool PubPerceivedObjectArray::Publish(const ObjectMap &objmap)
{
    if (this->_call_state != CallState::READY_TO_WRITE)
        return false;

    LOG(debug) << "PubPerceivedObjectArray[" << _id << "] publish a message.";
    std::lock_guard<std::mutex> lock(this->_proc_mtx);
    {
        struct timespec tp;
        int err = clock_gettime(CLOCK_REALTIME, &tp);

        // set message header
        _request.mutable_header()->set_seq(_msgSeq++);
        _request.mutable_header()->mutable_time_stamp()->set_seconds(tp.tv_sec);
        _request.mutable_header()->mutable_time_stamp()->set_nanos(tp.tv_nsec);
        // set message body
        for (auto obj : objmap.objects)
        {
            if (obj.valid)
            {
                _request.mutable_object_array()->mutable_objects(0)->set_id(obj.id);
                _request.mutable_object_array()->mutable_objects(1)->set_id(obj.id + 100);
                _request.mutable_object_array()->mutable_objects(2)->set_id(obj.id + 200);
                break;
            }
        }

        _call_state = CallState::WAIT_WRITE_DONE;
        _writer->Write(_request, (void *)this);
    }

    return true;
}

bool PubPerceivedObjectArray::OnCompletionEvent(bool ok)
{
    if (this->_call_state == CallState::WAIT_FINISH)
    {
        switch (this->_status.error_code())
        {
        case grpc::OK:
        {
            LOG(debug) << "PubPerceivedObjectArray[" << _id << "] Complete !!!";
        }
        break;

        case grpc::CANCELLED:
        {
            LOG(warn) << "PubPerceivedObjectArray[" << _id << "] Cancelled !!!";
        }
        break;

        default:
        {
            LOG(err) << "PubPerceivedObjectArray[" << _id << "] Failed !!!";
        }
        break;
        }
        return false;
    }
    else if (ok)
    {

        if (this->_call_state == CallState::WAIT_CONNECT)
        {
            LOG(debug) << "PubPerceivedObjectArray[" << _id << "] Start call.";
            std::lock_guard<std::mutex> lock(this->_proc_mtx);
            _call_state = CallState::READY_TO_WRITE;
        }
        else if (this->_call_state == CallState::WAIT_WRITE_DONE)
        {
            std::lock_guard<std::mutex> lock(_proc_mtx);
            _call_state = CallState::READY_TO_WRITE;
        }
        else if (this->_call_state == CallState::WAIT_RESPONSE)
        {
            LOG(debug) << "PubPerceivedObjectArray[" << _id << "] Get response...";
            std::lock_guard<std::mutex> lock(this->_proc_mtx);
            {
                _call_state = CallState::WAIT_FINISH;
                _writer->Finish(&(this->_status), (void *)this);
            }
        }
        else
        {
            LOG(err) << "PubPerceivedObjectArray[" << _id << "] Invalid call state (" << static_cast<int>(_call_state) << ")";
            GPR_ASSERT(false && "Invalid Call State.");
            return false;
        }
    }
    else
    {
        std::lock_guard<std::mutex> lock(this->_proc_mtx);
        {
            _call_state = CallState::WAIT_FINISH;
            _writer->Finish(&(this->_status), (void *)this);
        }
    }

    return true;
}