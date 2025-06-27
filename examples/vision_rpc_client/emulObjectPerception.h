#ifndef __EMUL_OBJECT_PERCEPTION_H__
#define __EMUL_OBJECT_PERCEPTION_H__

#include "dataTypes.h"
#include <atomic>
#include <chrono>
#include <cmath>
#include <dtMath/dtMath.h>
#include <thread>

class ObjectPerceptionEmulator
{
public:
    ObjectMap objmap;

public:
    ObjectPerceptionEmulator()
    {
        for (int i = 0; i < 10; i++)
        {
            objmap.objects[i].id = i;
        }

        _dataUpdater = std::thread([this] {
            _runUpdater.store(true);
            double t_ = 0.0;
            double dt_ = 0.1;
            uint32_t detected_obj_id_ = 0;
            while (_runUpdater.load())
            {
                detected_obj_id_++;
                if (detected_obj_id_ > 10)
                    detected_obj_id_ = 0;

                for (auto &obj : objmap.objects)
                {
                    obj.valid = (obj.id == detected_obj_id_);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds((long)(dt_ * 1000)));
                t_ += dt_;
            }
        });
    }
    ~ObjectPerceptionEmulator()
    {
        _runUpdater.store(false);
        _dataUpdater.join();
    }

private:
    std::thread _dataUpdater;
    std::atomic<bool> _runUpdater;
};

#endif // __EMUL_OBJECT_PERCEPTION_H__