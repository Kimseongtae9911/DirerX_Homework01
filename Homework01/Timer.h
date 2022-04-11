#pragma once

const ULONG MAX_SAMPLE_COUNT = 50;

class CGameTimer
{
private:
	double m_fTimeScale;
	float m_fTimeElapsed = 0.0f;

	__int64	m_nBasePerformanceCounter;
	__int64	m_nPausedPerformanceCounter;
	__int64	m_nStopPerformanceCounter;
	__int64	m_nCurrentPerformanceCounter = 0;
	__int64	m_nLastPerformanceCounter;

	__int64	m_PerformanceFrequencyPerSec;

	//float m_fFrameTime[MAX_SAMPLE_COUNT] = {};

	std::array<float, MAX_SAMPLE_COUNT> m_fFrameTime{};

	ULONG m_nSampleCount;

	unsigned long m_nCurrentFrameRate;
	unsigned long m_FramePerSecond;
	float m_fFPSTimeElapsed;
				  
	bool m_bStopped = false;

public:
	CGameTimer();
	virtual ~CGameTimer();

public:
	void Tick(float fLockFPS = 0.0f);
	void Start();
	void Stop();
	void Reset();

public:
	unsigned long GetFrameRate(LPTSTR lpszString = NULL, int nCharacters = 0);
	float GetTimeElapsed();
	float GetTotalTime();
};

