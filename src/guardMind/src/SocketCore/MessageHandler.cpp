#include "CommonDef.h"
//#include "motionController.h"
#include <iostream>
#include <thread>
#include "Log.h"
#include "Common.h"
#include "SocketCore/Socket.h"
#include "MessageHandler.h"
#include "ErrorCodeDef.h"



//extern  CMotionController m_motionController ;
//extern std::mutex lock_m_bulldogDriver;
//extern RaspberryProtocol ::RobotsDriver  m_bulldogDriver;


using namespace std;



//�����ʹ�
int CMessageHandler::HandlerArrivedData(CSocketData& socketData)
{    
    return CProtocol::GetInstance()->parseProtocol(socketData);;
}
//���ӹر�
int CMessageHandler::HandlerCloseMessage(CSocketData& socketData)
{
    CSocketMessage::HandlerCloseMessage(socketData);
    return err_Success;
}

//��������
int CMessageHandler::HandleConnectedMessage(CSocketData& socketData)
{

    CSocketMessage::HandleConnectedMessage(socketData);
    eProtocolType protocolType = socketData.GetMessageProtolType();
    return err_Success;
}
//���ӵ���
int CMessageHandler::HandlerNoAliveMessage(CSocketData& socketData)
{
    CSocketMessage::HandlerNoAliveMessage(socketData);
    return err_Success;
}
//�ͻ������ӷ������˿�
int	CMessageHandler::ClientConnected(string listenerPort, eSocketType socketType, eProtocolType protocolType, string clienIpv4, string clientPort)
{
    //CSocket::GetInstance()->DisConnect(clienIpv4, clientPort);
    return err_Success;
}

int HandleConnectedMessage(CSocketData& socketData)//�
{
    return err_Success;
}
