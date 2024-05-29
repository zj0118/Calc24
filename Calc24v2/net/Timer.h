#pragma once

#include <cstdint>

#include "ITimerService.h"

class Timer {
public:
	Timer(int32_t intervalMs, bool repeated, int64_t repeatedCount, TimerTask timerTask, 
		TimerMode mode = TimerMode::TimerModeFixedInterval);
	~Timer() = default;

	int64_t id() const {
		return m_id;
	}

	int64_t nextTriggeredTimeMs() {
		return m_nextTriggeredTimeMs;
	}

	void doTimer(int64_t timerID, int64_t nowMs);
private:
	static int64_t geneateTimerID();

	Timer(const Timer& t) = delete;
	Timer& operator=(const Timer& t) = delete;

private:
	int64_t				m_id;
	int32_t				m_intervalMs;//��ʱ��ʱ����
	bool				m_repeated;//�Ƿ��ظ�
	int64_t				m_repeatedCount;//�ظ�����
	TimerTask			m_timerTask;
	int64_t				m_nextTriggeredTimeMs;//��һ�δ�����ʱ��
	TimerMode           m_mode;
};