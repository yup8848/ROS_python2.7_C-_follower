//////////////////////////////////////////////////////////////////////////
//  File Name:  protocol.h
//  Author:     Tai
//  Email:      tai.pang@outlook.com
//  Date:       2022.12.19
//////////////////////////////////////////////////////////////////////////
#ifndef CPROTOCOL_H
#define CPROTOCOL_H
#include "systemdata.h"
#include "SocketMessage.h"
#include "Socket.h"
// #include "main.h"

class CProtocol
{
private:
    CProtocol(const CProtocol&) = delete;
    CProtocol& operator=(const CProtocol&) = delete;

    CProtocol();
    virtual ~CProtocol();
public:
    static CProtocol* GetInstance()
    {
        static CProtocol instance;
        return &instance;
    }

    int parseProtocol(CSocketData& socketData);
    int sendProtocol(eProtocolType protocolType, eCmdType cmdType);
    int setSocketInfo(eProtocolType protocolType, string ip, string port, string deviceID);
//    template<typename _Data>
//    int sendProtocol(E_PROTOCOL_TYPE protocolType, eCmdType cmdType, _Data& desData);
private:
    //字符数据还原为目标数据
    template<typename   _DesData>
    void writeCharDataToDesData(_DesData& desData, unsigned char* pData, int iLengthData)
    {
        if (iLengthData > 8)
        {
            int iSize = sizeof(desData) < iLengthData-8 ? sizeof(desData) : iLengthData-8;
            memcpy((void*)&desData, pData+8, iSize);
        }
    }
    //将目标数据转为字符数据  没有带数据的指令直接 输入eCmd即可
    template<typename   _Data>
    char* writeDesDataToCharData(eCmdType eCmd, int &iCharlength,_Data& desData )
    {
        if(-1 == iCharlength  )
        {
            char* pDesData = new char[4];
            int  iCmd = eCmd;
            memcpy(pDesData, ( char*)&iCmd, 4) ;  //cmd指令方desData
            iCharlength = 4;
            return pDesData;
        }
        iCharlength = sizeof(desData)+8;
        char* pDesData = new char[iCharlength];
        int iLengthData = sizeof(desData);
        memcpy(pDesData, ( char*)&eCmd, 4) ;                                       //cmd指令方
        memcpy(pDesData+4, (char*)&iLengthData, 4);                    //数据大小
        memcpy(pDesData+8,(char*)&desData,iLengthData);       //实际数据
        return pDesData;
    }

private:
    string m_ip_MIND;
    string m_port_MIND;
    string m_deviceID_MIND;
    string m_ip_DISPLAY;
    string m_port_DISPLAY;
    string m_deviceID_DISPLAY;
    string m_ip_GARAGE;
    string m_port_GARAGE;
    string m_deviceID_GARAGE;
};

#endif // CPROTOCOL_H
