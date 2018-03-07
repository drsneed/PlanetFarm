#include "Timer.h"
#include "StdIncludes.h"

Timer::Timer()
	: m_secondsPerCount(0.0)
	, m_deltaTime(-1.0)
	, m_framesPerSec(0.0)
	, m_fpsCounter(0)
	, m_secondCounter(0.0)
	, m_baseTime(0)
	, m_pausedTime(0)
	, m_prevTime(0)
	, m_currTime(0)
	, m_stopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSec));
	m_secondsPerCount = 1.0 / static_cast<double>(countsPerSec);

}

void Timer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));
	m_baseTime = currTime;
	m_prevTime = currTime;
	m_stopTime = 0;
	m_stopped = false;
}

void Timer::Stop()
{
	if (!m_stopped)
	{
		__int64 currTime;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));
		m_stopTime = currTime;
		m_stopped = true;
	}
}

void Timer::Start()
{
	__int64 currTime;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

	if (m_stopped)
	{
		m_pausedTime += (currTime - m_stopTime);
		m_prevTime = currTime;

		m_stopTime = 0;
		m_stopped = false;
	}

}


void Timer::Tick()
{
	if (m_stopped)
	{
		m_deltaTime = 0.0;
		return;
	}

	// run the high performance timer
	HANDLE currentThread = GetCurrentThread();
	DWORD_PTR previousMask = SetThreadAffinityMask(currentThread, 1);
	__int64 currTime;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));
	SetThreadAffinityMask(currentThread, previousMask);
	m_currTime = currTime;
	m_deltaTime = (m_currTime - m_prevTime) * m_secondsPerCount;
	m_prevTime = m_currTime;

	// If the processor goes into a power save mode or we get shuffled to
	// another processor, delta time can be negative.
	if (m_deltaTime < 0.0)
		m_deltaTime = 0.0;

	m_secondCounter += m_deltaTime;
	m_fpsCounter += 1;
	if (m_secondCounter >= 1.0)
	{
		m_secondCounter = 0.0;
		m_framesPerSec = (double)m_fpsCounter;
		m_fpsCounter = 0;
	}
}

float Timer::GetFPS() const
{
	return static_cast<float>(m_framesPerSec);
}

float Timer::GetDeltaTime() const
{
	return static_cast<float>(m_deltaTime);
}

float Timer::GetGameTime() const
{
	return static_cast<float>(
		m_stopped ? (m_stopTime - m_pausedTime - m_baseTime) * m_secondsPerCount
		: (m_currTime - m_pausedTime - m_baseTime) * m_secondsPerCount
		);
}