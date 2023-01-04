#pragma once
#include <thread>
#include <pthread.h>
#include <ros/ros.h>
#include <string>
#include "geometry_msgs/Twist.h"
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
#include "../Common/CondWait.h"
#include <mutex>
#include "systemdata.h"

using namespace cv;
enum motionStatus
{
    UNINITIALIZED, // 未初始化
    INIT,          // 已初始化
    START,         // 开始运动
    RUNNING,       // 正在运行中 通过路径信息运行
    STOP,          // 停止运动
    UP_CONTINUE,   // 向前连续运动
    DOWN_CONTINUE, // 向后连续运动
    TURN_LEFT,     // 连续的向左转向
    TURN_RIGHT,    // 连续的向右转向
};
class CMotionController
{
public:
    CMotionController() {}
    virtual ~CMotionController() {}
    void Init(ros::Publisher &ros); // 初始化
    int HandlerController();
    int Stop();
    int Start();
    void SetMotionStatus(motionStatus eStatus);       // 设置运动状态
    motionStatus GetMotionStatus();                   // 得到运动状态
    geometry_msgs::Twist calc_target(cv::Mat &frame); // 通过图片计算位置
    geometry_msgs::Twist Pid(double dError);
    void addMatImage(cv::Mat mat);        // 增加最新的图片
    void addErrorDistance(double dError); // 增加位置偏差值

    void SetMaxLinearX(double x);                               // 设置最大进行位移
    void SetMoveUp(double dDistance, bool bContinuous = false); // 设置向前移动    一次一步 (默认)      bContinuous为真为连续
    void SetMoveDown(double dDistance, bool bContinuous = false);
    void SetTurnLeft(double dAngle, bool bContinuous = false);
    void SetTurnRight(double dAngle, bool bContinuous = false);
    void SetStepDistance(const double dDistance);
    void GetStepDistance(double &dDistance);
    void SetPidKp(const double dPidKp);
    void SetPidKd(const double dPidKd);

private:
    Mat selectROI(Mat &frame);

private:
    // XCondWait m_WaitCmd;               //
    
    std::mutex lock_m_eMotionStatus;
    motionStatus m_eMotionStatus;       // 运动状态
    std::mutex lock_m_vecMatImage;      // 容器锁
    std::vector<cv::Mat> m_vecMatImage; // 图像容器
    ros::Publisher m_rosPub;
    std::mutex lock_m_vecErrorDistance;
    std::vector<double> m_vecErrorDistance;
    int move(geometry_msgs::Twist target); //
    std::vector<std::thread *> m_treads;   // 线程容器
    std::mutex lock_m_dMaxLinearX;
    double m_dMaxLinearX = 50; // x方向最大速度 用于调节速度 超过最大值按这个值计算
    std::mutex lock_m_dStepDistance;
    double m_dStepDistance = 0.1; // 一步的位移
    double m_dPidKp = 0.01;
    double m_dPidKd = 0;
};
