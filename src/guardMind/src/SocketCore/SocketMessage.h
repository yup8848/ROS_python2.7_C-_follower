#pragma once
#include <string>
#include "SocketData.h"
using std::string;

class CSocketMessage
{
public:
	CSocketMessage(){}
	virtual ~CSocketMessage(){}

	virtual int HandlerArrivedData(CSocketData& socketData) = 0;
	virtual int HandlerCloseMessage(CSocketData& socketData);
	virtual int HandlerNoAliveMessage(CSocketData& socketData);
	virtual int HandleConnectedMessage(CSocketData& socketData);
    virtual	int	ClientConnected(string listenerPort, eSocketType socketType, eProtocolType protocolType, string clienIpv4, string clientPort);
public:
	int DispatchSocketMessage(CSocketData& socketData);
};
