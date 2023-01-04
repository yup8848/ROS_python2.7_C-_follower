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
//写一个类模板
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
	XCondWait m_resolveWait;//数据处理线程同步信号量
	vector< std::thread *> m_resolveThreads;//数据处理线程

	int m_nResolveThreadsCounts;//数据处理线程数量
	queue<T> m_queueData;

	mutex m_mutex;
	bool m_bRun;
	pFunDispatchCallback m_funCallback;
};

