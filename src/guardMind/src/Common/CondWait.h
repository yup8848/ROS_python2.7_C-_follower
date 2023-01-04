#pragma once
#include <condition_variable>
#include <mutex>


enum WaitResult
{
	WaitResult_Quit,
    WaitResult_Timeout,
    WaitResult_NoTimeout
};


class XCondWait
{
public:
	XCondWait();
	~XCondWait();

    void Reset();
	int Wait();
	WaitResult WaitTime(uint64_t msTime);
	int SetSignal();
	int SetSignalAll();
	int SetQuitSignalAll();
	int SetQuitSignalOne();

private:
    std::mutex m_mutex;
    std::condition_variable m_condivar;
    //bool m_bIsAlreadySetSignal;
	short m_iSignalStatus;
};
