#pragma once
#include <thread>
#include <chrono>
#include "DataManager.h"
using namespace std;
using namespace std::chrono;

typedef void (*TimerCallback)(string id, void* param);

class CTimerTrigger
{
public:
	CTimerTrigger(long long _time, int _clock)
	{
		m_time = _time;
		m_clockInSecond = _clock;
	}
	int GetClock()
	{
		return m_clockInSecond;
	}
	long long GetStartTime()
	{
		return m_time;
	}
	void SetStartTime(long long _time)
	{
		m_time = _time;
	}
private:
	long long m_time;//计时器当前计时
	int m_clockInSecond;//定时时间
};

class CTimer
{
public:
	CTimer()
	{
		m_run = true;
		m_callBack = NULL;
	}
	~CTimer();
	void SetTimer(string _id, int _clockInSecond);
	void ResetTimer(string _id);
	void RemoveTimer(string _id) {
		m_clock.Erase(_id);
	}
	void SetCallback(TimerCallback callbackFun = NULL, void* contex = NULL)
	{
		m_contex = contex;
		m_callBack = callbackFun;
	}
	void Stop()
	{
		m_run = false;
	}
	void Start();

private:
	void _TimerClockThread();
private:
	CMapManager<string, CTimerTrigger> m_clock;
	TimerCallback m_callBack;
	bool m_run;
	void* m_contex;
};

