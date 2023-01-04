//////////////////////////////////////////////////////////////////////////
//  File Name:  systemdata.cpp
//  Author:     Tai
//  Email:      tai.pang@outlook.com
//  Date:       2022.12.16
//////////////////////////////////////////////////////////////////////////
#include "systemdata.h"

CSystemData::CSystemData()
{


}

template <>
void CSystemData::getData(sDisplayStateDataType& data){
    std::lock_guard<std::mutex> lock(lock_m_sDisplayStateData);
    data = m_sDisplayStateData;
}
template <>
void CSystemData::setData(sDisplayStateDataType& data){
    std::lock_guard<std::mutex> lock(lock_m_sDisplayStateData);
    m_sDisplayStateData = data;
}

template <>
void CSystemData::getData(sMindStateDataType& data){
    std::lock_guard<std::mutex> lock(lock_m_sMindStateData);
    data = m_sMindStateData;
}
template <>
void CSystemData::setData(sMindStateDataType& data){
    std::lock_guard<std::mutex> lock(lock_m_sMindStateData);
    m_sMindStateData = data;
}

template <>
void CSystemData::getData(sGarageStateDataType& data){
    std::lock_guard<std::mutex> lock(lock_m_sGarageStateData);
    data = m_sGarageStateData;
}
template <>
void CSystemData::setData(sGarageStateDataType& data){
    std::lock_guard<std::mutex> lock(lock_m_sGarageStateData);
    m_sGarageStateData = data;
}

template <>
void CSystemData::getData(sRobotsImuDataType& data){
    std::lock_guard<std::mutex> lock(lock_m_sRobotsImuData);
    data = m_sRobotsImuData;
}
template <>
void CSystemData::setData(sRobotsImuDataType& data){
    std::lock_guard<std::mutex> lock(lock_m_sRobotsImuData);
    m_sRobotsImuData = data;
}
template <>
void CSystemData::getData(sRobotsMoveType& data){
    std::lock_guard<std::mutex> lock(lock_m_sRobotsMove);
    data = m_sRobotsMove;
}


template <>
void CSystemData::setData(sRobotsMoveType& data){
    std::lock_guard<std::mutex> lock(lock_m_sRobotsMove);
    m_sRobotsMove = data;
}
