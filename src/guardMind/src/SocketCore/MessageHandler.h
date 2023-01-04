#pragma once
#include "SocketMessage.h"
#include <map>
#include "systemdata.h"
//#include "DeviceTool.h"
//#include "Common/DAGFileWritter.h"
#include "protocol.h"

class CMessageHandler :public CSocketMessage
{
public:
    CMessageHandler() {}
    virtual ~CMessageHandler() {}
public:
    virtual int HandlerArrivedData(CSocketData& socketData);
    int	ClientConnected(string listenerPort, eSocketType socketType, eProtocolType protocolType, string clienIpv4, string clientPort);
    int HandlerCloseMessage(CSocketData& socketData);
    int HandleConnectedMessage(CSocketData& socketData);
    int HandlerNoAliveMessage(CSocketData& socketData);

private:
    //std::map<string, CDAGFileWritter> m_mapFile;

};


