#include "Timer.h"
#include "Common.h"

void CTimer::SetTimer(string _id, int _clock)
{
	m_clock.Set(_id, CTimerTrigger(GetMilliSecondsSincePowerOn(), _clock));
}
void CTimer::ResetTimer(string _id)
{
	CTimerTrigger trigger(0, 0);
	if (0 == m_clock.Get(_id, trigger))
	{
		trigger.SetStartTime(GetMilliSecondsSincePowerOn());
		m_clock.Set(_id, trigger);
	}
}
CTimer::~CTimer()
{
	Stop();
}
void CTimer::_TimerClockThread()
{
	while (m_run)
	{
		this_thread::sleep_for(std::chrono::milliseconds(100));
		long long currentTime = GetMilliSecondsSincePowerOn();
		map<string, CTimerTrigger> clock = m_clock.Get();
		map<string, CTimerTrigger>::iterator it = clock.begin();
		while (it != clock.end())
		{
			if ((currentTime - it->second.GetStartTime()) > it->second.GetClock()*1000)
			{
				if (m_callBack)
				{
					m_callBack(it->first, m_contex);
				}
				it->second.SetStartTime(currentTime);
				m_clock.Update(it->first, it->second);
			}
			it++;
		}
	}
	m_clock.Clear();
}

void CTimer::Start()
{
	m_run = true;
	new thread(&CTimer::_TimerClockThread, this);
}