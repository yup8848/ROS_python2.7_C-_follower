#pragma once
#include <stdlib.h>
#include <string.h>
#include "SocketDef.h"
#include <string>
using namespace std;
class CSocketData
{
public:
	CSocketData(const unsigned char* data=(unsigned char*)"", int length=0)
	{
		m_data = NULL;
		m_data = new unsigned char[length+1];
		if (NULL != m_data)
		{
			memset(m_data, 0, length+1);
			m_dataLen = length;
			memcpy(m_data, data, length);
			m_protocolType = PROTOCOL_UNKNOWN;
			m_messType = MESS_READ;
			m_socketType = SOCKET_UDP;
			m_dataProcessInOrder = false;
		}
	}
	~CSocketData()
	{
		if (NULL != m_data)
		{
			delete[] m_data;
			m_data = NULL;
		}
		m_dataLen = 0;
	}
	CSocketData(const CSocketData& c)
	{
		m_data = new unsigned char[c.m_dataLen+1];
		if (NULL != m_data)
		{
			memset(m_data, 0, c.m_dataLen + 1);
			m_dataLen = c.m_dataLen;
			memcpy(m_data, c.m_data, m_dataLen);
			m_protocolType = c.m_protocolType;
			m_messType = c.m_messType;
			m_deviceId = c.m_deviceId;
			m_port = c.m_port;
			m_ipv4 = c.m_ipv4;
			m_recvTimeStampe = c.m_recvTimeStampe;
			m_recvMilliSecond = c.m_recvMilliSecond;
			m_socketType = c.m_socketType;
			m_dataProcessInOrder = c.m_dataProcessInOrder;
		}
	}

	CSocketData operator+(const CSocketData &c)
	{
		CSocketData _socketData = *this;
		_socketData.m_dataLen = _socketData.m_dataLen + c.m_dataLen;
		delete[] _socketData.m_data;

		_socketData.m_data = new unsigned char[_socketData.m_dataLen + 1];
		memset(_socketData.m_data, 0, _socketData.m_dataLen + 1);
		memcpy(_socketData.m_data, m_data, m_dataLen);
		memcpy(_socketData.m_data + m_dataLen, c.m_data, c.m_dataLen);
		return _socketData;
	}
	CSocketData &operator=(const CSocketData &c)
	{
		if (this == &c)
		{
			return *this;
		}

		if (NULL != m_data)
		{
			delete[] m_data;
			m_data = NULL;
			m_dataLen = 0;
		}

		m_data = new unsigned char[c.m_dataLen+1];
		if (NULL != m_data)
		{
			memset(m_data, 0, c.m_dataLen + 1);
			m_dataLen = c.m_dataLen;
			memcpy(m_data, c.m_data, m_dataLen);
			m_protocolType = c.m_protocolType;
			m_messType = c.m_messType;
			m_deviceId = c.m_deviceId;
			m_port = c.m_port;
			m_ipv4 = c.m_ipv4;
			m_recvTimeStampe = c.m_recvTimeStampe;
			m_recvMilliSecond = c.m_recvMilliSecond;
			m_socketType = c.m_socketType;
			m_dataProcessInOrder = c.m_dataProcessInOrder;
		}
		return *this;
	}
	unsigned char* GetRawData() { return m_data; }
	int GetRawDataLength() { return m_dataLen; }
    eCONN_MESS_Type GetMessageType() { return m_messType; }
	string GetDeviceId() { return m_deviceId; }
	string GetPort() { return m_port; }
	string GetIpv4() { return m_ipv4; }
    eSocketType GetSocketType() { return m_socketType; }
	string GetTimeStamp() { return m_recvTimeStampe; }
	long long GetMilliSecond() { return m_recvMilliSecond; }
    eProtocolType GetMessageProtolType() { return m_protocolType; }
	bool GetProcessInOrder() { return m_dataProcessInOrder; }

	bool SetDataOffset(int offset)
	{
		if (offset <= m_dataLen)
		{
			m_dataLen = m_dataLen - offset;
		}
		else
		{
			offset = m_dataLen;
			m_dataLen = 0;
		}


		unsigned char* data = new unsigned char[m_dataLen + 1];
		memcpy(data, m_data + offset, m_dataLen);
		delete[] m_data;
		m_data = data;
		return true;
	}
	void SetProcessInOrder(bool dataProcessInOrder) { m_dataProcessInOrder = dataProcessInOrder; }
	void SetDeviceId(string deviceId) { m_deviceId = deviceId; }
	void SetSocketPort(string port) { m_port = port; }
	void SetIpv4(string ipv4) { m_ipv4 = ipv4; }
    void SetSocketType(eSocketType type) { m_socketType = type; }
	void SetTimeStampe(string recvTimeStampe) { m_recvTimeStampe = recvTimeStampe; }
	void SetMilliSecond(long long milliSecond) { m_recvMilliSecond = milliSecond; }
    void SetMessasgeType(eCONN_MESS_Type messType) { m_messType = messType; }
    void SetProtocolType(eProtocolType protocolType) { m_protocolType = protocolType; }
private:
	unsigned char *m_data;
	int m_dataLen;
    eCONN_MESS_Type m_messType;
    eProtocolType m_protocolType;
	string m_deviceId;
	string m_port;
	string m_ipv4;
	string m_recvTimeStampe;
	long long m_recvMilliSecond; 
    eSocketType m_socketType;
	bool m_dataProcessInOrder;
};

