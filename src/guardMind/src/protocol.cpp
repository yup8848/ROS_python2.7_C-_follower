//////////////////////////////////////////////////////////////////////////
//  File Name:  protocol.cpp
//  Author:     Tai
//  Email:      tai.pang@outlook.com
//  Date:       2022.12.19
//////////////////////////////////////////////////////////////////////////
#include "protocol.h"
// #include "main.h"
// #include <QDebug>
#include "motionController.h"

CSystemData g_sysData;
extern CMotionController g_motionController;

CProtocol::CProtocol()
{
    m_ip_MIND = "192.168.1.30";
    m_port_MIND = "8000";
    m_deviceID_MIND = "设备ID";
    m_ip_DISPLAY = "192.168.1.3";
    m_port_DISPLAY = "8000";
    m_deviceID_DISPLAY = "设备ID";
    m_ip_GARAGE = "192.168.1.126";
    m_port_GARAGE = "8000";
    m_deviceID_GARAGE = "设备ID";
}
CProtocol::~CProtocol()
{
}

int CProtocol::setSocketInfo(eProtocolType protocolType, string ip, string port, string deviceID)
{
    switch (protocolType)
    {
    case PROTOCOL_DISPLAYTOMIND:
    {
        m_ip_MIND = ip;
        m_port_MIND = port;
        m_deviceID_MIND = deviceID;
        break;
    }
    case PROTOCOL_GARAGETODISPLAY:
    case PROTOCOL_MINDTODISPLAY:
    {
        m_ip_DISPLAY = ip;
        m_port_DISPLAY = port;
        m_deviceID_DISPLAY = deviceID;
        break;
    }
    case PROTOCOL_DISPLAYTOGARAGE:
    {
        m_ip_GARAGE = ip;
        m_port_GARAGE = port;
        m_deviceID_GARAGE = deviceID;
        break;
    }
    default:
        break;
    }
    return 0;
}
int CProtocol::parseProtocol(CSocketData &socketData)
{

    eProtocolType protocolType = socketData.GetMessageProtolType();
    int iCmdType;
    int iLength = socketData.GetRawDataLength();
    unsigned char *pData = socketData.GetRawData();
    if (iLength < 4)
    {
        return -1;
    }
    memcpy((char *)&iCmdType, (char *)pData, 4);

    switch (protocolType)
    {
    case PROTOCOL_NO_CASE:
        break;
    case PROTOCOL_UNKNOWN:
        break;
    case PROTOCOL_DISPLAYTOMIND:
    {
        switch (iCmdType)
        {
        case CMD_ROBOTS_STATE_DATA:
        {
            sMindStateDataType data;
            writeCharDataToDesData(data, pData, iLength);
            g_sysData.setData(data);
            break;
        }
        case CMD_ROBOTS_START:
        {
            cout << "ROBOTS_START" << endl;
            sMindStateDataType mindState;
            g_sysData.getData(mindState);
            mindState.eMindStateMachine = SM_AUTO_ROBOT_NAVIGATION;
            g_sysData.setData(mindState);
            //            m_motionController.SetMotionStatus(START);
            break;
        }
        case CMD_ROBOTS_STOP:
        {
            cout << "CMD_ROBOTS_STOP" << endl;
            sMindStateDataType mindState;
            g_sysData.getData(mindState);
            mindState.eMindStateMachine = SM_MANUAL;
            g_sysData.setData(mindState);
            //            m_motionController.SetMotionStatus(STOP);
            break;
        }
            //小车手动控制指令
        case CMD_ROBOTS_MOVE:
        {
            cout << "receive CMD_ROBOTS_MOVE." << endl;
            sRobotsMoveType sRobostMove;
            memcpy((char *)&sRobostMove, (char *)(pData + 8), sizeof(sRobotsMoveType));
            g_sysData.setData(sRobostMove);
            switch (sRobostMove.iMoveType)
            {
            case PROTOCOL_MOVE_UP:
                g_motionController.SetMoveUp(sRobostMove.dMoveValue);
                break;
            case PROTOCOL_MOVE_DOWN:
                g_motionController.SetMoveDown(sRobostMove.dMoveValue);
                break;
            case PROTOCOL_TURN_LEFT:
                g_motionController.SetTurnLeft(sRobostMove.dMoveValue);
                break;
            case PROTOCOL_TURN_RIGHT:
                g_motionController.SetTurnRight(sRobostMove.dMoveValue);
                break;
            case PROTOCOL_MOVE_UP_CONTINUE:
                g_motionController.SetMoveUp(sRobostMove.dMoveValue, true);
                break;
            case PROTOCOL_MOVE_DOWN_CONTINUE:
                g_motionController.SetMoveDown(sRobostMove.dMoveValue, true);
                break;
            case PROTOCOL_TURN_LEFT_CONTINUE:
                g_motionController.SetTurnLeft(sRobostMove.dMoveValue, true);
                break;
            case PROTOCOL_TURN_RIGHT_CONTINUE:
                g_motionController.SetTurnRight(sRobostMove.dMoveValue, true);
                break;
            case PROTOCOL_MOVE_SPEED:
                g_motionController.SetStepDistance(sRobostMove.dMoveValue);
                break;
            case PROTOCOL_SET_PID_KD:
                g_motionController.SetPidKd(sRobostMove.dMoveValue);
                break;
            case PROTOCOL_SET_PID_KP:
                g_motionController.SetPidKp(sRobostMove.dMoveValue);
                break;
            default:
                break;
            }
        }
        default:
            break;
        }
        break;
    }
    case PROTOCOL_MINDTODISPLAY:
    {
        //        qDebug() <<"parseProtocol-->PROTOCOL_MINDTODISPLAY. Cmd:"<<iCmdType<<Qt::endl;
        switch (iCmdType)
        {
        case CMD_ROBOTS_STATE_DATA:
        {
            sMindStateDataType data;
            writeCharDataToDesData(data, pData, iLength);
            g_sysData.setData(data);
            break;
        }
        case CMD_ROBOTS_IMU_DATA:
        {
            sRobotsImuDataType data;
            writeCharDataToDesData(data, pData, iLength);
            g_sysData.setData(data);
            break;
        }
        case CMD_ROBOTS_MOVE:
        {
            sRobotsMoveType data;
            writeCharDataToDesData(data, pData, iLength);
            g_sysData.setData(data);
            break;
        }
        default:
            break;
        }
        break;
    }
    case PROTOCOL_GARAGETODISPLAY:
    { /*
//        qDebug() <<"parseProtocol-->PROTOCOL_GARAGETODISPLAY. Cmd:"<<iCmdType<<Qt::endl;
switch (iCmdType){
case CMD_ROBOTS_STATE_DATA:{
sGarageStateDataType data;
writeCharDataToDesData(data,pData,iLength);
g_sysData.setData(data);
break;
}
case CMD_ROBOTS_OPENCOVER:{
// g_motorControl.bcmdOpenCoverFlag = true;
break;
}
case CMD_ROBOTS_CLOSECOVER:{
// g_motorControl.bcmdCloseCoverFlag = true;
break;
}
default:
break;
}*/
        break;
    }
    case PROTOCOL_DISPLAYTOGARAGE:
    {
        /*        switch (iCmdType){
        case CMD_ROBOTS_STATE_DATA:{
            sDisplayStateDataType data;
            writeCharDataToDesData(data,pData,iLength);
            g_sysData.setData(data);
            break;
        }
        default:
            break;
        }*/
        break;
    }
    default:
        break;
    }
    return 0;
}

int CProtocol::sendProtocol(eProtocolType protocolType, eCmdType iCmdType)
{
    int iLength;
    char *pSocketData;
    char *pdata = NULL;

    switch (protocolType)
    {
    case PROTOCOL_NO_CASE:
        break;
    case PROTOCOL_UNKNOWN:
        break;
    case PROTOCOL_DISPLAYTOMIND:
    {
        switch (iCmdType)
        {
        case CMD_ROBOTS_START:
        case CMD_ROBOTS_STOP:
        {
            iLength = -1;
            pSocketData = writeDesDataToCharData(iCmdType, iLength, pdata);
            break;
        }
        case CMD_ROBOTS_STATE_DATA:
        {
            sDisplayStateDataType data;
            g_sysData.getData(data);
            iLength = sizeof(data);
            pSocketData = writeDesDataToCharData(iCmdType, iLength, data);
            break;
        }
        case CMD_ROBOTS_IMU_DATA:
        {
            sRobotsImuDataType data;
            g_sysData.getData(data);
            iLength = sizeof(data);
            pSocketData = writeDesDataToCharData(iCmdType, iLength, data);
            break;
        }
        default:
            break;
        }
        CSocket::GetInstance()->Connect(m_ip_MIND, m_port_MIND, m_deviceID_MIND, SOCKET_TCP, PROTOCOL_DISPLAYTOMIND, 3000);
        CSocket::GetInstance()->WriteFrame(m_ip_MIND, m_port_MIND, pSocketData, iLength);
        break;
    }
    case PROTOCOL_MINDTODISPLAY:
    {
        switch (iCmdType)
        {
        case CMD_ROBOTS_STATE_DATA:
        {
            sMindStateDataType data;
            g_sysData.getData(data);
            iLength = sizeof(data);
            pSocketData = writeDesDataToCharData(iCmdType, iLength, data);
            break;
        }
        case CMD_ROBOTS_MOVE:
        {
            iLength = -1;
            pSocketData = writeDesDataToCharData(iCmdType, iLength, pdata);
            break;
        }
        default:
            break;
        }
        CSocket::GetInstance()->Connect(m_ip_DISPLAY, m_port_DISPLAY, m_deviceID_DISPLAY, SOCKET_TCP, PROTOCOL_GARAGETODISPLAY, 3000);
        CSocket::GetInstance()->WriteFrame(m_ip_DISPLAY, m_port_DISPLAY, pSocketData, iLength);
        break;
    }
    case PROTOCOL_DISPLAYTOGARAGE:
    {
        // qDebug() <<"sendProtocol-->PROTOCOL_DISPLAYTOGARAGE, cmd:"<<iCmdType<<Qt::endl;
        switch (iCmdType)
        {
        case CMD_ROBOTS_STATE_DATA:
        {
            sDisplayStateDataType data;
            g_sysData.getData(data);
            iLength = sizeof(data);
            pSocketData = writeDesDataToCharData(iCmdType, iLength, data);
            break;
        }
        default:
            break;
        }
        CSocket::GetInstance()->Connect(m_ip_GARAGE, m_port_GARAGE, m_deviceID_GARAGE, SOCKET_TCP, PROTOCOL_DISPLAYTOGARAGE, 3000);
        CSocket::GetInstance()->WriteFrame(m_ip_GARAGE, m_port_GARAGE, pSocketData, iLength);
        break;
    }
    case PROTOCOL_GARAGETODISPLAY:
    { /*
switch (iCmdType)
{
case  CMD_ROBOTS_STATE_DATA:{
sGarageStateDataType data;
g_sysData.getData(data);
iLength = sizeof(data);
pSocketData = writeDesDataToCharData(iCmdType, iLength, data);
break;
}
default:
break;
}
CSocket::GetInstance()->Connect(m_ip_DISPLAY, m_port_DISPLAY,m_deviceID_DISPLAY, SOCKET_TCP, PROTOCOL_GARAGETODISPLAY,3000);
CSocket::GetInstance()->WriteFrame(m_ip_DISPLAY, m_port_DISPLAY, pSocketData, iLength);
*/
        break;
    }
    default:
        break;
    }

    return 0;
}

// template<typename _Data>
// int sendProtocol(E_PROTOCOL_TYPE protocolType, eCmdType cmdType, _Data& desData){

//}
