//***************************************************************************************
// GameTimer.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#ifndef GAMETIMER_H
#define GAMETIMER_H

class GameTimer
{
public:
	GameTimer();

	float TotalTime()const; // in seconds 单位为秒
	float DeltaTime()const; // in seconds 单位为秒

	void Reset(); // Call before message loop.消息循环前调用
	void Start(); // Call when unpaused.      取消暂停时调用
	void Stop();  // Call when paused.        暂停时调用
	void Tick();  // Call every frame.        每帧调用

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