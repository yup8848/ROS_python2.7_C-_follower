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

#define SOCKET_FRAME_SIZE 1024*100	//单包最大100K
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
	//int nType;//雷达/防区/。。。。
	long long llAliveTimeStamp;//最近一次收到数据包的时间戳
	long long llTimeout2Close;
	bool dataProcessInOrder;
	bool tcpFrameNeedPreProcess;

	//用于SDK的通信句柄，外部调用

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
	// 禁止复制构造函数
	CSocket(const CSocket&) = delete;
	// 禁止对象赋值操作符
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

	//开始和停止服务
	int				Start();
	void			Stop();
	//启动和停止指定端口监听
    int				AddListener(int port, eSocketType socketType, eProtocolType protocolType = PROTOCOL_NO_CASE,
							long long second2Close = KEEP_ALIVE_TIMEOUT_SECOND_DEFAULT, bool dataProcessInOrder = false);
	int				RemoveListener(int port);
	//默认TCP使用异步连接方式
    int				Connect(string ipv4, int port, string deviceId, eSocketType socketType, eProtocolType protocolType = PROTOCOL_NO_CASE,
							long long second2Close = KEEP_ALIVE_TIMEOUT_SECOND_DEFAULT, bool asyn2Tcp = true, bool dataProcessInOrder = false);
    int				Connect(string ipv4, string port, string deviceId, eSocketType socketType, eProtocolType protocolType = PROTOCOL_NO_CASE,
							long long second2Close = KEEP_ALIVE_TIMEOUT_SECOND_DEFAULT, bool asyn2Tcp = true, bool dataProcessInOrder = false);
	//向指定链接中写入报文
	int				WriteFrame(string ipv4, string port, const char* pData, int nLen);

	//TCP/UDP直接连接-写-读socket，短连接
    int				WriteRead(string ipv4, string port, eSocketType socketType, const char* sendBuf, int sendLen, char* recvBuf, int& recvLenInOut, int timeout = 500);

	void			DisConnect(string ipv4, string port);
	bool			IsConnected(string ipv4, string port);//是否已经链接成功
	bool			IsConnecting(string ipv4, string port);//是否正在连接，针对TCP异步连接方式
	void			_CloseSocket(int sock);
	void			RegMessageHandler(CSocketMessage* sockMessage);


protected:
	int				_SocketRecvFrom(struct sockaddr_in& server, int& sock, char* recvBuf, int& recvLenInOut, int timeout);
	//TCP连接，支持异步连接和同步连接（同步连接通过异步方式（select检查）支持自定义超时）
	bool			_TcpConnect(string ipv4, string port, bool asyn, int& sock, int timeout);

	int				_Start();
	void			_ResolveThreadProc();
	//单线程处理
	bool			_AddSocketData2Queue(CSocketData socketData);
	bool			_GetSocketDataFromQueue(CSocketData& socketData, bool sequence);
	//单线程，保证数据按接收顺序处理
	void			_ResolveSequenceThreadProc();
	void			_AliveCheckThreadProc();
	virtual int		_AddConnSocket(StSocket& stSock);
	int				_GetConnSocket(string key, StSocket& stSock);
	map<string, StSocket>	GetDevMap();
	int				_AddConnectingSocket(StSocket& stSock);
    void			_AddConnectedEvent(StSocket& stSock, eCONN_MESS_Type messType);
	int				_RemoveConnectingSocket(string key, bool closeSocket);

	//服务端监听线程
	void			_BroadcastListenerProc();

	void			_TcpConnectingProc();

	//更新socket连接的最近一次通信时间
	void			_UpdateAliveTimeStamp(string strDeviceID);

	//根据协议接收socket数据、将数据加入队列，也可以根据不同的协议调用不同的数据接收接口
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
	map<string, StSocket>		m_mapDev;					//（目标设备devid）与本地socket相关数据的key-value

	volatile bool				m_devUpdate;
	volatile bool				m_bRun;
	XCondWait					m_resolveWait;				//数据处理线程同步信号量
	XCondWait					m_preProcessWait;			//数据预处理线程同步信号量
	int							m_nResolveThreadsCounts;	//数据处理线程数量
	vector< std::thread *>		m_resolveThreads;			//数据处理线程
	CQueueManager<CSocketData>	m_vSocketDataRecvSequence;	//需要保证顺序处理的数据队列
	CQueueManager<CSocketData>	m_vSocketDataRecv;			//读取到的数据队列
	map<string, CSocketData>	m_mapSocketDataPreProcess;	//需要预处理的数据
	
															//数据处理回调
	CSocketMessage*				m_pSockMessage;
	//const static int m_nAliveTime = 300;//*秒无SOCKET数据，表示掉线

	//服务端监听器
	map<int, StSocket>			m_mapListener;
	map<string, StSocket>		m_mapDevConnecting;
};

bool SocketServerListener(int tcpServerPort, eSocketType socketType, eProtocolType protocolType, bool dataProcessInOrder = false,
							long long second2Close = KEEP_ALIVE_TIMEOUT_SECOND_DEFAULT);
