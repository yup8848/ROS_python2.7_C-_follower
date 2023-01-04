#pragma once
#include <string>
#include <iostream>
#include <cstring>
#include <map>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include "../Common/CommonDef.h"
#include "SocketMessage.h"
#include "../Common/CondWait.h"
#include "../Common/ErrorCodeDef.h"

#ifdef _WIN32
#include <Winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#endif
#include "SocketData.h"
#include "../Common/DataManager.h"
using namespace std;
using namespace std::chrono;

#define SOCKET_FRAME_SIZE 1024*100	//�������100K
#define SOCKET_RECV_BUF_SIZE SOCKET_FRAME_SIZE//1024*100
#define SOCKET_QUENE_MAX 3000

#ifdef _WIN32
#define GET_LAST_ERROR GetLastError()
#else
#define GET_LAST_ERROR errno
#endif

typedef struct ST_SOCKET {
	string deviceId;
	string ipv4;
	string port;
	string localPort;
	sockaddr_in sockDest;
	sockaddr_in sockSrc;
    eProtocolType protocolType;
    eSocketType sockType;
	int sock;
	//int nType;//�״�/����/��������
	long long llAliveTimeStamp;//���һ���յ����ݰ���ʱ���
	long long llTimeout2Close;
	bool dataProcessInOrder;
	bool tcpFrameNeedPreProcess;

	//����SDK��ͨ�ž�����ⲿ����

	ST_SOCKET()
	{
		tcpFrameNeedPreProcess = false;
		dataProcessInOrder = false;
		llAliveTimeStamp = 0;
		llTimeout2Close = KEEP_ALIVE_TIMEOUT_SECOND_DEFAULT;
		sock = -1;
	}
}StSocket;

class CSocket
{
private:
	// ��ֹ���ƹ��캯��
	CSocket(const CSocket&) = delete;
	// ��ֹ����ֵ������
	CSocket& operator=(const CSocket&) = delete;

	CSocket();
	virtual ~CSocket();
public:
	static CSocket* GetInstance()
	{
		static CSocket instance;
		return &instance;
	}
	int				GetMapKey(string ipv4, string port, string& key);

	//��ʼ��ֹͣ����
	int				Start();
	void			Stop();
	//������ָֹͣ���˿ڼ���
    int				AddListener(int port, eSocketType socketType, eProtocolType protocolType = PROTOCOL_NO_CASE,
							long long second2Close = KEEP_ALIVE_TIMEOUT_SECOND_DEFAULT, bool dataProcessInOrder = false);
	int				RemoveListener(int port);
	//Ĭ��TCPʹ���첽���ӷ�ʽ
    int				Connect(string ipv4, int port, string deviceId, eSocketType socketType, eProtocolType protocolType = PROTOCOL_NO_CASE,
							long long second2Close = KEEP_ALIVE_TIMEOUT_SECOND_DEFAULT, bool asyn2Tcp = true, bool dataProcessInOrder = false);
    int				Connect(string ipv4, string port, string deviceId, eSocketType socketType, eProtocolType protocolType = PROTOCOL_NO_CASE,
							long long second2Close = KEEP_ALIVE_TIMEOUT_SECOND_DEFAULT, bool asyn2Tcp = true, bool dataProcessInOrder = false);
	//��ָ��������д�뱨��
	int				WriteFrame(string ipv4, string port, const char* pData, int nLen);

	//TCP/UDPֱ������-д-��socket��������
    int				WriteRead(string ipv4, string port, eSocketType socketType, const char* sendBuf, int sendLen, char* recvBuf, int& recvLenInOut, int timeout = 500);

	void			DisConnect(string ipv4, string port);
	bool			IsConnected(string ipv4, string port);//�Ƿ��Ѿ����ӳɹ�
	bool			IsConnecting(string ipv4, string port);//�Ƿ��������ӣ����TCP�첽���ӷ�ʽ
	void			_CloseSocket(int sock);
	void			RegMessageHandler(CSocketMessage* sockMessage);


protected:
	int				_SocketRecvFrom(struct sockaddr_in& server, int& sock, char* recvBuf, int& recvLenInOut, int timeout);
	//TCP���ӣ�֧���첽���Ӻ�ͬ�����ӣ�ͬ������ͨ���첽��ʽ��select��飩֧���Զ��峬ʱ��
	bool			_TcpConnect(string ipv4, string port, bool asyn, int& sock, int timeout);

	int				_Start();
	void			_ResolveThreadProc();
	//���̴߳���
	bool			_AddSocketData2Queue(CSocketData socketData);
	bool			_GetSocketDataFromQueue(CSocketData& socketData, bool sequence);
	//���̣߳���֤���ݰ�����˳����
	void			_ResolveSequenceThreadProc();
	void			_AliveCheckThreadProc();
	virtual int		_AddConnSocket(StSocket& stSock);
	int				_GetConnSocket(string key, StSocket& stSock);
	map<string, StSocket>	GetDevMap();
	int				_AddConnectingSocket(StSocket& stSock);
    void			_AddConnectedEvent(StSocket& stSock, eCONN_MESS_Type messType);
	int				_RemoveConnectingSocket(string key, bool closeSocket);

	//����˼����߳�
	void			_BroadcastListenerProc();

	void			_TcpConnectingProc();

	//����socket���ӵ����һ��ͨ��ʱ��
	void			_UpdateAliveTimeStamp(string strDeviceID);

	//����Э�����socket���ݡ������ݼ�����У�Ҳ���Ը��ݲ�ͬ��Э����ò�ͬ�����ݽ��սӿ�
	//int			_SocketRecv(StSocket stSock);
	int				_DispatchRecvData(int nRecvLen, unsigned char *recvData, const StSocket& stSock);
	int				_IncreaseResolveThread();
	int				_DecreaseResolveThread();

	bool			_InvalidDevice(string ipv4, string port);

	void			_AddSocketData2PreProcessQueue(CSocketData& socketData);
	void			_PreProcessThreadProc();
private:
	pollfd						*m_pollArray;
	static int					m_pollFdMax;
	map<string, StSocket>		m_mapDev;					//��Ŀ���豸devid���뱾��socket������ݵ�key-value

	volatile bool				m_devUpdate;
	volatile bool				m_bRun;
	XCondWait					m_resolveWait;				//���ݴ����߳�ͬ���ź���
	XCondWait					m_preProcessWait;			//����Ԥ�����߳�ͬ���ź���
	int							m_nResolveThreadsCounts;	//���ݴ����߳�����
	vector< std::thread *>		m_resolveThreads;			//���ݴ����߳�
	CQueueManager<CSocketData>	m_vSocketDataRecvSequence;	//��Ҫ��֤˳��������ݶ���
	CQueueManager<CSocketData>	m_vSocketDataRecv;			//��ȡ�������ݶ���
	map<string, CSocketData>	m_mapSocketDataPreProcess;	//��ҪԤ���������
	
															//���ݴ���ص�
	CSocketMessage*				m_pSockMessage;
	//const static int m_nAliveTime = 300;//*����SOCKET���ݣ���ʾ����

	//����˼�����
	map<int, StSocket>			m_mapListener;
	map<string, StSocket>		m_mapDevConnecting;
};

bool SocketServerListener(int tcpServerPort, eSocketType socketType, eProtocolType protocolType, bool dataProcessInOrder = false,
							long long second2Close = KEEP_ALIVE_TIMEOUT_SECOND_DEFAULT);
