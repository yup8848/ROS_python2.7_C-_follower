#include "SocketMessage.h"
#include "../Common/ErrorCodeDef.h"
#include "Socket.h"
#include <iostream>
using namespace std;
int CSocketMessage::DispatchSocketMessage(CSocketData& socketData)
{
	int ret = err_Success;
	if (MESS_READ ==  socketData.GetMessageType())
	{
		HandlerArrivedData(socketData);
	}
	else if (MESS_CLOSE == socketData.GetMessageType())
	{
		HandlerCloseMessage(socketData);
	}
	else if (MESS_NO_ALIVE == socketData.GetMessageType())
	{
		HandlerNoAliveMessage(socketData);
	}
	else if (MESS_CONNECTED == socketData.GetMessageType())
	{
		HandleConnectedMessage(socketData);
	}
	else
	{
		ret = err_ProtocolUnsupport;
	}
	return ret;
}

int CSocketMessage::HandleConnectedMessage(CSocketData& socketData)
{
	return err_Success;
}

int CSocketMessage::HandlerCloseMessage(CSocketData& socketData)
{
	return err_Success;
}
int CSocketMessage::HandlerNoAliveMessage(CSocketData& socketData)
{
	CSocket::GetInstance()->DisConnect(socketData.GetIpv4(), socketData.GetPort());
	return err_Success;
}

int	CSocketMessage::ClientConnected(string listenerPort, eSocketType socketType, eProtocolType protocolType, string clienIpv4, string clientPort)
{
	return err_Success;
}
