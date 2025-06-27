#include "rpcServer.h"
#include "onReqRobotInfo.h"
#include "onReqVersion.h"
#include "onPerceivedObject.h"
#include "robotData.h"
#include <dtCore/src/dtLog/dtLog.h>
#include <dtProto/Service.grpc.pb.h>

/////////////////////////////////////////////////////////////////////////
// RpcServer implementation
//
RpcServer::RpcServer(void *robotData)
    : _dtServiceListener(std::make_unique<dt::DAQ::ServiceListenerGrpc>(
          std::make_unique<dtproto::dtService::AsyncService>(),
          "0.0.0.0:50052")),
      _perceptionServiceListener(std::make_unique<dt::DAQ::ServiceListenerGrpc>(
          std::make_unique<dtproto::perception::AsyncService>(),
          "0.0.0.0:50059")),
      _robotStatePublisher(std::make_unique<dt::DAQ::StatePublisherGrpc<
                               dtproto::robot_msgs::RobotStateTimeStamped>>(
          "RobotState", "0.0.0.0:50053", 0)),
      _opStatePublisher(std::make_unique<dt::DAQ::StatePublisherGrpc<
                        dtproto::dualarm::OperationStateTimeStamped>>(
          "OpState", "0.0.0.0:50060", 0)),
      _robotData((RobotData *)robotData)
{
    _dtServiceListener->AddSession<OnReqRobotInfo>(robotData);
    _dtServiceListener->AddSession<OnReqVersion>(robotData);
    
    _perceptionServiceListener->AddSession<OnPerceivedObjectArray>(robotData);

    // pre-allocate and initialize message structure
    _robotStateMsg.mutable_header()->set_frame_id("base");
    for (int ji = 0; ji < NUM_JOINT; ji++)
    {
        _robotStateMsg.mutable_state()->add_joint_state();
    }
}

RpcServer::~RpcServer()
{
    _dtServiceListener->Stop();
    _perceptionServiceListener->Stop();
}

void RpcServer::Run()
{
    _dataUpdater = std::thread([this] {
        _runUpdater.store(true);
        double t_ = 0.0;
        double dt_ = 0.01;
        double prevStatTimeSec = t_;
        while (_runUpdater.load())
        {
            struct timespec tp;
            int err = clock_gettime(CLOCK_REALTIME, &tp);

            //
            // Publish RobotState
            //
            // set message header
            _robotStateMsg.mutable_header()->set_seq(0);
            _robotStateMsg.mutable_header()->mutable_time_stamp()->set_seconds(
                tp.tv_sec);
            _robotStateMsg.mutable_header()->mutable_time_stamp()->set_nanos(
                tp.tv_nsec);
            // set message body
            // robot position in {V-odom}
            _robotStateMsg.mutable_state()
                ->mutable_base_pose()
                ->mutable_position()
                ->set_x(_robotData->basePos.x);
            _robotStateMsg.mutable_state()
                ->mutable_base_pose()
                ->mutable_position()
                ->set_y(_robotData->basePos.y);
            _robotStateMsg.mutable_state()
                ->mutable_base_pose()
                ->mutable_position()
                ->set_z(_robotData->basePos.z);
            // robot orientation in {V-odom}
            _robotStateMsg.mutable_state()
                ->mutable_base_pose()
                ->mutable_orientation()
                ->set_w(_robotData->baseRot.w);
            _robotStateMsg.mutable_state()
                ->mutable_base_pose()
                ->mutable_orientation()
                ->set_x(_robotData->baseRot.x);
            _robotStateMsg.mutable_state()
                ->mutable_base_pose()
                ->mutable_orientation()
                ->set_y(_robotData->baseRot.y);
            _robotStateMsg.mutable_state()
                ->mutable_base_pose()
                ->mutable_orientation()
                ->set_z(_robotData->baseRot.z);
            // joint position
            for (int ji = 0; ji < NUM_JOINT; ji++)
            {
                _robotStateMsg.mutable_state()
                    ->mutable_joint_state(ji)
                    ->set_position(_robotData->jointPos[ji]);
            }
            // publish
            _robotStatePublisher->Publish(_robotStateMsg);

            //
            // Publish OperationState
            //
            // set message header
            _opStateMsg.mutable_header()->set_seq(0);
            _opStateMsg.mutable_header()->mutable_time_stamp()->set_seconds(
                tp.tv_sec);
            _opStateMsg.mutable_header()->mutable_time_stamp()->set_nanos(
                tp.tv_nsec);
            // set message body
            _opStateMsg.mutable_state()->set_op_mode(1);
            _opStateMsg.mutable_state()->set_op_status(2);
            // publish
            _opStatePublisher->Publish(_opStateMsg);


            // Advance time
            std::this_thread::sleep_for(
                std::chrono::milliseconds((long)(dt_ * 1000)));
            t_ += dt_;
        }
    });
}

void RpcServer::Stop()
{
    _runUpdater.store(false);
    _dataUpdater.join();

    _dtServiceListener->Stop();
    _perceptionServiceListener->Stop();
}
