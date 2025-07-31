#ifndef BMR_H
#define BMR_H

#include "robotData.h"
#include <thread>
#include <chrono>

class BMR
{
public:
    RobotData robotData;

public:
    BMR() 
    {
        _dataUpdater = std::thread([this] {
            _runUpdater.store(true);
            double t_ = 0.0;
            double dt_ = 0.001;
            while (_runUpdater.load()) {
                robotData.basePos.x = sin(3.14 * t_);
                robotData.basePos.y = cos(3.14 * t_);
                robotData.basePos.z = 0.45;
                robotData.baseRot.fromEuler(0.0, 0.0, t_);

                robotData.jointPos[0] = 0.0;
                robotData.jointPos[1] = 0.7 * sin(3.14 * t_) * sin(3.14 * t_);
                robotData.jointPos[2] = -2.0 * robotData.jointPos[1];

                robotData.jointPos[3] = 0.0;
                robotData.jointPos[4] = 0.7 * sin(3.14 * t_) * sin(3.14 * t_);
                robotData.jointPos[5] = -2.0 * robotData.jointPos[1];
                robotData.jointPos[6] = 0.0;
                robotData.jointPos[7] = 0.7 * cos(3.14 * t_) * cos(3.14 * t_);
                robotData.jointPos[8] = -2.0 * robotData.jointPos[4];
                robotData.jointPos[9] = 0.0;

                robotData.jointPos[10] = 0.0;
                robotData.jointPos[11] = 0.7 * sin(3.14 * t_) * sin(3.14 * t_);
                robotData.jointPos[12] = -2.0 * robotData.jointPos[1];
                robotData.jointPos[13] = 0.0;
                robotData.jointPos[14] = 0.7 * cos(3.14 * t_) * cos(3.14 * t_);
                robotData.jointPos[15] = -2.0 * robotData.jointPos[4];
                robotData.jointPos[16] = 0.0;

                std::this_thread::sleep_for(std::chrono::milliseconds((long)(dt_ * 1000)));
                t_ += dt_;
            }
        });
    }
    ~BMR()
    {
        _runUpdater.store(false);
        _dataUpdater.join();
    }

private:
    std::thread _dataUpdater;
    std::atomic_bool _runUpdater;
};

#endif // BMR_H