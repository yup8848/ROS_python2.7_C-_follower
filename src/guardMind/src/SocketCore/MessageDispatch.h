#pragma once
#include "../Common/CondWait.h"
#include <string>
#include <iostream>
#include <cstring>
#include <map>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>

using namespace std;
//дһ����ģ��
template<typename T>
class CMessageDispatch
{
public:
	CMessageDispatch()
	{
		m_funCallback = NULL;
	}
	typedef void(*pFunDispatchCallback)(T&);
	int		Start();
	void	Stop();
	bool	AddMessage2Queue(T& data);
	void	SetCallback(pFunDispatchCallback funCallback) { m_funCallback = funCallback; }
	virtual int		MessageHandle(T& data);

private:
	void	_ResolveThreadProc();
	bool	_GetMessageFromQueue(T& data);

private:
	XCondWait m_resolveWait;//���ݴ����߳�ͬ���ź���
	vector< std::thread *> m_resolveThreads;//���ݴ����߳�

	int m_nResolveThreadsCounts;//���ݴ����߳�����
	queue<T> m_queueData;

	mutex m_mutex;
	bool m_bRun;
	pFunDispatchCallback m_funCallback;
};

