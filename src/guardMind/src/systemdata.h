//////////////////////////////////////////////////////////////////////////
//  File Name:  systemdata.h
//  Author:     Tai
//  Email:      tai.pang@outlook.com
//  Date:       2022.12.16
//////////////////////////////////////////////////////////////////////////
#ifndef CSYSTEMDATA_H
#define CSYSTEMDATA_H
#include <mutex>
// #include "main.h"
#include "string.h"

typedef enum
{
    SM_MANUAL = 0,
    SM_AUTO_GARAGE_EXCHANGE,
    SM_AUTO_GARAGE_TRANSITION,
    SM_AUTO_ROBOT_NAVIGATION,
    SM_AUTO_BANK_EXCHANGE,
    SM_ERROR
} eGlobalStateMachineType;

// 指令集
typedef enum eCmdType
{
    CMD_ROBOTS_START = 0,
    CMD_ROBOTS_STOP,
    CMD_ROBOTS_STATE_DATA,
    CMD_ROBOTS_IMU_DATA,
    CMD_ROBOTS_MOVE,
    CMD_ROBOTS_OPENCOVER,
    CMD_ROBOTS_CLOSECOVER
} eCmdType;
// 移动类型
typedef enum
{
    PROTOCOL_MOVE_UP,
    PROTOCOL_MOVE_DOWN,
    PROTOCOL_TURN_LEFT,
    PROTOCOL_TURN_RIGHT,
    PROTOCOL_MOVE_UP_CONTINUE,
    PROTOCOL_MOVE_DOWN_CONTINUE,
    PROTOCOL_TURN_LEFT_CONTINUE,
    PROTOCOL_TURN_RIGHT_CONTINUE,
    PROTOCOL_MOVE_SPEED,
    PROTOCOL_SET_PID_KD,
    PROTOCOL_SET_PID_KP,
} eMoveType;

#pragma pack(1)
typedef struct
{
    eGlobalStateMachineType eDisPlayStateMachine = SM_MANUAL;
    bool bIsRobotInPosition = false;
    bool bIsRobotEmpty = false;
    bool bIsRobotCoverOpen = false;
    bool bIsRobotCoverClosed = false;
} sDisplayStateDataType;
typedef struct
{
    eGlobalStateMachineType eMindStateMachine = SM_MANUAL;
    double fVoltage = 0; // 电压
    double fPrecent = 0; // 电量百分比
    double fCapcity = 0; // 电能容量
    double fLeftMotorCurrent = 0;
    double fRightMotorCurrent = 0;
    int iLeftMotorEncoderCounts = 0;
    int iRightMotorEncoderCounts = 0;
    int iControllerTemp = 0;
} sMindStateDataType;

typedef struct
{
    eGlobalStateMachineType eGarageStateMachine = SM_MANUAL;
} sGarageStateDataType;

typedef struct
{
    double fOrientation[4] = {0.0};
    double fOrientation_covariance[9] = {0.0};
    double fAngular_velocity[3] = {0.0};
    double fAngular_velocity_covariance[9] = {0.0};
    double fLinear_acceleration[3] = {0.0};
    double fLinear_acceleration_covariance[9] = {0.0};
} sRobotsImuDataType;

typedef struct
{
    eMoveType iMoveType;
    double dMoveValue = 0;
} sRobotsMoveType;
#pragma pack()

class CSystemData
{
public:
    CSystemData();
    template <typename T>
    void getData(T &data);
    template <typename T>
    void setData(T &data);

private:
    std::mutex lock_m_sDisplayStateData;
    sDisplayStateDataType m_sDisplayStateData;
    std::mutex lock_m_sMindStateData;
    sMindStateDataType m_sMindStateData;
    std::mutex lock_m_sGarageStateData;
    sGarageStateDataType m_sGarageStateData;
    std::mutex lock_m_sRobotsImuData;
    sRobotsImuDataType m_sRobotsImuData;
    std::mutex lock_m_sRobotsMove;
    sRobotsMoveType m_sRobotsMove;

private:
    // 字符数据还原为目标数据
    template <typename _DesData>
    void getSocketdata(_DesData &desData, unsigned char *pData, int iLengthData)
    {
        if (iLengthData > 8)
        {
            int iSize = sizeof(desData) < iLengthData - 8 ? sizeof(desData) : iLengthData - 8;
            memcpy((void *)&desData, pData + 8, iSize);
        }
    }
};

#endif // CSYSTEMDATA_H
