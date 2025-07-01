#ifndef __DATA_TYPES_H__
#define __DATA_TYPES_H__

#include <chrono>
#include <cmath>
#include <ctime>
#include <dtCore/type.h>
#include <thread>

typedef struct _Point
{
    double x;
    double y;
    double z;
} Point;

typedef struct _Quaternion
{
    double x;
    double y;
    double z;
    double w;

    void fromEuler(double roll, double pitch, double yaw)
    { // roll (x), pitch (y), yaw (z), angles are in radians
        double cr = cos(roll * 0.5);
        double sr = sin(roll * 0.5);
        double cp = cos(pitch * 0.5);
        double sp = sin(pitch * 0.5);
        double cy = cos(yaw * 0.5);
        double sy = sin(yaw * 0.5);

        w = cr * cp * cy + sr * sp * sy;
        x = sr * cp * cy - cr * sp * sy;
        y = cr * sp * cy + sr * cp * sy;
        z = cr * cp * sy - sr * sp * cy;
    }
} Quaternion;

typedef struct _Odom
{
    Point position;
    Quaternion orientation;
} Odom;

typedef struct _SE2Pose
{
    double x;
    double y;
    double th;
} SE2Pose;

typedef struct _SE2Velocity
{
    double vx;
    double vy;
    double w;
} SE2Velocity;

#define NAVCOMMAND_TRAJ_MAX (32)

typedef struct _SE2Trajectory
{
    typedef struct _SE2TrajectoryPoint
    {
        SE2Pose pose;
        double duration;
    } SE2TrajectoryPoint;
    SE2TrajectoryPoint points[NAVCOMMAND_TRAJ_MAX];
    dt::TimeStamp ref_time;
    int nop; //<! number of valid points
} SE2Trajectory;

typedef struct _RobotCommand
{
    typedef struct _NavCommand
    {
        enum CommandType
        {
            NavCommandType_Unknown = 0,
            NavCommandType_SE2Traj,
            NavCommandType_SE2Pose,
            NavCommandType_SE2Vel,
            // NavCommandType_SE3Traj,
            // NavCommandType_SE3Pose,
            // NavCommandType_SE3Vel,
        };
        CommandType cmd_type{NavCommandType_Unknown};
        dt::TimeStamp recv_time;
        dt::TimeStamp end_time; //<! The timestamp (in robot time) that the command is valid until.
        SE2Trajectory traj;     //<! trajectory(route) to follow
        SE2Pose pose;           //<! goal pose
        SE2Velocity vel;
    } NavCommand;
    NavCommand nav;
} RobotCommand;

#endif // __DATA_TYPES_H__