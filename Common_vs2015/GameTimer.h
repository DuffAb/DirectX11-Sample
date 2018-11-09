//***************************************************************************************
// GameTimer.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#ifndef GAMETIMER_H
#define GAMETIMER_H

class GameTimer
{
public:
	GameTimer();

	float TotalTime()const; // in seconds ��λΪ��
	float DeltaTime()const; // in seconds ��λΪ��

	void Reset(); // Call before message loop.��Ϣѭ��ǰ����
	void Start(); // Call when unpaused.      ȡ����ͣʱ����
	void Stop();  // Call when paused.        ��ͣʱ����
	void Tick();  // Call every frame.        ÿ֡����

private:
	double mSecondsPerCount;
	double mDeltaTime;

	__int64 mBaseTime;
	__int64 mPausedTime;
	__int64 mStopTime;
	__int64 mPrevTime;
	__int64 mCurrTime;

	bool mStopped;
};

#endif // GAMETIMER_H