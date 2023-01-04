#include "MessageDispatch.h"
#include "../Common/RWLock.h"
#include "../Common/Common.h"
#include "../Common/Log.h"
#include "../Common/ErrorCodeDef.h"

#define MESSAGE_QUENE_MAX 5000
template<typename T>
void CMessageDispatch<T>::_ResolveThreadProc()
{
	while (m_bRun)
	{
		if (m_resolveWait.Wait() == WaitResult_Quit)
		{
			LOG_DEBUG("receive notify resolve thread exit");
			break;
		}

		T data;
		if (!_GetMessageFromQueue(data))
		{
			m_resolveWait.Reset();
			this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		//TODO:调用处理函数
		if (m_funCallback)
		{
			m_funCallback(data);
		}
	}
}
template<typename T>
int	CMessageDispatch<T>::MessageHandle(T& data)
{
	return err_Success;
}

template<typename T>
bool CMessageDispatch<T>::AddMessage2Queue(T& data)
{
		std::lock_guard<std::mutex> guard(m_mutex);
		if (m_queueData.size() < MESSAGE_QUENE_MAX)
			m_queueData.push(data);
		else
		{
			m_queueData.pop();
			m_queueData.push(data);
		}
		m_resolveWait.SetSignal();
	return true;
}
template<typename T>
bool CMessageDispatch<T>::_GetMessageFromQueue(T& data)
{
		std::lock_guard<std::mutex> guard(m_mutex);
		if (m_queueData.size() > 0)
		{
			data = m_queueData.front();
			m_queueData.pop();
			return true;
		}

	return false;
}
template<typename T>
int CMessageDispatch<T>::Start()
{
	m_nResolveThreadsCounts = 5;
	m_bRun = true;

	m_resolveWait.Reset();
	for (int i = 0; i < m_nResolveThreadsCounts; i++)
	{
		std::thread *thrResolve = new thread(&CMessageDispatch::_ResolveThreadProc, this);
		m_resolveThreads.push_back(thrResolve);
	}
	return 0;
}

template<typename T>
void CMessageDispatch<T>::Stop()
{
	m_bRun = false;
	m_resolveWait.SetQuitSignalAll();
	for (size_t i = 0; i < m_resolveThreads.size(); i++)
	{
		if (m_resolveThreads[i] && m_resolveThreads[i]->joinable())
		{
			m_resolveThreads[i]->join();
		}
		delete m_resolveThreads[i];
	}
	m_resolveThreads.clear();
}