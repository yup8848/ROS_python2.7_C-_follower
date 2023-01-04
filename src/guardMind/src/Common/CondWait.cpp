#include "CondWait.h"


#define SIGNAL_OFF 0
#define SIGNAL_ON 1
#define SIGNAL_QUIT 2


XCondWait::XCondWait()
{
	m_iSignalStatus = SIGNAL_OFF;
}


XCondWait::~XCondWait()
{
	SetQuitSignalAll();
}

int XCondWait::Wait()
{
	std::unique_lock <std::mutex> lck(m_mutex);
	if (SIGNAL_ON == m_iSignalStatus)
	{
		return WaitResult_NoTimeout;
	}
	else if (SIGNAL_QUIT == m_iSignalStatus)
	{
		return WaitResult_Quit;
	}
	m_condivar.wait(lck);
	return WaitResult_NoTimeout;
}

WaitResult XCondWait::WaitTime(uint64_t msTime)
{
    std::unique_lock <std::mutex> lck(m_mutex);
	if (SIGNAL_ON == m_iSignalStatus)
	{
		return WaitResult_NoTimeout;
	}
	else if (SIGNAL_QUIT == m_iSignalStatus)
	{
		return WaitResult_Quit;
	}
	while (m_iSignalStatus == SIGNAL_OFF)
	{
		std::cv_status status = m_condivar.wait_for(lck, std::chrono::milliseconds(msTime));
		if (status == std::cv_status::timeout)
		{
			return WaitResult_Timeout;
		}
	}
    return WaitResult_NoTimeout;
}

int XCondWait::SetSignal()
{
	std::unique_lock <std::mutex> lck(m_mutex);
	m_iSignalStatus = SIGNAL_ON;
	m_condivar.notify_one();
	return  0;
}


int XCondWait::SetSignalAll()
{
	std::unique_lock <std::mutex> lck(m_mutex);
	m_iSignalStatus = SIGNAL_ON;
	m_condivar.notify_all();
	return  0;
}


int XCondWait::SetQuitSignalAll()
{
	std::unique_lock <std::mutex> lck(m_mutex);
	m_iSignalStatus = SIGNAL_QUIT;
	m_condivar.notify_all(); 
	return  0;
}

int XCondWait::SetQuitSignalOne()
{
	std::unique_lock <std::mutex> lck(m_mutex);
	m_iSignalStatus = SIGNAL_QUIT;
	m_condivar.notify_one();
	return  0;
}

void XCondWait::Reset()
{
	std::unique_lock <std::mutex> lck(m_mutex);
	m_iSignalStatus = SIGNAL_OFF;
}