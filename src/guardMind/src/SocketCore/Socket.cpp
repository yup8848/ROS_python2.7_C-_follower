#include <iostream>
#include <chrono>
#include "Socket.h"
#include "../Common/ErrorCodeDef.h"
#include "../Common/CommonDef.h"
#include "../Common/Log.h"
#include "SocketMessage.h"
#include "../Common/Common.h"
#include "../Common/RWLock.h"
#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wsock32.lib")
#else
#include <error.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif


mutex m_muPreProcess;
static CRWLock m_lockRW;//设备连接列表读写锁(写优先)
static CRWLock m_lockConnectingRW;//设备连接列表读写锁(写优先)


int CSocket::m_pollFdMax = 5000;
CSocket::CSocket():m_vSocketDataRecv(SOCKET_QUENE_MAX)
,m_vSocketDataRecvSequence(SOCKET_QUENE_MAX)
{
	m_pSockMessage = NULL;
	m_bRun = false;
	m_resolveWait.Reset();
	m_mapDev.clear();
	m_pollArray = new pollfd[m_pollFdMax];
}

CSocket::~CSocket()
{
	Stop();
#ifdef _WIN32
	WSACleanup();
#endif
}


void CSocket::RegMessageHandler(CSocketMessage* sockMessage)
{
	m_pSockMessage = sockMessage;
}
int CSocket::WriteFrame(string ipv4, string port, const char* pData, int nLen)
{
	if (_InvalidDevice(ipv4, port))
		return err_Success;
	string key;
	int nRet = GetMapKey(ipv4, port, key);
	if (err_Success != nRet)
		return nRet;

	StSocket stSock;
	if (err_Success == _GetConnSocket(key, stSock))
	{
		string strData = ByteStream2HexString((char*)pData, nLen);
		LOG_DEBUG("write data to device %s,length=%d%s%s", ipv4.c_str(), nLen, GetNewLine(), strData.c_str());
		//if (send(stSock.sock, pData, nLen, 0) == -1)
		if (sendto(stSock.sock, pData, nLen, 0, (struct sockaddr *) &stSock.sockDest, sizeof(stSock.sockDest)) == -1)
		{
			int code = 0;
			code = GET_LAST_ERROR;
			LOG_ERROR("socket sendto error,sock = %d, code=%d, msg = %s,ipv4=%s", stSock.sock, code, strerror(code), ipv4.c_str());
			return err_SocketWrite;
		}
	}
	else
	{
		return err_Connect;
		LOG_ERROR("not find dev, please connected first");
	}
	return nRet;
}

int	CSocket::WriteRead(string ipv4, string port, eSocketType socketType, const char* sendBuf, int sendLen, char* recvBuf, int& recvLenInOut, int timeout)
{
	if (_InvalidDevice(ipv4, port))
		return err_Success;
	int ret = err_Success;
	int sock = -1;
	struct sockaddr_in server;
	server.sin_addr.s_addr = inet_addr(ipv4.c_str());
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(port.c_str()));

	if (SOCKET_TCP == socketType)
	{
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (!_TcpConnect(ipv4, port, false, sock, timeout))
		{
			ret = err_Connect;
		}
	}
	else if (SOCKET_UDP == socketType)
	{
		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		struct sockaddr_in local;

		local.sin_addr.s_addr = htonl(INADDR_ANY);
		local.sin_family = AF_INET;
		local.sin_port = 0;

		if (::bind(sock, (sockaddr *)&local, sizeof(sockaddr)))
		{
			int code = 0;
			code = GET_LAST_ERROR;
			LOG_ERROR("socket bind error,sock = %d, code=%d,msg=%s,ipv4=%s", sock, code, strerror(code), ipv4.c_str());
			ret = err_Connect;
		}
	}
	else
	{
		LOG_ERROR("only support tcp/udp,ipv4=%s", ipv4.c_str());
		ret = err_Connect;
	}
	if (err_Success == ret)
	{
		if (sendto(sock, sendBuf, sendLen, 0, (struct sockaddr *) &server, sizeof(server)) == -1)
		{
			int code = 0;
			code = GET_LAST_ERROR;
			LOG_ERROR("socket sendto error code=%d, msg = %s,ipv4=%s", code, strerror(code), ipv4.c_str());
		}
		else
		{
			string strData = ByteStream2HexString((char*)sendBuf, sendLen);
			LOG_DEBUG("write data to device %s,length=%d%s%s", ipv4.c_str(), sendLen, GetNewLine(), strData.c_str());
			ret = _SocketRecvFrom(server, sock, recvBuf, recvLenInOut, timeout);
		}
	}

	if (err_Success != ret)
	{
		recvLenInOut = 0;
	}
	LOG_DEBUG("socket disconnected,ipv4=%s", ipv4.c_str());
	_CloseSocket(sock);
	return ret;
}
int CSocket::_SocketRecvFrom(struct sockaddr_in& server, int& sock, char* recvBuf, int& recvLenInOut, int timeout)
{
	shared_ptr<unsigned char> spRecvBuf(new unsigned char[SOCKET_RECV_BUF_SIZE], std::default_delete<unsigned char[]>());
	unsigned char* pData = spRecvBuf.get();
	memset(pData, 0, SOCKET_RECV_BUF_SIZE);
	//sockaddr_in addrSever = { 0 };
	socklen_t lenSockIn = sizeof(server);

#ifdef _WIN32
	unsigned long mode = 1;
	ioctlsocket(sock, FIONBIO, &mode);
#else
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);//设置为非阻塞模式
#endif
	long long milliSecondsStart = GetMilliSecondsSincePowerOn();
	long long milliSecondsEnd = GetMilliSecondsSincePowerOn(); 
	int _recvLen = -1;
	int ret = err_SocketRead;
	while ((milliSecondsEnd - milliSecondsStart) < timeout)
	{
		fd_set fdsErr, fdsRead;
		timeval timeout = { 1,0 }; //select等待1秒，1秒轮询，要非阻塞就置0 
		FD_ZERO(&fdsErr);
		FD_ZERO(&fdsRead);
		int nMaxFd = sock;
		FD_SET(sock, &fdsErr);
		FD_SET(sock, &fdsRead);

		int nSelRet = 0;
#ifdef _WIN32
		//select函数的第一个参数，在windows下可以忽略，但在linux下必须设为最大文件描述符加1；
		nSelRet = select(0, &fdsRead, NULL, &fdsErr, &timeout);
#else
		nSelRet = select(nMaxFd + 1, &fdsRead, NULL, &fdsErr, &timeout);
#endif		
		if (-1 == nSelRet)
		{
			this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		else if (0 == nSelRet)
		{
			this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		else
		{
			if (FD_ISSET(sock, &fdsRead)) //测试sock是否连接成功&失败 
			{
				_recvLen = recvfrom(sock, (char*)pData, SOCKET_RECV_BUF_SIZE, 0, (sockaddr *)&server, &lenSockIn);
				if (_recvLen>0)
				{
					string ipv4 = inet_ntoa(server.sin_addr);
					string strData = ByteStream2HexString((char*)pData, _recvLen);
					LOG_DEBUG("read data from %s, length=%d%s%s", ipv4.c_str(), _recvLen, GetNewLine(), strData.c_str());
					memcpy(recvBuf, pData, recvLenInOut);
					ret = err_Success;
				}
				recvLenInOut = _recvLen;
				break;
			}
			if (FD_ISSET(sock, &fdsErr))
			{
				break;
			}
		}
		milliSecondsEnd = GetMilliSecondsSincePowerOn();
	}

#ifdef _WIN32
	mode = 0;
	ioctlsocket(sock, FIONBIO, &mode);
#else
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);//设置为阻塞模式
#endif
	return ret;
}
void CSocket::_CloseSocket(int sock)
{
#ifdef _WIN32
	shutdown(sock, SD_BOTH);
	closesocket(sock);
#else
	close(sock);
#endif
	sock = -1;

}

bool CSocket::_TcpConnect(string ipv4, string port, bool asyn, int& sock, int timeout)
{
	bool ret = false;
	struct sockaddr_in server;
	server.sin_addr.s_addr = inet_addr(ipv4.c_str());
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(port.c_str()));
	
#ifdef _WIN32
	unsigned long mode = 1;
	ioctlsocket(sock, FIONBIO, &mode);
#else
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);//设置为非阻塞模式
#endif
	int r = connect(sock, (struct sockaddr*)&server, sizeof(server));
	long long milliSecondsStart = GetMilliSecondsSincePowerOn();
	long long milliSecondsEnd = GetMilliSecondsSincePowerOn();

	if (asyn)
	{
		ret = true;
	}
	else
	{
		while ((milliSecondsEnd - milliSecondsStart) < timeout)
		{
			fd_set fdsWrite, fdsErr, fdsRead;
			timeval timeout = { 1,0 }; //select等待1秒，1秒轮询，要非阻塞就置0 
			FD_ZERO(&fdsWrite); //每次循环都要清空集合，否则不能检测描述符变化 
			FD_ZERO(&fdsErr);
			FD_ZERO(&fdsRead);
			int nMaxFd = sock;
			FD_SET(sock, &fdsWrite);
			FD_SET(sock, &fdsErr);
			FD_SET(sock, &fdsRead);

			int nSelRet = 0;
#ifdef _WIN32
			//select函数的第一个参数，在windows下可以忽略，但在linux下必须设为最大文件描述符加1；
			nSelRet = select(0, &fdsRead, &fdsWrite, &fdsErr, &timeout);
#else
			nSelRet = select(nMaxFd + 1, &fdsRead, &fdsWrite, &fdsErr, &timeout);
#endif		
			if (-1 == nSelRet)
			{
				this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			else if (0 == nSelRet)
			{
				this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			else
			{
				if (FD_ISSET(sock, &fdsWrite)) //测试sock是否连接成功&失败 
				{
					int status = 0;
					int slen = 4;
					int r = getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&status, (socklen_t *)&slen);
					int code = GET_LAST_ERROR;
					if (-1 != r && 0 == status)
					{
						LOG_DEBUG("%s:%s connect success(tcp)", ipv4.c_str(), port.c_str());
						ret = true;
					}
					else
					{
						LOG_ERROR("%s:%s connect failed(tcp),getsockopt retcode=%d, output socket status=%d,lastErrorCode=%d", ipv4.c_str(), port.c_str(),
							r, status, code);
					}
					break;
				}
				if (FD_ISSET(sock, &fdsErr))
				{
					LOG_ERROR("%s:%s connect failed(tcp), close socket", ipv4.c_str(), port.c_str());
					break;
				}
			}
			milliSecondsEnd = GetMilliSecondsSincePowerOn();
		}
	}

#ifdef _WIN32
	mode = 0;
	ioctlsocket(sock, FIONBIO, &mode);
#else
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);//设置为阻塞模式
#endif
	if (!ret)
	{
		LOG_ERROR("socket disconnected,ipv4=%s", ipv4.c_str());
		_CloseSocket(sock);
	}
	return ret;
}
bool CSocket::_InvalidDevice(string ipv4, string port)
{
	if ((string::npos != ipv4.find("1.1.1")) || string::npos != ipv4.find("127.0.0"))
	{
		return true;
	}
	return false;
}
int CSocket::Connect(string ipv4, int port, string deviceId, eSocketType socketType, eProtocolType protocolType, long long second2Close,
	bool asyn2Tcp, bool dataProcessInOrder)
{
	if (_InvalidDevice(ipv4, to_string(port)))
		return err_Success;
	return Connect(ipv4, to_string(port), deviceId, socketType, protocolType, second2Close, asyn2Tcp, dataProcessInOrder);
}
int CSocket::Connect(string ipv4, string port, string deviceId, eSocketType socketType, eProtocolType protocolType, long long second2Close,
	bool asyn2Tcp, bool dataProcessInOrder)
{
	if (_InvalidDevice(ipv4, port))
		return err_Success;
	if (IsConnecting(ipv4, port))
		return err_Success;
	int ret = err_Success;
	int sock = -1;
	struct sockaddr_in dest;
	dest.sin_addr.s_addr = inet_addr(ipv4.c_str());
	dest.sin_family = AF_INET;
	dest.sin_port = htons(atoi(port.c_str()));

	if (SOCKET_TCP == socketType)
	{
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	else if (SOCKET_UDP == socketType)
	{
		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (string::npos != ipv4.find("255"))//广播连接需要设置套接字
		{
			const char on = 1;
			second2Close = KEEP_ALIVE_TIMEOUT_SECOND_NO_CASE;//广播一直开着，直到用户手动停止
			setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)); //设置套接字选项
		}
	}
	else
	{
		LOG_ERROR("only support tcp/udp,ipv4=%s", ipv4.c_str());
		ret = err_Connect;
	}

	StSocket stSock;
	stSock.sock = sock;
	stSock.sockDest = dest;
	stSock.ipv4 = ipv4;
	stSock.port = port;
	stSock.sockType = socketType;
	stSock.deviceId = deviceId;
	stSock.protocolType = protocolType;
	stSock.llTimeout2Close = second2Close;
	stSock.dataProcessInOrder = dataProcessInOrder;
	stSock.llAliveTimeStamp = GetSecondsSincePowerOn();

	if (SOCKET_TCP == socketType)
	{
		if (_TcpConnect(ipv4, port, asyn2Tcp, sock, TCP_CONNECT_TIMEOUT_SECOND*1000))
		{
			if (asyn2Tcp)
			{
				_AddConnectingSocket(stSock);
			}
			else
			{
				_AddConnSocket(stSock);
			}
		}
	}
	else if (SOCKET_UDP == socketType)
	{
		struct sockaddr_in local;

		local.sin_addr.s_addr = htonl(INADDR_ANY);
		//local.sin_addr.s_addr = inet_addr("127.0.0.1");
		local.sin_family = AF_INET;
		local.sin_port = 0;

		if (::bind(sock, (sockaddr *)&local, sizeof(sockaddr)))
		{
			int code = 0;
			code = GET_LAST_ERROR;
			LOG_ERROR("socket bind error,sock = %d, code=%d,msg=%s,ipv4=%s", sock, code, strerror(code), ipv4.c_str());
			_CloseSocket(sock);
			return err_Connect;
		}
		_AddConnSocket(stSock);
	}
	else
	{
		LOG_ERROR("only support tcp/udp,ipv4=%s", ipv4.c_str());
		_CloseSocket(sock);
		return err_Connect;
	}
    usleep(500000); //Added by Tai. Solve Bug "first time cannot send"
	return err_Success;
}
int CSocket::_AddConnSocket(StSocket& stSock)
{
	string key;
	int ret = GetMapKey(stSock.ipv4, stSock.port, key);
	map<string, StSocket>::iterator it;
	{

		CMyWLockManager writeLock(m_lockRW);
		it = m_mapDev.find(key.c_str());
		if (it != m_mapDev.end())
		{
			_CloseSocket(it->second.sock);
			it->second = stSock;
		}
		else
		{
			m_mapDev.insert(make_pair(key, stSock));
		}
	}
	_AddConnectedEvent(stSock, MESS_CONNECTED);
	return ret;
}
int CSocket::_GetConnSocket(string key, StSocket& stSock)
{
	CMyRLockManager readLock(m_lockRW);
	auto it = m_mapDev.find(key.c_str());
	if (it != m_mapDev.end())
	{
		stSock = it->second;
	}
	else
	{
		return err_NotConnect;
	}
	return err_Success;
}
map<string, StSocket> CSocket::GetDevMap()
{
	CMyRLockManager readLock(m_lockRW);
	return m_mapDev;
}

void CSocket::_AddConnectedEvent(StSocket& stSock, eCONN_MESS_Type messType)
{
	//添加连接成功消息到消息队列，通知调用者
	CSocketData socketData;
	socketData.SetMessasgeType(messType);
	socketData.SetDeviceId(stSock.deviceId);
	socketData.SetIpv4(stSock.ipv4);
	socketData.SetSocketPort(stSock.port);
	socketData.SetProtocolType(stSock.protocolType);
	socketData.SetSocketType(stSock.sockType);
	socketData.SetProcessInOrder(true);
	_AddSocketData2Queue(socketData);
	LOG_DEBUG("%s:%s connect success", socketData.GetIpv4().c_str(), socketData.GetPort().c_str());
}
int CSocket::_AddConnectingSocket(StSocket& stSock)
{
	string key;

	struct sockaddr_in server;
	server.sin_addr.s_addr = inet_addr(stSock.ipv4.c_str());
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(stSock.port.c_str()));

	int r = connect(stSock.sock, (struct sockaddr*)&server, sizeof(server));
	int ret = GetMapKey(stSock.ipv4, stSock.port, key);
	map<string, StSocket>::iterator it;
	{
		CMyWLockManager writeLock(m_lockConnectingRW);
		it = m_mapDevConnecting.find(key.c_str());
		if (it == m_mapDevConnecting.end())
		{
			m_mapDevConnecting.insert(make_pair(key, stSock));
		}
		else//已经正在连接了，关闭当前连接请求
		{
			_CloseSocket(stSock.sock);
		}
	}
	return ret;
}
int CSocket::GetMapKey(string ipv4, string port, string& key)
{
	int ret = err_Success;

	if (ipv4.length()>7 && port.length()>1)
	{
		key = ipv4 + port;
	}
	else
	{
		ret = err_ParamInvalid;
	}
	return ret;
}
void CSocket::_UpdateAliveTimeStamp(string key)
{
	long long llStart = GetMilliSecondsSincePowerOn();
	CMyWLockManager writeLock(m_lockRW);

	map<string, StSocket>::iterator it;
	it = m_mapDev.find(key.c_str());
	if (it != m_mapDev.end())
	{
		it->second.llAliveTimeStamp = GetSecondsSincePowerOn();
	}
}

void CSocket::DisConnect(string ipv4, string port)
{
	string key;
	int ret = GetMapKey(ipv4, port, key);
	if (err_Success != ret)
	{
		LOG_ERROR("invalid key for %s:%s", ipv4.c_str(), port.c_str());
		return;
	}
	//LOG_DEBUG("try to enter CMyRLockManager writeLock");
	CMyWLockManager writeLock(m_lockRW);

	map<string, StSocket>::iterator it;
	it = m_mapDev.find(key.c_str());
	if (it != m_mapDev.end())
	{
		LOG_DEBUG("socket disconnected,ipv4=%s,port=%s，timestamp=%s", ipv4.c_str(), port.c_str(),GetTimeStamp().c_str());
		_CloseSocket(it->second.sock);
		m_mapDev.erase(key);
	}
}
void CSocket::Stop()
{
	LOG_DEBUG("stoped");
	CMyWLockManager writeLock(m_lockRW);
	m_bRun = false;
	m_resolveWait.SetQuitSignalAll();

	map<string, StSocket>::iterator it;
	it = m_mapDev.begin();
	while (it != m_mapDev.end())
	{
		_CloseSocket(it->second.sock);
		it++;
	}
	m_mapDev.clear();

	for (size_t i = 0; i < m_resolveThreads.size(); i++)
	{
		if (m_resolveThreads[i] && m_resolveThreads[i]->joinable())
		{
			m_resolveThreads[i]->join();
		}
		delete m_resolveThreads[i];
	}
	m_resolveThreads.clear();
	if (m_pollArray)
	{
		delete[]m_pollArray;
	}
	m_pollArray = NULL;
}

bool CSocket::IsConnected(string ipv4, string port)
{
	if (_InvalidDevice(ipv4, port))
		return true;
	string key;
	int ret = GetMapKey(ipv4, port, key);
	if (err_Success != ret)
	{
		return false;
	}
	auto mapDev = GetDevMap();
	auto it = mapDev.find(key.c_str());
	if (it != mapDev.end())
	{
		long long llCurTime = GetSecondsSincePowerOn();
		if ((llCurTime - it->second.llAliveTimeStamp) > it->second.llTimeout2Close)
		{
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool CSocket::IsConnecting(string ipv4, string port)
{
	if (_InvalidDevice(ipv4, port))
		return true;
	string key;
	int ret = GetMapKey(ipv4, port, key);
	if (err_Success != ret)
	{
		return false;
	}

	CMyRLockManager readLock(m_lockConnectingRW);
	map<string, StSocket>::iterator it;
	it = m_mapDevConnecting.find(key.c_str());
	if (it != m_mapDevConnecting.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

//数据包预处理，主要处理TCP通信数据包大于MTU被分包需要重新组包问题
void CSocket::_PreProcessThreadProc()
{
	while (m_bRun)
	{
		if (m_preProcessWait.Wait() == WaitResult_Quit)
		{
			LOG_ERROR("receive notify resolve thread exit");
			break;
		}

		std::lock_guard<std::mutex> guard(m_muPreProcess);
		auto it = m_mapSocketDataPreProcess.begin();
		while (it != m_mapSocketDataPreProcess.end())
		{
			CSocketData _socketData = it->second;
			if (_socketData.GetMessageProtolType() == PROTOCOL_GUARD_MIND)
			{
				LOG_DEBUG("tcp frame length = %d", _socketData.GetRawDataLength());
			}
			it++;
		}
		m_preProcessWait.Reset();
	}
}
void CSocket::_AddSocketData2PreProcessQueue(CSocketData& socketData)
{
	if (socketData.GetMessageProtolType() == PROTOCOL_GUARD_MIND)
	{
		string key;
		int ret = GetMapKey(socketData.GetIpv4(), socketData.GetPort(), key);
		if (err_Success == ret)
		{
			std::lock_guard<std::mutex> guard(m_muPreProcess);
			auto it = m_mapSocketDataPreProcess.find(key);
			if (it != m_mapSocketDataPreProcess.end())
			{
				CSocketData _socketData = it->second + socketData;
				it->second = _socketData;
			}
			else
			{
				m_mapSocketDataPreProcess.insert(make_pair(key, socketData));
			}
			m_preProcessWait.SetSignal();
		}
		else
		{
			LOG_ERROR("invalid key for %s:%s", socketData.GetIpv4().c_str(), socketData.GetPort().c_str());
		}
	}
	else
	{
		_AddSocketData2Queue(socketData);
	}
}
bool CSocket::_AddSocketData2Queue(CSocketData socketData)
{
	if (socketData.GetProcessInOrder())
	{
		m_vSocketDataRecvSequence.EnQueue(socketData);
	}
	else
	{
		m_vSocketDataRecv.EnQueue(socketData);
		m_resolveWait.SetSignal();
	}

	return true;
}

bool CSocket::_GetSocketDataFromQueue(CSocketData& socketData, bool sequence)
{
	if (sequence)
	{
		return m_vSocketDataRecvSequence.DeQueue(socketData);
	}
	else
	{
		return m_vSocketDataRecv.DeQueue(socketData);
	}

	return false;
}
void CSocket::_AliveCheckThreadProc()
{
	const static int sleep_milliseconds = 2000;
	//int sleep_count = m_nAliveTime * 1000 / 50;
	while (m_bRun)
	{
		this_thread::sleep_for(std::chrono::milliseconds(sleep_milliseconds));
		{
			//LOG_DEBUG("before GetDevMap");
			auto mapDev = GetDevMap();
			//LOG_DEBUG("after GetDevMap");
			map<string, StSocket>::iterator it = mapDev.begin();
			while (it != mapDev.end())
			{
				long long llCurTime = GetSecondsSincePowerOn();
				if ((KEEP_ALIVE_TIMEOUT_SECOND_NO_CASE != it->second.llTimeout2Close))
				{
					if ((llCurTime - it->second.llAliveTimeStamp) > it->second.llTimeout2Close)
					{
						it->second.llAliveTimeStamp = llCurTime;
						CSocketData socketData;
						socketData.SetMessasgeType(MESS_NO_ALIVE);
						socketData.SetDeviceId(it->second.deviceId);
						socketData.SetIpv4(it->second.ipv4);
						socketData.SetSocketPort(it->second.port);
						socketData.SetProtocolType(it->second.protocolType);
						socketData.SetSocketType(it->second.sockType);
						socketData.SetProcessInOrder(true);
						_AddSocketData2Queue(socketData);
						LOG_DEBUG("device %s:%s not alive,add message to queue,timestamp=%s", it->second.ipv4.c_str(), it->second.port.c_str(),GetTimeStamp().c_str());
					}
				}
				else
					it->second.llAliveTimeStamp = llCurTime;
				//do heartbeat for sdk
				it++;
			}
			LOG_DEBUG("device count = %d", mapDev.size());
		}
	}
	LOG_DEBUG("_AliveCheckThreadProc OVER");
}

#ifdef THREAD_PRI
unsigned int CSocket::_ResolveThreadProc(void* param)
{
	CSocket* pObj = (CSocket*)param;
	while (pObj->m_bRun)
	{
		if (pObj->m_resolveWait.Wait() == WaitResult_Quit)
		{
			//LOG_DEBUG("receive notify resolve thread exit");
			break;
		}

		CSocketData socketData;
		if (!pObj->_GetSocketDataFromQueue(socketData, false))
		{
			pObj->m_resolveWait.Reset();
			//this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}

		//LOG_DEBUG("multi thread handler message,ipv4=%s", socketData.GetIpv4().c_str());

		//StSockDev sockDevice;
		//sockDevice.ipv4 = socketData.GetIpv4();
		//sockDevice.deviceId = socketData.GetDeviceId();
		//sockDevice.port = socketData.GetPort();
		//sockDevice.timeStamp = socketData.GetTimeStamp();
		//sockDevice.milliSecond = socketData.GetMilliSecond();
		//sockDevice.socketType = socketData.GetSocketType();
		if (m_pSockMessage)
		{
			pObj->m_pSockMessage->DispatchSocketMessage(socketData);
		}
	}
}
#else
void CSocket::_ResolveThreadProc()
{
	while (m_bRun)
	{
		if (m_resolveWait.Wait() == WaitResult_Quit)
		{
			LOG_ERROR("receive notify resolve thread exit");
			break;
		}
		CSocketData socketData;
		if (!_GetSocketDataFromQueue(socketData, false))
		{
			m_resolveWait.Reset();
			//this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}

		LOG_DEBUG("multi thread handler message,ipv4=%s", socketData.GetIpv4().c_str());

		//StSockDev sockDevice;
		//sockDevice.ipv4 = socketData.GetIpv4();
		//sockDevice.deviceId = socketData.GetDeviceId();
		//sockDevice.port = socketData.GetPort();
		//sockDevice.timeStamp = socketData.GetTimeStamp();
		//sockDevice.milliSecond = socketData.GetMilliSecond();
		//sockDevice.socketType = socketData.GetSocketType();
		if (m_pSockMessage)
		{
			m_pSockMessage->DispatchSocketMessage(socketData);
		}
	}
}
#endif
int CSocket::_DecreaseResolveThread()
{
	m_resolveWait.SetQuitSignalOne();
	return err_Success;
}
int CSocket::_IncreaseResolveThread()
{
	std::thread *thrResolve = new thread(&CSocket::_ResolveThreadProc, this);
	m_resolveThreads.push_back(thrResolve);
	m_nResolveThreadsCounts++;
	return err_Success;
}
void CSocket::_ResolveSequenceThreadProc()
{
	while (m_bRun)
	{
		CSocketData socketData;
		if (!_GetSocketDataFromQueue(socketData, true))
		{
			this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}

		LOG_DEBUG("single thread handler message,ipv4=%s", socketData.GetIpv4().c_str());

		//StSockDev sockDevice;
		//sockDevice.ipv4 = socketData.GetIpv4();
		//sockDevice.deviceId = socketData.GetDeviceId();
		//sockDevice.port = socketData.GetPort();
		//sockDevice.timeStamp = socketData.GetTimeStamp();
		//sockDevice.milliSecond = socketData.GetMilliSecond();
		//sockDevice.socketType = socketData.GetSocketType();
		if (m_pSockMessage)
		{
			m_pSockMessage->DispatchSocketMessage(socketData);
		}
	}
}
int CSocket::Start()
{
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 1), &wsaData))
	{
		LOG_ERROR("Winsock init error, code=%d", GET_LAST_ERROR);
		WSACleanup();
		return err_SocketInit;
	}
#endif
	m_nResolveThreadsCounts = 5;
	m_bRun = true;

	m_resolveWait.Reset();
	for (int i = 0; i < m_nResolveThreadsCounts; i++)
	{
#ifdef THREAD_PRI
		HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, &CSocket::_ResolveThreadProc, (void*)this, 0, NULL);
		BOOL b = SetThreadPriority(handle, THREAD_PRIORITY_ABOVE_NORMAL);
		if (b)
		{
			cout << "SetThreadPriority success" << endl;
		}
		else
			cout << "SetThreadPriority failed" << endl;
#else
		std::thread *thrResolve = new thread(&CSocket::_ResolveThreadProc, this);
		m_resolveThreads.push_back(thrResolve);
#endif
	}
	new thread(&CSocket::_ResolveSequenceThreadProc, this); 

	new thread(&CSocket::_Start, this);
	new thread(&CSocket::_AliveCheckThreadProc, this);
	
	new thread(&CSocket::_BroadcastListenerProc, this);

	new thread(&CSocket::_TcpConnectingProc, this);

	new thread(&CSocket::_PreProcessThreadProc, this);

	return 0;
}

int CSocket::_Start()
{
	fd_set fdsRead;
	fd_set fdsExcept;
	timeval timeout = { 1,0 }; //select等待1秒，1秒轮询，要非阻塞就置0 
	shared_ptr<unsigned char> spRecvBuf(new unsigned char[SOCKET_RECV_BUF_SIZE], std::default_delete<unsigned char[]>());

	int disCount = 0;
	while (m_bRun)
	{
		map<string, StSocket> mapDev = GetDevMap();
		FD_ZERO(&fdsRead); //每次循环都要清空集合，否则不能检测描述符变化 
		FD_ZERO(&fdsExcept);
		int nMaxFd = 0;
		//添加描述符 
		{
			if (mapDev.size() == 0)
			{
				this_thread::sleep_for(std::chrono::milliseconds(50));
				continue;
			}
			//LOG_DEBUG("mapDev size=%d", mapDev.size());
			map<string, StSocket>::iterator it;
			it = mapDev.begin();
			while (it != mapDev.end())
			{
				nMaxFd = nMaxFd > it->second.sock ? nMaxFd : it->second.sock;
				FD_SET(it->second.sock, &fdsRead);
				FD_SET(it->second.sock, &fdsExcept);
				it++;
			}
		}

		//select
		int nSelRet = 0;
		//TODO:需要改为Poll或者EPOLL/完成端口，windows下系统文件对select只支持64个连接
#ifdef _WIN32
		//select函数的第一个参数，在windows下可以忽略，但在linux下必须设为最大文件描述符加1；
		nSelRet = select(0, &fdsRead, NULL, &fdsExcept, &timeout);
		//Greater than zero	The time, in milliseconds, to wait.
		//Zero	Return immediately.
		//Less than zero	Wait indefinitely.
		//WSAPoll(m_pollArray, mapDev.size(), 10);
#else
		nSelRet = select(nMaxFd + 1, &fdsRead, NULL, &fdsExcept, &timeout);
#endif		

		//循环6000次(一分钟)写一次日志
		disCount++;
		if (disCount == 18000)
		{
			LOG_DEBUG("select return value=%d, nMaxFd=%d", nSelRet, nMaxFd);
			disCount = 0;
		}

		if (-1 == nSelRet)
		{
			this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		else if (0 == nSelRet)
		{
			this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		else
		{
			long long llStartTimeStamp = GetMilliSecondsSincePowerOn();
			{
				map<string, StSocket>::iterator itDev;
				itDev = mapDev.begin();
				//逐个匹配socket
				while (itDev != mapDev.end())
				{
					if (!m_bRun)
					{
						LOG_DEBUG("_Start thread end, m_bRun=%d", m_bRun);
						return err_Success;
					}
					if (FD_ISSET(itDev->second.sock, &fdsRead)) //测试sock是否可读，即是否网络上有数据 
					{
						unsigned char* pData = spRecvBuf.get();
						memset(pData, 0, SOCKET_RECV_BUF_SIZE);
						sockaddr_in addrSever = { 0 };
						socklen_t lenSockIn = sizeof(addrSever);
						//接收数据，一次数据接收，可能存在多个数据帧
						int nRecvLen = -1;
						if(SOCKET_TCP == itDev->second.sockType)
							nRecvLen = recv(itDev->second.sock, (char*)pData, SOCKET_RECV_BUF_SIZE, 0);
						else
							nRecvLen = recvfrom(itDev->second.sock, (char*)pData, SOCKET_RECV_BUF_SIZE, 0, (sockaddr *)&addrSever, &lenSockIn);
#if 0
						if (0 == nRecvLen)
						{
							int status = 0;
							int slen = 4;
							//同样将连接的socket加入读写错误三个事件中，调用select函数，如果socket连接失败那么socket会变的同时可读和可写
							int r = getsockopt(itDev->second.sock, SOL_SOCKET, SO_ERROR, (char*)&status, (socklen_t *)&slen);
							int code = GET_LAST_ERROR;
							if (-1 != r && 0 == status)//no error
							{
								LOG_DEBUG("socket recv length = 0,ipv4=%s, no error of getsockopt",	itDev->second.ipv4.c_str());
							}
							else
							{
								LOG_DEBUG("socket recv length = 0,ipv4=%s,getsockopt retcode=%d, output socket status=%d,lastErrorCode=%d",
									itDev->second.ipv4.c_str(), r, status, code);
								_DispatchRecvData(nRecvLen, pData, itDev->second);//some errors
							}
						}
						else
						{
							_DispatchRecvData(nRecvLen, pData, itDev->second);
						}
#else

						string ipv4 = inet_ntoa(addrSever.sin_addr);
						string port = to_string(ntohs(addrSever.sin_port));
						if (0 == nRecvLen)
						{
							int status = 0;
							int slen = 4;
							//同样将连接的socket加入读写错误三个事件中，调用select函数，如果socket连接失败那么socket会变的同时可读和可写
							int r = getsockopt(itDev->second.sock, SOL_SOCKET, SO_ERROR, (char*)&status, (socklen_t *)&slen);
							int code = GET_LAST_ERROR;
							LOG_DEBUG("socket recv length = 0,ipv4=%s,getsockopt retcode=%d, output socket status=%d,lastErrorCode=%d",
								ipv4.c_str(), r, status, code);
						}
						StSocket sock = itDev->second;
						sock.ipv4 = ipv4;
						sock.port = port;
						_DispatchRecvData(nRecvLen, pData, itDev->second);
#endif
					}

					//if (FD_ISSET(itDev->second.sock, &fdsExcept))
					//{
					//	int nRecvLen = -1;
					//	char buf[1024] = {0};
					//	nRecvLen = recv(itDev->second.sock, buf, sizeof(buf), MSG_OOB);
					//	if (nRecvLen == -1) 
					//		LOG_DEBUG("recv带外数据出错！");
					//	LOG_DEBUG("收到%d字节带外数据：%s\n", nRecvLen, buf);
					//}
					itDev++;
				}
			}
			long long llOverTimeStamp = GetMilliSecondsSincePowerOn();
			long long llTimeCost = llOverTimeStamp - llStartTimeStamp;
		}
	}//end while 
	return err_Success;
}

int CSocket::_DispatchRecvData(int nRecvLen, unsigned char *recvData, const StSocket& stSock)
{
	int ret = err_Success;
	if (nRecvLen <= 0)
	{
		LOG_ERROR("socket closed, recv length=%d %s:%s, code=%d,timestamp=%s", nRecvLen, stSock.ipv4.c_str(), stSock.port.c_str(), GET_LAST_ERROR, GetTimeStamp().c_str());
		if (IsConnected(stSock.ipv4, stSock.port))
		{
			DisConnect(stSock.ipv4, stSock.port);
			CSocketData socketData;
			socketData.SetMessasgeType(MESS_CLOSE);
			socketData.SetDeviceId(stSock.deviceId);
			socketData.SetIpv4(stSock.ipv4);
			socketData.SetSocketPort(stSock.port);
			socketData.SetProtocolType(stSock.protocolType);
			socketData.SetTimeStampe(GetTimeStamp());
			socketData.SetMilliSecond(GetMilliSeconds());
			socketData.SetSocketType(stSock.sockType);
			socketData.SetProcessInOrder(true);
			_AddSocketData2Queue(socketData);//连接、断开等时间按队列顺序处理
		}
		else
		{
			LOG_DEBUG("%s:%s not connected", stSock.ipv4.c_str(), stSock.port.c_str());
		}
		return err_Success;
	}
	else
	{
		string key;
		ret = GetMapKey(stSock.ipv4, stSock.port, key);
		_UpdateAliveTimeStamp(key);
		//格式化数据，然后输出到日志
		string strData = ByteStream2HexString((char*)recvData, nRecvLen);
		LOG_DEBUG("read data from device %s, length=%d%s%s", stSock.ipv4.c_str(), nRecvLen, GetNewLine(), strData.c_str());

		if (SOCKET_FRAME_SIZE >= nRecvLen)
		{
			CSocketData socketData(recvData, nRecvLen);
			socketData.SetMessasgeType(MESS_READ);
			socketData.SetDeviceId(stSock.deviceId);
			socketData.SetIpv4(stSock.ipv4);
			socketData.SetSocketPort(stSock.port);
			socketData.SetProtocolType(stSock.protocolType);
			socketData.SetTimeStampe(GetTimeStamp());
			socketData.SetMilliSecond(GetMilliSeconds());
			socketData.SetSocketType(stSock.sockType);
			socketData.SetProcessInOrder(stSock.dataProcessInOrder);
			//_AddSocketData2Queue(socketData);//数据按实际要求处理
			_AddSocketData2PreProcessQueue(socketData);
		}
		else
		{
			LOG_ERROR("data read from socket is too large,max data length is %d byte, the data length is %d byte", SOCKET_FRAME_SIZE, nRecvLen);
		}
	}
	return err_Success;
}

int CSocket::RemoveListener(int port)
{
	map<int, StSocket>::iterator it = m_mapListener.find(port);
	if (it != m_mapListener.end())
	{
		_CloseSocket(it->second.sock);
		m_mapListener.erase(it);
	}
	return err_Success;
}
int CSocket::AddListener(int port, eSocketType socketType, eProtocolType protocolType, long long second2Close, bool dataProcessInOrder)
{
	int sock = -1;
	int ret = err_Success;

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (SOCKET_TCP == socketType)
	{
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	//else if (SOCKET_UDP == socketType)
	//{
	//	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//}
	else
	{
		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	int reuse = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0)
	{
		LOG_ERROR("set socket reused falsed, port = %d", port);
	}
	if(::bind(sock, (struct sockaddr*)&local, sizeof(local)))
	{
		int code = 0;
		code = GET_LAST_ERROR;
		LOG_ERROR("socket bind error,sock = %d, code=%d,msg=%s,port=%d", sock, code, strerror(code), port);
		ret = err_SocketInit;
	}
	else
	{
		if (SOCKET_TCP == socketType)
		{
			if (listen(sock, 5))
			{
				int code = GET_LAST_ERROR;
				LOG_ERROR("socket listen error,sock = %d, code=%d,msg=%s,port=%d", sock, code, strerror(code), port);
				ret = err_SocketInit;
			}
		}
		if(ret == err_Success)
		{
			LOG_DEBUG("listening port %d, protocol=%d", port, protocolType);
			map<int, StSocket>::iterator it;
			{
				StSocket stSock;
				stSock.sock = sock;
				stSock.port = to_string(port);
				stSock.protocolType = protocolType;
				stSock.sockType = socketType;
				stSock.llTimeout2Close = second2Close;
				stSock.dataProcessInOrder = dataProcessInOrder;
				stSock.llAliveTimeStamp = GetSecondsSincePowerOn();

				//CMyWLockManager writeLock(m_lockRW);
				it = m_mapListener.find(port);
				if (it != m_mapListener.end())
				{
					_CloseSocket(it->second.sock);
					it->second = stSock;
				}
				else
				{
					m_mapListener.insert(make_pair(port, stSock));
				}
			}
		}
	}	
	if (err_Success != ret)
	{
		_CloseSocket(sock);
		return ret;
	}

	return ret;
}
void CSocket::_BroadcastListenerProc()
{
	fd_set fds;
	timeval timeout = { 1,0 }; //select等待1秒，1秒轮询，要非阻塞就置0 
	shared_ptr<unsigned char> spRecvBuf(new unsigned char[SOCKET_RECV_BUF_SIZE], std::default_delete<unsigned char[]>());
	while (m_bRun)
	{
		if (m_mapListener.size() == 0)
		{
			this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		FD_ZERO(&fds); //每次循环都要清空集合，否则不能检测描述符变化 
		int nMaxFd = 0;
		//添加描述符 
		{
			map<int, StSocket>::iterator it;
			it = m_mapListener.begin();
			while (it != m_mapListener.end())
			{
				nMaxFd = nMaxFd > it->second.sock ? nMaxFd : it->second.sock;
				FD_SET(it->second.sock, &fds);
				it++;
			}
		}
		int nSelRet = 0;
#ifdef _WIN32
		//select函数的第一个参数，在windows下可以忽略，但在linux下必须设为最大文件描述符加1；
		nSelRet = select(0, &fds, NULL, NULL, &timeout);
#else
		nSelRet = select(nMaxFd + 1, &fds, NULL, NULL, &timeout);
#endif		
		if (-1 == nSelRet)
		{
			this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		else if (0 == nSelRet)
		{
			this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		else
		{
			map<int, StSocket>::iterator it;
			it = m_mapListener.begin();
			//逐个匹配socket
			while (it != m_mapListener.end())
			{
				if (!m_bRun)
					return ;

				if (FD_ISSET(it->second.sock, &fds)) 
				{
					ST_SOCKET stSock;
					int tmpLength = sizeof(stSock.sockDest);
					if (SOCKET_TCP == it->second.sockType)
					{
						int clientSocket = accept(it->second.sock, (sockaddr*)&stSock.sockDest, (socklen_t*)&tmpLength);
						if (-1 != clientSocket)
						{
							//int port = ntohs(it->second.sockDest.sin_port);
							//int retval = getnameinfo(
							//	(SOCKADDR *)&it->second.sockDest,
							//	tmpLength,
							//	hoststr,
							//	NI_MAXHOST,
							//	NULL,
							//	0,
							//	NI_NUMERICHOST //| NI_NUMERICSERV
							//);

							//新的连接加入到接收队列
							string localPort = to_string(htons(it->first));
							stSock.sock = clientSocket;
							stSock.ipv4 = inet_ntoa(stSock.sockDest.sin_addr);
							stSock.port = to_string(ntohs(stSock.sockDest.sin_port));
							stSock.protocolType = it->second.protocolType;
							stSock.sockType = it->second.sockType;
							stSock.dataProcessInOrder = it->second.dataProcessInOrder;
							stSock.llTimeout2Close = it->second.llTimeout2Close;
							stSock.llAliveTimeStamp = GetSecondsSincePowerOn();
							_AddConnSocket(stSock);
							LOG_DEBUG("client connected,ipv4=%s, port=%s, protocol=%d", stSock.ipv4.c_str(), stSock.port.c_str(), stSock.protocolType);
							if (m_pSockMessage)
							{
								m_pSockMessage->ClientConnected(it->second.port, it->second.sockType, it->second.protocolType, stSock.ipv4, stSock.port);
							}
						}
						else
						{
							int code = GET_LAST_ERROR;
							LOG_ERROR("socket accept error,code=%d,msg=%s", code, strerror(code));
						}
					}
					else if (SOCKET_UDP == it->second.sockType)
					{
						unsigned char* pData = spRecvBuf.get();
						memset(pData, 0, SOCKET_RECV_BUF_SIZE);
						int nRecvLen = -1;
						char data[10] = { 0 };
						sockaddr_in addrSever = { 0 };
						socklen_t lenSockIn = sizeof(addrSever);
						nRecvLen = recvfrom(it->second.sock, (char*)pData, SOCKET_RECV_BUF_SIZE, 0, (sockaddr *)&addrSever, &lenSockIn);
						string ipv4 = inet_ntoa(addrSever.sin_addr);
						string port = to_string(ntohs(addrSever.sin_port));
						LOG_DEBUG("udp message from %s:%s to %d", ipv4.c_str(), port.c_str(), it->first);
						string key;
						if (err_Success == GetMapKey(ipv4, port, key))
						{
							stSock.ipv4 = ipv4;
							stSock.port = port;
							stSock.protocolType = it->second.protocolType;
							stSock.sockType = it->second.sockType;
							stSock.dataProcessInOrder = it->second.dataProcessInOrder;
							stSock.llTimeout2Close = it->second.llTimeout2Close;
							stSock.llAliveTimeStamp = GetSecondsSincePowerOn();
							_DispatchRecvData(nRecvLen, pData, stSock);
						}
					}
				}
				it++;
			}
		}
	}
}

void CSocket::_TcpConnectingProc()
{
	fd_set fdsWrite, fdsErr, fdsRead;
	timeval timeout = { 1,0 }; //select等待1秒，1秒轮询，要非阻塞就置0 

	while (m_bRun)
	{
		FD_ZERO(&fdsWrite); //每次循环都要清空集合，否则不能检测描述符变化 
		FD_ZERO(&fdsErr);
		FD_ZERO(&fdsRead);
		int nMaxFd = 0;
		map<string, StSocket> mapDev;
		//添加描述符 
		{
			//std::lock_guard<std::mutex> guard(m_muSocketConnecting);
			mapDev = m_mapDevConnecting;
			map<string, StSocket>::iterator it;
			it = mapDev.begin();
			while (it != mapDev.end())
			{
				nMaxFd = nMaxFd > it->second.sock ? nMaxFd : it->second.sock;
				FD_SET(it->second.sock, &fdsWrite);
				FD_SET(it->second.sock, &fdsErr);
				FD_SET(it->second.sock, &fdsRead);
				it++;
			}
		}
		if (mapDev.size() == 0)
		{
			this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		int nSelRet = 0;
#ifdef _WIN32
		//select函数的第一个参数，在windows下可以忽略，但在linux下必须设为最大文件描述符加1；
		nSelRet = select(0, &fdsRead, &fdsWrite, &fdsErr, &timeout);
#else
		nSelRet = select(nMaxFd + 1, &fdsRead, &fdsWrite, &fdsErr, &timeout);
#endif		
		if (-1 == nSelRet)
		{
			this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		else if (0 == nSelRet)
		{
			this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		else
		{
			map<string, StSocket>::iterator it;
			it = mapDev.begin();
			//逐个匹配socket
			while (it != mapDev.end())
			{
				if (!m_bRun)
					return;

				if (FD_ISSET(it->second.sock, &fdsErr))
				{
					_RemoveConnectingSocket(it->first, true);//从正在连接的设备map中删除该连接
					////添加连接失败消息到消息队列，通知调用者
					_AddConnectedEvent(it->second, MESS_CLOSE);
					LOG_ERROR("%s:%s connect failed(tcp), close socket", it->second.ipv4.c_str(), it->second.port.c_str());
				}
				else if (FD_ISSET(it->second.sock, &fdsWrite)) //测试sock是否连接成功&失败 
				{
					int status = 0;
					int slen = 4;
					//同样将连接的socket加入读写错误三个事件中，调用select函数，如果socket连接失败那么socket会变的同时可读和可写
					int r = getsockopt(it->second.sock, SOL_SOCKET, SO_ERROR, (char*)&status, (socklen_t *)&slen);
					int code = GET_LAST_ERROR;
					if (-1 != r && 0 == status)
					{
						ST_SOCKET stSock = it->second;
						stSock.llAliveTimeStamp = GetSecondsSincePowerOn();
						_RemoveConnectingSocket(it->first, false);//从正在连接的设备map中删除该连接
						_AddConnSocket(stSock);//新建的连接加入已连接设备map
						//添加连接成功消息到消息队列，通知调用者
						LOG_DEBUG("%s:%s connect success(tcp)", it->second.ipv4.c_str(), it->second.port.c_str());
					}
					else
					{
						_RemoveConnectingSocket(it->first, true);//从正在连接的设备map中删除该连接
						//添加连接失败消息到消息队列，通知调用者
						_AddConnectedEvent(it->second, MESS_CLOSE);
						LOG_ERROR("%s:%s connect failed(tcp), close socket,getsockopt retcode=%d, output socket status=%d,lastErrorCode=%d", 
							it->second.ipv4.c_str(), it->second.port.c_str(), r, status, code);
					}
				}
				it++;
			}
		}
	}
}

int	CSocket::_RemoveConnectingSocket(string key, bool closeSocket)
{
	CMyWLockManager writeLock(m_lockConnectingRW);
	map<string, StSocket>::iterator iter = m_mapDevConnecting.find(key);
	if (iter != m_mapDevConnecting.end())
	{
		if (closeSocket)
		{
			_CloseSocket(iter->second.sock);
		}
		m_mapDevConnecting.erase(iter);
	}
	return err_Success;
}

bool SocketServerListener(int tcpServerPort, eSocketType socketType, eProtocolType protocolType, bool dataProcessInOrder, long long second2Close)
{
	const int retryTimes = 20;
	const int sleepMilliSeconds = 1000;
	if (tcpServerPort > 0)
	{
		int count = 0;
		for (count = 0; count < retryTimes; count++)
		{
			if (err_Success != CSocket::GetInstance()->AddListener(tcpServerPort, socketType, protocolType, second2Close, dataProcessInOrder))
			{
				LOG_ERROR("%d port listener failed, socketType=%d, protocolType=%d, retry after %d milliseconds", tcpServerPort, socketType, protocolType, sleepMilliSeconds);
				std::this_thread::sleep_for(std::chrono::milliseconds(sleepMilliSeconds));
				continue;
			}
			else
			{
				break;
			}
		}
		if (count == retryTimes)
		{
			return false;
		}
	}
	return true;
}
