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
	int32_t				m_intervalMs;//定时器时间间隔
	bool				m_repeated;//是否重复
	int64_t				m_repeatedCount;//重复次数
	TimerTask			m_timerTask;
	int64_t				m_nextTriggeredTimeMs;//下一次触发的时间
	TimerMode           m_mode;
};