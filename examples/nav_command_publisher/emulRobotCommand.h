#ifndef __EMUL_ROBOTCOMMAND_H__
#define __EMUL_ROBOTCOMMAND_H__

#include "dataTypes.h"
#include <atomic>
#include <chrono>
#include <cmath>
#include <thread>

class RobotCommandEmulator
{
public:
    RobotCommand cmd;

public:
    RobotCommandEmulator()
    {
        _dataUpdater = std::thread([this] {
            _runUpdater.store(true);
            double t_ = 0.0;
            double dt_ = 0.01;
            int nop = 10;
            int cpi = 0; // current point index (0 ~ nop-1)
            while (_runUpdater.load())
            {

                cmd.nav.pose.x += 0.1;
                cmd.nav.pose.y += 0.03;
                cmd.nav.pose.th += 0.1;
                if (cmd.nav.pose.x > 10.0) cmd.nav.pose.x -= 10.0;
                if (cmd.nav.pose.y > 10.0) cmd.nav.pose.y -= 10.0;
                if (cmd.nav.pose.th > 2.0 * 3.14159) cmd.nav.pose.th -= (2.0 * 3.14159);

                cmd.nav.vel.vx = cmd.nav.pose.x;
                cmd.nav.vel.vy = cmd.nav.pose.y;
                cmd.nav.vel.w = cmd.nav.pose.th;

                cmd.nav.traj.points[cpi].pose = cmd.nav.pose;
                cmd.nav.traj.points[cpi].duration = 1.0;
                cmd.nav.traj.nop = std::max(nop, ++cpi);
                if (cpi >= nop) cpi = 0;

                std::this_thread::sleep_for(std::chrono::milliseconds((long)(dt_ * 1000)));
                t_ += dt_;
            }
        });
    }
    ~RobotCommandEmulator()
    {
        _runUpdater.store(false);
        _dataUpdater.join();
    }

private:
    std::thread _dataUpdater;
    std::atomic<bool> _runUpdater;
};

#endif // __EMUL_ROBOTCOMMAND_H__