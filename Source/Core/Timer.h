#pragma once

class Timer
{
public:
	Timer();
	float GetGameTime() const; // in seconds
	float GetDeltaTime() const; // in seconds
	float GetFPS() const; // in seconds

	void Reset(); // Call before message loop.
	void Start(); // Call when unpaused.
	void Stop(); // Call when paused.
	void Tick(); // Call every frame

private:
	double m_secondsPerCount;
	double m_deltaTime;
	double m_framesPerSec;
	double m_secondCounter;
	int m_fpsCounter;
	__int64 m_baseTime;
	__int64 m_pausedTime;
	__int64 m_stopTime;
	__int64 m_prevTime;
	__int64 m_currTime;
	

	bool m_stopped;
};
