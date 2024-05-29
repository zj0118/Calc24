#include "Timer.h"

#include <atomic>
#include <chrono>

Timer::Timer(int32_t intervalMs, bool repeated, int64_t repeatedCount, TimerTask timerTask, TimerMode mode)
	:m_intervalMs(intervalMs),m_repeated(repeated),m_repeatedCount(repeatedCount),m_timerTask(timerTask), m_mode(mode)
{
	m_id = Timer::geneateTimerID();

	int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();

	m_nextTriggeredTimeMs = nowMs + m_intervalMs;
}

void Timer::doTimer(int64_t timerID, int64_t nowMs)
{
	if (m_repeated)
	{
		if (m_mode == TimerMode::TimerModeFixedInterval)
		{
			m_nextTriggeredTimeMs += m_intervalMs;
		}
		else
		{
			m_nextTriggeredTimeMs = nowMs + m_intervalMs;
		}
	}

	//����TimerModeFixedInterval��
	//���趨ʱ����ʱ������1s��ĳ��ִ�ж�ʱ���ص�����2s����һ�δ�����ʱ��ǰʱ���ǵ�3s����Ҫ����ִ��2�ζ�ʱ���ص�����
	//����TimerModeFixedDelay��
	//���趨ʱ����ʱ������1s��ĳ��ִ�ж�ʱ���ص�����2s����һ�δ�����ʱ��ǰʱ���ǵ�3s����Ҫ����ִ��1�ζ�ʱ���ص�����
	m_timerTask(timerID);
}

int64_t Timer::geneateTimerID()
{
	static std::atomic<int64_t> baseID{ 0 };
	baseID++;

	return baseID;
}
