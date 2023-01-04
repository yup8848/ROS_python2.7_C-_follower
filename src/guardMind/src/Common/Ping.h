#pragma once
//��Ĭ��windows.h�����winsock.h���������winsock2.h�ͻ��ͻ������ڰ���windows.hǰ��Ҫ����һ����,#define WIN32_LEAN_AND_MEAN ;ȥ��winsock.h
//Ҫô��#include <winsock2.h>����#include<windows.h>ǰ�����ֱ��ȥ��#include<windows.h>

#ifdef _WIN32
#include <Winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#define PING_TIME_OUT 500

struct PingReply
{
	unsigned short m_usSeq;
	unsigned long m_dwRoundTripTime;
	unsigned long m_dwBytes;
	unsigned long m_dwTTL;
	PingReply()
	{
		m_usSeq = 0;
		m_dwRoundTripTime = 0;
		m_dwBytes = 0;
		m_dwTTL = 0;
	}
};

bool Init();
bool Release();
bool Ping(const char* ipv4, PingReply *pPingReply = NULL, int sendCount = 4,  unsigned long dwTimeout = PING_TIME_OUT);

#ifdef _WIN32
class CPing
{
public:
	CPing();
	~CPing();
	bool Ping(unsigned long dwDestIP, PingReply *pPingReply = NULL, unsigned long dwTimeout = PING_TIME_OUT);
	bool Ping(const char *szDestIP, PingReply *pPingReply = NULL, unsigned long dwTimeout = PING_TIME_OUT);
private:
	bool PingCore(unsigned long dwDestIP, PingReply *pPingReply, unsigned long dwTimeout);
	unsigned short CalCheckSum(unsigned short *pBuffer, int nSize);
	unsigned long GetTickCountCalibrate();
private:
	int m_sockRaw;
	WSAEVENT m_event;
	unsigned short m_usCurrentProcID;
	char *m_szICMPData;
	bool m_bIsInitSucc;
private:
	static unsigned short s_usPacketSeq;
};
#else
#endif
