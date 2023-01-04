#include "Ping.h"
#include "Log.h"
#include "Common.h"
#include <iostream>
#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wsock32.lib")
#define GET_LAST_ERROR GetLastError()
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> /* for bzero */
#include <signal.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <setjmp.h>
#include <errno.h>
#define GET_LAST_ERROR errno
#endif


static void CloseSocket(int sock)
{
#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif
}
#ifdef _WIN32
#define DEF_PACKET_SIZE 32
#define ECHO_REQUEST 8
#define ECHO_REPLY 0
unsigned short CPing::s_usPacketSeq = 0;
struct IPHeader
{
	unsigned char m_byVerHLen; //4位版本+4位首部长度
	unsigned char m_byTOS; //服务类型
	unsigned short m_usTotalLen; //总长度
	unsigned short m_usID; //标识
	unsigned short m_usFlagFragOffset; //3位标志+13位片偏移
	unsigned char m_byTTL; //TTL
	unsigned char m_byProtocol; //协议
	unsigned short m_usHChecksum; //首部检验和
	unsigned long m_ulSrcIP; //源IP地址
	unsigned long m_ulDestIP; //目的IP地址
};

struct ICMPHeader
{
	unsigned char m_byType; //类型
	unsigned char m_byCode; //代码
	unsigned short m_usChecksum; //检验和 
	unsigned short m_usID; //标识符
	unsigned short m_usSeq; //序号
	unsigned long m_ulTimeStamp; //时间戳（非标准ICMP头部）
	ICMPHeader()
	{
		m_byType = 0;
		m_byCode = 0;
		m_usChecksum = 0;
		m_usSeq = 0;
		m_usID = 0;
		m_ulTimeStamp = 0;
	}
};
#if 0
CPing::CPing() :m_szICMPData(NULL), m_bIsInitSucc(false)
{
	//DAG使用时在Socket模块进行了初始化
	//WSADATA WSAData;
	//if (WSAStartup(MAKEWORD(1, 1), &WSAData) != 0)
	//{
	//	/*如果初始化不成功则报错，GetLastError()返回发生的错误信息*/
	//	printf("WSAStartup() failed: %d\n", GetLastError());
	//	return;
	//}
	m_event = WSACreateEvent();
	m_usCurrentProcID = (unsigned short)GetCurrentProcessId();
	//setsockopt(m_sockRaw);
	/*if ((m_sockRaw = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, 0)) != SOCKET_ERROR)
	{
		WSAEventSelect(m_sockRaw, m_event, FD_READ);
		m_bIsInitSucc = TRUE;

		m_szICMPData = (char*)malloc(DEF_PACKET_SIZE + sizeof(ICMPHeader));

		if (m_szICMPData == NULL)
		{
			m_bIsInitSucc = false;
		}
	}*/
	m_sockRaw = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, 0);
	if (m_sockRaw == INVALID_SOCKET)
	{
		std::cerr << "WSASocket() failed:" << WSAGetLastError() << std::endl;  //10013 以一种访问权限不允许的方式做了一个访问套接字的尝试。
	}
	else
	{
		WSAEventSelect(m_sockRaw, m_event, FD_READ);
		m_bIsInitSucc = true;

		m_szICMPData = (char*)malloc(DEF_PACKET_SIZE + sizeof(ICMPHeader));

		if (m_szICMPData == NULL)
		{
			m_bIsInitSucc = false;
		}
	}
}

CPing::~CPing()
{
	//DAG使用时在Socket模块进行释放，且Socket模块需要一直使用
	//WSACleanup();
	if (NULL != m_szICMPData)
	{
		free(m_szICMPData);
		m_szICMPData = NULL;
	}
}

bool CPing::Ping(unsigned long dwDestIP, PingReply *pPingReply, unsigned long dwTimeout)
{
	return PingCore(dwDestIP, pPingReply, dwTimeout);
}

bool CPing::Ping(const char *szDestIP, PingReply *pPingReply, unsigned long dwTimeout)
{
	if (NULL != szDestIP)
	{
		return PingCore(inet_addr(szDestIP), pPingReply, dwTimeout);
	}
	return false;
}

bool CPing::PingCore(unsigned long dwDestIP, PingReply *pPingReply, unsigned long dwTimeout)
{
	//判断初始化是否成功
	if (!m_bIsInitSucc)
	{
		return false;
	}

	//配置SOCKET
	sockaddr_in sockaddrDest;
	sockaddrDest.sin_family = AF_INET;
	sockaddrDest.sin_addr.s_addr = dwDestIP;
	int nSockaddrDestSize = sizeof(sockaddrDest);

	//构建ICMP包
	int nICMPDataSize = DEF_PACKET_SIZE + sizeof(ICMPHeader);
	unsigned long ulSendTimestamp = GetTickCountCalibrate();
	unsigned short usSeq = ++s_usPacketSeq;
	memset(m_szICMPData, 0, nICMPDataSize);
	ICMPHeader *pICMPHeader = (ICMPHeader*)m_szICMPData;
	pICMPHeader->m_byType = ECHO_REQUEST;
	pICMPHeader->m_byCode = 0;
	pICMPHeader->m_usID = m_usCurrentProcID;
	pICMPHeader->m_usSeq = usSeq;
	pICMPHeader->m_ulTimeStamp = ulSendTimestamp;
	pICMPHeader->m_usChecksum = CalCheckSum((unsigned short*)m_szICMPData, nICMPDataSize);

	//发送ICMP报文
	if (sendto(m_sockRaw, m_szICMPData, nICMPDataSize, 0, (struct sockaddr*)&sockaddrDest, nSockaddrDestSize) == SOCKET_ERROR)
	{
		return false;
	}

	//判断是否需要接收相应报文
	if (pPingReply == NULL)
	{
		return true;
	}

	char recvbuf[256] = { "\0" };
	while (1)
	{
		//接收响应报文
		if (WSAWaitForMultipleEvents(1, &m_event, FALSE, 100, FALSE) != WSA_WAIT_TIMEOUT)
		{
			WSANETWORKEVENTS netEvent;
			WSAEnumNetworkEvents(m_sockRaw, m_event, &netEvent);

			if (netEvent.lNetworkEvents & FD_READ)
			{
				unsigned long nRecvTimestamp = GetTickCountCalibrate();
				int nPacketSize = recvfrom(m_sockRaw, recvbuf, 256, 0, (struct sockaddr*)&sockaddrDest, &nSockaddrDestSize);
				if (nPacketSize != SOCKET_ERROR)
				{
					IPHeader *pIPHeader = (IPHeader*)recvbuf;
					unsigned short usIPHeaderLen = (unsigned short)((pIPHeader->m_byVerHLen & 0x0f) * 4);
					ICMPHeader *pICMPHeader = (ICMPHeader*)(recvbuf + usIPHeaderLen);

					if (pICMPHeader->m_usID == m_usCurrentProcID //是当前进程发出的报文
						&& pICMPHeader->m_byType == ECHO_REPLY //是ICMP响应报文
						&& pICMPHeader->m_usSeq == usSeq //是本次请求报文的响应报文
						)
					{
						pPingReply->m_usSeq = usSeq;
						pPingReply->m_dwRoundTripTime = nRecvTimestamp - pICMPHeader->m_ulTimeStamp;
						pPingReply->m_dwBytes = nPacketSize - usIPHeaderLen - sizeof(ICMPHeader);
						pPingReply->m_dwTTL = pIPHeader->m_byTTL;
						return true;
					}
				}
			}
		}
		//超时
		if (GetTickCountCalibrate() - ulSendTimestamp >= dwTimeout)
		{
			pPingReply->m_dwRoundTripTime = dwTimeout;
			return false;
		}
	}
}

unsigned short CPing::CalCheckSum(unsigned short *pBuffer, int nSize)
{
	unsigned long ulCheckSum = 0;
	while (nSize > 1)
	{
		ulCheckSum += *pBuffer++;
		nSize -= sizeof(unsigned short);
	}
	if (nSize)
	{
		ulCheckSum += *(UCHAR*)pBuffer;
	}

	ulCheckSum = (ulCheckSum >> 16) + (ulCheckSum & 0xffff);
	ulCheckSum += (ulCheckSum >> 16);

	return (unsigned short)(~ulCheckSum);
}

unsigned long CPing::GetTickCountCalibrate()
{
	static unsigned long s_ulFirstCallTick = 0;
	static long long s_ullFirstCallTickMS = 0;

	SYSTEMTIME systemtime;
	FILETIME filetime;
	GetLocalTime(&systemtime);
	SystemTimeToFileTime(&systemtime, &filetime);
	LARGE_INTEGER liCurrentTime;
	liCurrentTime.HighPart = filetime.dwHighDateTime;
	liCurrentTime.LowPart = filetime.dwLowDateTime;
	long long llCurrentTimeMS = liCurrentTime.QuadPart / 10000;

	if (s_ulFirstCallTick == 0)
	{
		s_ulFirstCallTick = GetTickCount();
	}
	if (s_ullFirstCallTickMS == 0)
	{
		s_ullFirstCallTickMS = llCurrentTimeMS;
	}

	return s_ulFirstCallTick + (unsigned long)(llCurrentTimeMS - s_ullFirstCallTickMS);
}
#endif
#endif


bool Init()
{
#ifdef _WIN32
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(1, 1), &WSAData) != 0)
	{
		/*如果初始化不成功则报错，GetLastError()返回发生的错误信息*/
		LOG_ERROR("WSAStartup() failed: %d\n", GetLastError());
		return false;
	}
#endif
	return true;
}
bool Release()
{
#ifdef _WIN32
	WSACleanup();
#endif
	return true;
}


#ifdef _WIN32
unsigned short CalCheckSum(unsigned short *pBuffer, int nSize)
{
	unsigned long ulCheckSum = 0;
	while (nSize > 1)
	{
		ulCheckSum += *pBuffer++;
		nSize -= sizeof(unsigned short);
	}
	if (nSize)
	{
		ulCheckSum += *(unsigned char*)pBuffer;
	}

	ulCheckSum = (ulCheckSum >> 16) + (ulCheckSum & 0xffff);
	ulCheckSum += (ulCheckSum >> 16);

	return (unsigned short)(~ulCheckSum);
}
unsigned long GetTickCountCalibrate()
{
	static unsigned long s_ulFirstCallTick = 0;
	static long long s_ullFirstCallTickMS = 0;

	SYSTEMTIME systemtime;
	FILETIME filetime;
	GetLocalTime(&systemtime);
	SystemTimeToFileTime(&systemtime, &filetime);
	LARGE_INTEGER liCurrentTime;
	liCurrentTime.HighPart = filetime.dwHighDateTime;
	liCurrentTime.LowPart = filetime.dwLowDateTime;
	long long llCurrentTimeMS = liCurrentTime.QuadPart / 10000;

	if (s_ulFirstCallTick == 0)
	{
		s_ulFirstCallTick = GetTickCount();
	}
	if (s_ullFirstCallTickMS == 0)
	{
		s_ullFirstCallTickMS = llCurrentTimeMS;
	}

	return s_ulFirstCallTick + (unsigned long)(llCurrentTimeMS - s_ullFirstCallTickMS);
}

bool Ping(const char* ipv4, PingReply *pPingReply, int sendCount, unsigned long dwTimeout)
{
	unsigned short s_usPacketSeq = 0;
	int m_sockRaw;
	m_sockRaw = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, 0);
	bool ret = false;
	if (m_sockRaw != INVALID_SOCKET)
	{
		WSAEVENT m_event;
		char m_szICMPData[4096] = { 0 };
		unsigned short m_usCurrentProcID;
		m_event = WSACreateEvent();
		WSAEventSelect(m_sockRaw, m_event, FD_READ);
		m_usCurrentProcID = (unsigned short)GetCurrentProcessId();
		//配置SOCKET
		sockaddr_in sockaddrDest;
		sockaddrDest.sin_family = AF_INET;
		sockaddrDest.sin_addr.s_addr = inet_addr(ipv4);
		int nSockaddrDestSize = sizeof(sockaddrDest);
		//构建ICMP包
		int nICMPDataSize = DEF_PACKET_SIZE + sizeof(ICMPHeader);
		unsigned long long ulSendTimestamp = GetMilliSecondsSincePowerOn();
		unsigned short usSeq = ++s_usPacketSeq;
		memset(m_szICMPData, 0, nICMPDataSize);
		ICMPHeader *pICMPHeader = (ICMPHeader*)m_szICMPData;
		pICMPHeader->m_byType = ECHO_REQUEST;
		pICMPHeader->m_byCode = 0;
		pICMPHeader->m_usID = m_usCurrentProcID;
		pICMPHeader->m_usSeq = usSeq;
		pICMPHeader->m_ulTimeStamp = GetTickCountCalibrate();
		pICMPHeader->m_usChecksum = CalCheckSum((unsigned short*)m_szICMPData, nICMPDataSize);

		int _alreadySend = 0;
		for (; _alreadySend < sendCount; _alreadySend++)
		{		
			if (sendto(m_sockRaw, m_szICMPData, nICMPDataSize, 0, (struct sockaddr*)&sockaddrDest, nSockaddrDestSize) != SOCKET_ERROR)
			{
				break;
			}
		}
		if (_alreadySend < sendCount)
		{
			//判断是否需要接收相应报文
			if (pPingReply == NULL)
			{
				ret = true;
			}
			else
			{
				char recvbuf[1024] = { 0 };
				while (1)
				{
					//接收响应报文
					if (WSAWaitForMultipleEvents(1, &m_event, FALSE, 100, FALSE) != WSA_WAIT_TIMEOUT)
					{
						WSANETWORKEVENTS netEvent;
						WSAEnumNetworkEvents(m_sockRaw, m_event, &netEvent);

						if (netEvent.lNetworkEvents & FD_READ)
						{
							unsigned long nRecvTimestamp = GetTickCountCalibrate();
							int nPacketSize = recvfrom(m_sockRaw, recvbuf, 256, 0, (struct sockaddr*)&sockaddrDest, &nSockaddrDestSize);
							if (nPacketSize != SOCKET_ERROR)
							{
								IPHeader *pIPHeader = (IPHeader*)recvbuf;
								unsigned short usIPHeaderLen = (unsigned short)((pIPHeader->m_byVerHLen & 0x0f) * 4);
								ICMPHeader *pICMPHeader = (ICMPHeader*)(recvbuf + usIPHeaderLen);

								if (pICMPHeader->m_usID == m_usCurrentProcID //是当前进程发出的报文
									&& pICMPHeader->m_byType == ECHO_REPLY //是ICMP响应报文
									&& pICMPHeader->m_usSeq == usSeq //是本次请求报文的响应报文
									)
								{
									pPingReply->m_usSeq = usSeq;
									pPingReply->m_dwRoundTripTime = nRecvTimestamp - pICMPHeader->m_ulTimeStamp;
									pPingReply->m_dwBytes = nPacketSize - usIPHeaderLen - sizeof(ICMPHeader);
									pPingReply->m_dwTTL = pIPHeader->m_byTTL;
									ret = true;
									break;
								}
							}
						}
					}
					//超时
					if (GetMilliSecondsSincePowerOn() - ulSendTimestamp >= dwTimeout)
					{
						pPingReply->m_dwRoundTripTime = dwTimeout;
						break;
					}
				}
			}
		}
		else
		{
			LOG_ERROR("ping send data failed");
		}
	}
	CloseSocket(m_sockRaw);
	return ret;
}
#else
/* 两个timeval结构相减 */
void tv_sub(struct timeval *recv, struct timeval *send) {
	if ((recv->tv_usec -= send->tv_usec) < 0) {
		--recv->tv_sec;
		recv->tv_usec += 1000000;
	}
	recv->tv_sec -= send->tv_sec;
}
/* 计算校验和的算法 */
unsigned short cal_chksum(unsigned short *addr, int len)
{
	int sum = 0;
	int nleft = len;
	unsigned short *w = addr;
	unsigned short answer = 0;

	/* 把ICMP报头二进制数据以2字节为单位累加起来 */
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}
	/*
	 * 若ICMP报头为奇数个字节，会剩下最后一字节。
	 * 把最后一个字节视为一个2字节数据的高字节，
	 * 这2字节数据的低字节为0，继续累加
	 */
	if (nleft == 1) {
		*(unsigned char *)(&answer) = *(unsigned char *)w;
		sum += answer;    /* 这里将 answer 转换成 int 整数 */
	}
	sum = (sum >> 16) + (sum & 0xffff);        /* 高位低位相加 */
	sum += (sum >> 16);        /* 上一步溢出时，将溢出位也加到sum中 */
	answer = ~sum;             /* 注意类型转换，现在的校验和为16位 */

	return answer;
}

/* 设置ICMP报头，以及将发送的时间设置为ICMP的末尾的数据部分和校验和 */
int pack(int packIndex, char* data, int length)
{
	int packsize;
	struct icmp    *icmp;
	struct timeval *tval;

	icmp = (struct icmp*)data;
	icmp->icmp_type = ICMP_ECHO;    /* icmp的类型 */
	icmp->icmp_code = 0;            /* icmp的编码 */
	icmp->icmp_cksum = 0;           /* icmp的校验和 */
	icmp->icmp_seq = packIndex;       /* icmp的顺序号 */
	icmp->icmp_id = getpid();;            /* icmp的标志符 */
	packsize = 8 + length;   /* icmp8字节的头 加上数据的长度(datalen=56), packsize = 64 */

	tval = (struct timeval *)icmp->icmp_data;    /* 获得icmp结构中最后的数据部分的指针 */
	gettimeofday(tval, NULL); /* 将发送的时间填入icmp结构中最后的数据部分 */

	icmp->icmp_cksum = cal_chksum((unsigned short *)icmp, packsize);/*填充发送方的校验和*/

	return packsize;
}

/* 对ICMP报头解包 */
int unpack(char *buf, int len, PingReply *pPingReply)
{
	int iphdrlen;
	struct ip *ip = (struct ip *)buf;
	iphdrlen = ip->ip_hl << 2;    /* 求ip报头长度,即ip报头的长度标志乘4 */

	struct icmp *icmp = (struct icmp *)(buf + iphdrlen); /* 越过ip报头,指向ICMP报头 */
	len -= iphdrlen;        /* ICMP报头及ICMP数据报的总长度 */
	if (len < 8) {                /* 小于ICMP报头长度则不合理 */
		printf("ICMP packets\'s length is less than 8\n");
		return -1;
	}
	struct timeval    tvrecv;
	gettimeofday(&tvrecv, NULL); /* 记录接收到icmp包时的时间 */
	/* 确保所接收的是我所发的的ICMP的回应 */
	if ((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == getpid())) {
		struct timeval *tvsend = (struct timeval *)icmp->icmp_data;
		tv_sub(&tvrecv, tvsend);   /* 接收和发送的时间差 */
		/* 以毫秒为单位计算发送和接收的时间差rtt */
		double rtt = tvrecv.tv_sec * 1000 + tvrecv.tv_usec / 1000;
		/* 显示相关信息 */
		pPingReply->m_usSeq = icmp->icmp_seq;
		pPingReply->m_dwTTL = ip->ip_ttl;
		pPingReply->m_dwBytes = len;
		pPingReply->m_dwRoundTripTime = rtt;
		//printf("%d byte from %s: icmp_seq=%u ttl=%d time=%.3f ms\n",
		//	len,        /* ICMP报头及ICMP数据报的总长度 */
		//	inet_ntoa(from.sin_addr),    /* ICMP的源地址 */
		//	icmp->icmp_seq,        /* icmp包发送的顺序 */
		//	ip->ip_ttl,            /* icmp存活的时间 */
		//	rtt);        /* 以毫秒为单位计算发送和接收的时间差rtt */
		return 0;
	}
	else
		return -1;
}
#define PACKET_SIZE 4096    /* 数据包的大小 */
bool Ping(const char* ipv4, PingReply *pPingReply, int sendCount, unsigned long dwTimeout)
{
	int datalen = 56; /* icmp数据包中数据的长度 */
	char sendpacket[PACKET_SIZE] = { 0 }; /* 发送的数据包 */
	char recvpacket[PACKET_SIZE] = { 0 }; /* 接收的数据包 */
	struct sockaddr_in dest_addr;  /* icmp包目的地址 */
	//int size = 50 * 1024;        //50k
	int sock = -1;
	bool ret = false;
	struct protoent *protocol;
	if ((protocol = getprotobyname("icmp")) != NULL) 
	{
		/* 生成使用ICMP的原始套接字,这种套接字只有root才能生成 */
		if ((sock = socket(AF_INET, SOCK_RAW, protocol->p_proto)) >= 0)
		{
			setuid(getuid());    /* 回收root权限,设置当前用户权限 */

			/*
			 * 扩大套接字接收缓冲区到50K这样做主要为了减小接收缓冲区溢出的
			 * 的可能性,若无意中ping一个广播地址或多播地址,将会引来大量应答
			 */
			 //setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
			dest_addr.sin_family = AF_INET;
			unsigned long inaddr = inet_addr(ipv4);
			memcpy((char*)&dest_addr.sin_addr, (char*)&inaddr, sizeof(inaddr));

			int packetsize = pack(1, sendpacket, datalen); /* 设置ICMP报头 */
			int _alreadySend = 0;
			for (; _alreadySend < sendCount; _alreadySend++)
			{
				if (sendto(sock, sendpacket, packetsize, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) >= 0)
				{
					break;
				}
			}
			if (_alreadySend < sendCount)
			{
				//判断是否需要接收相应报文
				if (pPingReply == NULL)
				{
					ret = true;
				}
				else
				{
					unsigned long long ulSendTimestamp = GetMilliSecondsSincePowerOn();
					int recvLength = -1;
					struct sockaddr_in from; /* icmp包源地址 */
					int fromlen = sizeof(from);        /* icmp包源地址的大小*/
					while (1) {
						//超时
						if (GetMilliSecondsSincePowerOn() - ulSendTimestamp >= dwTimeout)
						{
							pPingReply->m_dwRoundTripTime = dwTimeout;
							break;
						}
						if ((recvLength = recvfrom(sock, recvpacket, sizeof(recvpacket), 0, (struct sockaddr *)&from, (socklen_t *)&fromlen)) < 0)
						{
							if (errno == EINTR)
								continue;
							continue;
						}
						if (unpack(recvpacket, recvLength, pPingReply) == -1)
							continue;
						else
						{
							ret = true;
							break;
						}
					}
				}
			}
			else
			{
				LOG_ERROR("ping send data failed");
			}
		}
		else
		{
			LOG_ERROR("create socket failed");
		}
	}
	else
	{
		LOG_ERROR("getprotobyname icmp failed");
	}
	CloseSocket(sock);
	return ret;
}
#endif