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
	unsigned char m_byVerHLen; //4λ�汾+4λ�ײ�����
	unsigned char m_byTOS; //��������
	unsigned short m_usTotalLen; //�ܳ���
	unsigned short m_usID; //��ʶ
	unsigned short m_usFlagFragOffset; //3λ��־+13λƬƫ��
	unsigned char m_byTTL; //TTL
	unsigned char m_byProtocol; //Э��
	unsigned short m_usHChecksum; //�ײ������
	unsigned long m_ulSrcIP; //ԴIP��ַ
	unsigned long m_ulDestIP; //Ŀ��IP��ַ
};

struct ICMPHeader
{
	unsigned char m_byType; //����
	unsigned char m_byCode; //����
	unsigned short m_usChecksum; //����� 
	unsigned short m_usID; //��ʶ��
	unsigned short m_usSeq; //���
	unsigned long m_ulTimeStamp; //ʱ������Ǳ�׼ICMPͷ����
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
	//DAGʹ��ʱ��Socketģ������˳�ʼ��
	//WSADATA WSAData;
	//if (WSAStartup(MAKEWORD(1, 1), &WSAData) != 0)
	//{
	//	/*�����ʼ�����ɹ��򱨴�GetLastError()���ط����Ĵ�����Ϣ*/
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
		std::cerr << "WSASocket() failed:" << WSAGetLastError() << std::endl;  //10013 ��һ�ַ���Ȩ�޲�����ķ�ʽ����һ�������׽��ֵĳ��ԡ�
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
	//DAGʹ��ʱ��Socketģ������ͷţ���Socketģ����Ҫһֱʹ��
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
	//�жϳ�ʼ���Ƿ�ɹ�
	if (!m_bIsInitSucc)
	{
		return false;
	}

	//����SOCKET
	sockaddr_in sockaddrDest;
	sockaddrDest.sin_family = AF_INET;
	sockaddrDest.sin_addr.s_addr = dwDestIP;
	int nSockaddrDestSize = sizeof(sockaddrDest);

	//����ICMP��
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

	//����ICMP����
	if (sendto(m_sockRaw, m_szICMPData, nICMPDataSize, 0, (struct sockaddr*)&sockaddrDest, nSockaddrDestSize) == SOCKET_ERROR)
	{
		return false;
	}

	//�ж��Ƿ���Ҫ������Ӧ����
	if (pPingReply == NULL)
	{
		return true;
	}

	char recvbuf[256] = { "\0" };
	while (1)
	{
		//������Ӧ����
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

					if (pICMPHeader->m_usID == m_usCurrentProcID //�ǵ�ǰ���̷����ı���
						&& pICMPHeader->m_byType == ECHO_REPLY //��ICMP��Ӧ����
						&& pICMPHeader->m_usSeq == usSeq //�Ǳ��������ĵ���Ӧ����
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
		//��ʱ
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
		/*�����ʼ�����ɹ��򱨴�GetLastError()���ط����Ĵ�����Ϣ*/
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
		//����SOCKET
		sockaddr_in sockaddrDest;
		sockaddrDest.sin_family = AF_INET;
		sockaddrDest.sin_addr.s_addr = inet_addr(ipv4);
		int nSockaddrDestSize = sizeof(sockaddrDest);
		//����ICMP��
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
			//�ж��Ƿ���Ҫ������Ӧ����
			if (pPingReply == NULL)
			{
				ret = true;
			}
			else
			{
				char recvbuf[1024] = { 0 };
				while (1)
				{
					//������Ӧ����
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

								if (pICMPHeader->m_usID == m_usCurrentProcID //�ǵ�ǰ���̷����ı���
									&& pICMPHeader->m_byType == ECHO_REPLY //��ICMP��Ӧ����
									&& pICMPHeader->m_usSeq == usSeq //�Ǳ��������ĵ���Ӧ����
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
					//��ʱ
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
/* ����timeval�ṹ��� */
void tv_sub(struct timeval *recv, struct timeval *send) {
	if ((recv->tv_usec -= send->tv_usec) < 0) {
		--recv->tv_sec;
		recv->tv_usec += 1000000;
	}
	recv->tv_sec -= send->tv_sec;
}
/* ����У��͵��㷨 */
unsigned short cal_chksum(unsigned short *addr, int len)
{
	int sum = 0;
	int nleft = len;
	unsigned short *w = addr;
	unsigned short answer = 0;

	/* ��ICMP��ͷ������������2�ֽ�Ϊ��λ�ۼ����� */
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}
	/*
	 * ��ICMP��ͷΪ�������ֽڣ���ʣ�����һ�ֽڡ�
	 * �����һ���ֽ���Ϊһ��2�ֽ����ݵĸ��ֽڣ�
	 * ��2�ֽ����ݵĵ��ֽ�Ϊ0�������ۼ�
	 */
	if (nleft == 1) {
		*(unsigned char *)(&answer) = *(unsigned char *)w;
		sum += answer;    /* ���ｫ answer ת���� int ���� */
	}
	sum = (sum >> 16) + (sum & 0xffff);        /* ��λ��λ��� */
	sum += (sum >> 16);        /* ��һ�����ʱ�������λҲ�ӵ�sum�� */
	answer = ~sum;             /* ע������ת�������ڵ�У���Ϊ16λ */

	return answer;
}

/* ����ICMP��ͷ���Լ������͵�ʱ������ΪICMP��ĩβ�����ݲ��ֺ�У��� */
int pack(int packIndex, char* data, int length)
{
	int packsize;
	struct icmp    *icmp;
	struct timeval *tval;

	icmp = (struct icmp*)data;
	icmp->icmp_type = ICMP_ECHO;    /* icmp������ */
	icmp->icmp_code = 0;            /* icmp�ı��� */
	icmp->icmp_cksum = 0;           /* icmp��У��� */
	icmp->icmp_seq = packIndex;       /* icmp��˳��� */
	icmp->icmp_id = getpid();;            /* icmp�ı�־�� */
	packsize = 8 + length;   /* icmp8�ֽڵ�ͷ �������ݵĳ���(datalen=56), packsize = 64 */

	tval = (struct timeval *)icmp->icmp_data;    /* ���icmp�ṹ���������ݲ��ֵ�ָ�� */
	gettimeofday(tval, NULL); /* �����͵�ʱ������icmp�ṹ���������ݲ��� */

	icmp->icmp_cksum = cal_chksum((unsigned short *)icmp, packsize);/*��䷢�ͷ���У���*/

	return packsize;
}

/* ��ICMP��ͷ��� */
int unpack(char *buf, int len, PingReply *pPingReply)
{
	int iphdrlen;
	struct ip *ip = (struct ip *)buf;
	iphdrlen = ip->ip_hl << 2;    /* ��ip��ͷ����,��ip��ͷ�ĳ��ȱ�־��4 */

	struct icmp *icmp = (struct icmp *)(buf + iphdrlen); /* Խ��ip��ͷ,ָ��ICMP��ͷ */
	len -= iphdrlen;        /* ICMP��ͷ��ICMP���ݱ����ܳ��� */
	if (len < 8) {                /* С��ICMP��ͷ�����򲻺��� */
		printf("ICMP packets\'s length is less than 8\n");
		return -1;
	}
	struct timeval    tvrecv;
	gettimeofday(&tvrecv, NULL); /* ��¼���յ�icmp��ʱ��ʱ�� */
	/* ȷ�������յ����������ĵ�ICMP�Ļ�Ӧ */
	if ((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == getpid())) {
		struct timeval *tvsend = (struct timeval *)icmp->icmp_data;
		tv_sub(&tvrecv, tvsend);   /* ���պͷ��͵�ʱ��� */
		/* �Ժ���Ϊ��λ���㷢�ͺͽ��յ�ʱ���rtt */
		double rtt = tvrecv.tv_sec * 1000 + tvrecv.tv_usec / 1000;
		/* ��ʾ�����Ϣ */
		pPingReply->m_usSeq = icmp->icmp_seq;
		pPingReply->m_dwTTL = ip->ip_ttl;
		pPingReply->m_dwBytes = len;
		pPingReply->m_dwRoundTripTime = rtt;
		//printf("%d byte from %s: icmp_seq=%u ttl=%d time=%.3f ms\n",
		//	len,        /* ICMP��ͷ��ICMP���ݱ����ܳ��� */
		//	inet_ntoa(from.sin_addr),    /* ICMP��Դ��ַ */
		//	icmp->icmp_seq,        /* icmp�����͵�˳�� */
		//	ip->ip_ttl,            /* icmp����ʱ�� */
		//	rtt);        /* �Ժ���Ϊ��λ���㷢�ͺͽ��յ�ʱ���rtt */
		return 0;
	}
	else
		return -1;
}
#define PACKET_SIZE 4096    /* ���ݰ��Ĵ�С */
bool Ping(const char* ipv4, PingReply *pPingReply, int sendCount, unsigned long dwTimeout)
{
	int datalen = 56; /* icmp���ݰ������ݵĳ��� */
	char sendpacket[PACKET_SIZE] = { 0 }; /* ���͵����ݰ� */
	char recvpacket[PACKET_SIZE] = { 0 }; /* ���յ����ݰ� */
	struct sockaddr_in dest_addr;  /* icmp��Ŀ�ĵ�ַ */
	//int size = 50 * 1024;        //50k
	int sock = -1;
	bool ret = false;
	struct protoent *protocol;
	if ((protocol = getprotobyname("icmp")) != NULL) 
	{
		/* ����ʹ��ICMP��ԭʼ�׽���,�����׽���ֻ��root�������� */
		if ((sock = socket(AF_INET, SOCK_RAW, protocol->p_proto)) >= 0)
		{
			setuid(getuid());    /* ����rootȨ��,���õ�ǰ�û�Ȩ�� */

			/*
			 * �����׽��ֽ��ջ�������50K��������ҪΪ�˼�С���ջ����������
			 * �Ŀ�����,��������pingһ���㲥��ַ��ಥ��ַ,������������Ӧ��
			 */
			 //setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
			dest_addr.sin_family = AF_INET;
			unsigned long inaddr = inet_addr(ipv4);
			memcpy((char*)&dest_addr.sin_addr, (char*)&inaddr, sizeof(inaddr));

			int packetsize = pack(1, sendpacket, datalen); /* ����ICMP��ͷ */
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
				//�ж��Ƿ���Ҫ������Ӧ����
				if (pPingReply == NULL)
				{
					ret = true;
				}
				else
				{
					unsigned long long ulSendTimestamp = GetMilliSecondsSincePowerOn();
					int recvLength = -1;
					struct sockaddr_in from; /* icmp��Դ��ַ */
					int fromlen = sizeof(from);        /* icmp��Դ��ַ�Ĵ�С*/
					while (1) {
						//��ʱ
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